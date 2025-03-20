// Copyright (c) 2025 Xist.GG

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "MTGSimTimeSubsystem.generated.h"

class UMassSimulationSubsystem;

/**
 * MTG Sim Time Subsystem
 *
 * This is the subsystem that does most of the heavy lifting in this project.
 * 
 * This interfaces with UMassSimulationSubsystem which does the actual Play/Pause
 * state management, and this adds global time dilation functionality.
 *
 * To simplify usage, I reimplemented the Pause/Resume events here so you don't
 * need to worry about which subsystem creates that versus the time dilation
 * events, you can just subscribe to all the relevant events here and ignore
 * the use of UMassSimulationSubsystem under the hood.
 */
UCLASS(Config=MTG, meta=(DisplayName="MTG Sim Time Subsystem"))
class MASSTIMEGAME_API UMTGSimTimeSubsystem  : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPauseStateChanged, TNotNull<UMTGSimTimeSubsystem*> /*this*/);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeDilationChanged, TNotNull<UMTGSimTimeSubsystem*> /*this*/);

	/** Delegate broadcast when the simulation enters the Paused state */
	FOnPauseStateChanged& GetOnSimulationPaused() { return OnSimulationPaused; }

	/** Delegate broadcast when the simulation enters the Resumed state */
	FOnPauseStateChanged& GetOnSimulationResumed() { return OnSimulationResumed; }

	/** Delegate broadcast when the simulation time dilation factor changes */
	FOnTimeDilationChanged& GetOnTimeDilationChanged() { return OnTimeDilationChanged; }

	// Set Class Defaults
	UMTGSimTimeSubsystem();

	//~Begin UObject interface
	virtual void PostInitProperties() override;
	//~End UObject interface

	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End USubsystem interface

	//~Begin UTickableWorldSubsystem interface
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMTGSimTimeSubsystem, STATGROUP_Tickables); }
	virtual void Tick(float DeltaTime) override;
	//~End UTickableWorldSubsystem interface

	/**
	 * Convert a dilated time (probably a DeltaTime) into real time
	 * @param TimeSeconds A globally dilated time in seconds
	 * @return The real time represented by TimeSeconds
	 */
	float GetRealTimeSeconds(const float TimeSeconds) const { return TimeSeconds / GetSimTimeDilation(); }

	/**
	 * Get the time dilation factor to apply to a time to convert it from dilated time to real time.
	 * This is the inverse of the sim time dilation factor.
	 * @return Real time dilation factor
	 */
	float GetRealTimeDilation() const { return 1. / GetSimTimeDilation(); }

	/**
	 * Is the simulation currently paused?
	 * @return True if the simulation is currently paused, else False
	 */
	bool IsPaused() const { return bIsSimPaused; }

	/**
	 * Get the current simulation DeltaTime
	 *
	 * This will be reported as zero when IsPaused() is true
	 * 
	 * @return Dilated simulation DeltaTime
	 */
	double GetSimDeltaTime() const { return SimDeltaTime; }

	/**
	 * Get the current simulation tick number
	 * @return Number of frames the simulation has ticked
	 */
	uint64 GetSimTickNumber() const { return SimTickNumber; }

	/**
	 * Get the total amount of time that has elapsed in the simulation.
	 * This value is affected by the sim dilation and by the pause state.
	 * Time does not accumulate while the simulation is paused.
	 * @return Total simulation time elapsed
	 */
	double GetSimTimeElapsed() const { return SimTimeElapsed; }

	/**
	 * Get the current sim time dilation factor
	 *
	 * THIS WILL NEVER BE ZERO. A minimum of UE_SMALL_NUMBER is enforced at all times.
	 * A larger minimum may be enforced by WorldSettings, but it will NEVER EVER
	 * be less than UE_SMALL_NUMBER.
	 * 
	 * @return Current simulation time dilation factor
	 */
	float GetSimTimeDilation() const { return SimTimeDilation; }

	/**
	 * Is it possible to increase the sim speed?
	 * @return True if faster speeds are available, else False
	 */
	bool CanIncreaseSimSpeed() const { return SimSpeedIndex < SimSpeedOptions.Num() - 1; }

	/**
	 * Is it possible to decrease the sim speed?
	 * @return True if slower speeds are available, else False
	 */
	bool CanDecreaseSimSpeed() const { return SimSpeedIndex > 0; }

	/**
	 * Try to increase the sim speed
	 * @return True if the speed has been increased, else False
	 */
	bool IncreaseSimSpeed();

	/**
	 * Try to decrease the sim speed
	 * @return True if the speed has been decreased, else False
	 */
	bool DecreaseSimSpeed();

	/**
	 * Try to toggle the simulation Play/Pause state
	 *
	 * NOTICE: UMassSimulationSubsystem actually has control over the Play/Pause
	 * state, and it DOES NOT immediately change the state if we request a change
	 * while it is processing. Thus, you MAY NOT see an immediate pause, but
	 * the pause SHOULD take effect by the end of the current frame.
	 * 
	 * @return True if we attempted to change the Play/Pause state, else False
	 */
	bool TogglePlayPause();

	/**
	 * Put the simulation into the Paused state.
	 * @return True if we attempted to pause the simulation, else False
	 */
	bool PauseSimulation();

	/**
	 * Put the simulation into the Resumed/Play state.
	 * @return True if we attempted to resume the simulation, else False
	 */
	bool ResumeSimulation();

protected:
	/**
	 * An ordered array of all the possible sim speed settings.
	 *
	 * This array must never be empty. To prevent it being empty, PostInitProperties
	 * will insert a single value equal to the current time dilation (probably 1.0)
	 * if the array is empty after reading the Config.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Xist", Config)
	TArray<float> SimSpeedOptions;

	/**
	 * Try to find the index in SimSpeedOptions that corresponds to the current SimTimeDilation.
	 * @return SimSpeedOptions index of the highest value that is <= SimTimeDilation
	 */
	int32 FindApproximateSimSpeedIndex();

	/**
	 * Callback from UMassSimulationSubsystem when the simulation changes to the Paused state
	 * @param MassSimulationSubsystem the World's MassSimulationSubsystem
	 */
	void NativeOnSimulationPaused(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);

	/**
	 * Callback from UMassSimulationSubsystem when the simulation changes to the Resumed state
	 * @param MassSimulationSubsystem the World's MassSimulationSubsystem
	 */
	void NativeOnSimulationResumed(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);

private:
	/** Is the sim currently paused? */
	bool bIsSimPaused = false;

	/** Current sim DeltaTime (with applied global time dilation) */
	double SimDeltaTime = 0.;

	/** Number of non-paused simulation ticks since the world began play */
	uint64 SimTickNumber = 0;

	/** Total amount of dilated elapsed sim time since the world began play */
	double SimTimeElapsed = 0.;

	/** Current sim time dilation factor */
	float SimTimeDilation = 1.;

	/** SimSpeedOptions index most closely matching the current SimTimeDilation */
	int32 SimSpeedIndex = INDEX_NONE;

	/** Delegate broadcast when the simulation enters the Paused state */
	FOnPauseStateChanged OnSimulationPaused;

	/** Delegate broadcast when the simulation enters the Resumed state */
	FOnPauseStateChanged OnSimulationResumed;

	/** Delegate broadcast when the simulation time dilation factor changes */
	FOnTimeDilationChanged OnTimeDilationChanged;
};
