// Copyright (c) 2025 Xist.GG

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "MTGSimTimeSubsystem.generated.h"

class UMassSimulationSubsystem;

UCLASS(Config=MTG, meta=(DisplayName="MTG Sim Time Subsystem"))
class MASSTIMEGAME_API UMTGSimTimeSubsystem  : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnPauseStateChanged, TNotNull<UMTGSimTimeSubsystem*> /*this*/);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeDilationChanged, TNotNull<UMTGSimTimeSubsystem*> /*this*/);

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

	FOnPauseStateChanged& GetOnSimulationPaused() { return OnSimulationPaused; }
	FOnPauseStateChanged& GetOnSimulationResumed() { return OnSimulationResumed; }
	FOnTimeDilationChanged& GetOnTimeDilationChanged() { return OnTimeDilationChanged; }

	float GetRealTimeSeconds(const float TimeSeconds) const { return TimeSeconds / GetSimTimeDilation(); }
	float GetRealTimeDilation() const { return 1. / GetSimTimeDilation(); }

	bool IsPaused() const { return bIsSimPaused; }

	double GetSimDeltaTime() const { return SimDeltaTime; }
	uint64 GetSimTickNumber() const { return SimTickNumber; }
	double GetSimTimeElapsed() const { return SimTimeElapsed; }
	float GetSimTimeDilation() const { return SimTimeDilation; }

	bool CanIncreaseSimSpeed() const { return SimSpeedIndex < SimSpeedOptions.Num() - 1; }
	bool CanDecreaseSimSpeed() const { return SimSpeedIndex > 0; }

	bool IncreaseSimSpeed();
	bool DecreaseSimSpeed();

	bool TogglePlayPause();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Xist", Config)
	TArray<float> SimSpeedOptions;

	int32 FindApproximateSimSpeedIndex();

	void NativeOnSimulationPaused(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);
	void NativeOnSimulationResumed(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);

private:
	bool bIsSimPaused = false;
	double SimDeltaTime = 0.;
	uint64 SimTickNumber = 0;
	double SimTimeElapsed = 0.;
	float SimTimeDilation = 1.;
	int32 SimSpeedIndex = INDEX_NONE;

	FOnPauseStateChanged OnSimulationPaused;
	FOnPauseStateChanged OnSimulationResumed;
	FOnTimeDilationChanged OnTimeDilationChanged;
};
