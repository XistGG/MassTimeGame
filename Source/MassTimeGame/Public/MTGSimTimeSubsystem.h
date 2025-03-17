// Copyright (c) 2025 Xist.GG

#pragma once

#include "Subsystems/WorldSubsystem.h"
#include "MTGSimTimeSubsystem.generated.h"

class UMassSimulationSubsystem;

UCLASS(Config=MTG)
class MASSTIMEGAME_API UMTGSimTimeSubsystem  : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnSimulationPauseEvent, TNotNull<UMTGSimTimeSubsystem*> /*this*/);

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

	FOnSimulationPauseEvent& GetOnSimulationPaused() { return OnSimulationPaused; }
	FOnSimulationPauseEvent& GetOnSimulationResumed() { return OnSimulationResumed; }

	bool IsPaused() const { return bIsSimPaused; }
	double GetDeltaTime() const { return SimDeltaTime; }
	uint64 GetTickNumber() const { return SimTickNumber; }
	double GetTime() const { return SimTime; }
	float GetTimeDilation() const { return SimTimeDilation; }

	bool CanIncreaseSimSpeed() const { return SimSpeedIndex < SimSpeedOptions.Num() - 1; }
	bool CanDecreaseSimSpeed() const { return SimSpeedIndex > 0; }

	bool IncreaseSimSpeed();
	bool DecreaseSimSpeed();

	bool TogglePlayPause();

protected:
	UPROPERTY(VisibleInstanceOnly, Category="Xist")
	int32 SimSpeedIndex {INDEX_NONE};

	UPROPERTY(EditDefaultsOnly, Category="Xist", Config)
	TArray<float> SimSpeedOptions;

	void NativeOnSimulationPaused(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);
	void NativeOnSimulationResumed(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem);

private:
	bool bIsSimPaused = false;
	double SimDeltaTime = 0.;
	uint64 SimTickNumber = 0;
	double SimTime = 0.;
	float SimTimeDilation = 1.;

	FOnSimulationPauseEvent OnSimulationPaused;
	FOnSimulationPauseEvent OnSimulationResumed;
};
