// Copyright (c) 2025 Xist.GG

#include "MTGSimTimeSubsystem.h"

#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"
#include "GameFramework/WorldSettings.h"

UMTGSimTimeSubsystem::UMTGSimTimeSubsystem()
{
	// Override in Config if you want different options
	SimSpeedOptions = {.125f, .25f, .5f, .75f, 1.f, 2.f, 3.f, 4.f, 8.f};
}

void UMTGSimTimeSubsystem::PostInitProperties()
{
	Super::PostInitProperties();

	// Make sure the sim speed is always sorted ascending
	SimSpeedOptions.Sort();

	if (const UWorld* World = GetWorld())
	{
		const AWorldSettings* WorldSettings = World->GetWorldSettings();
		if (ensure(WorldSettings))
		{
			// Make sure this is NEVER ZERO
			const float MinAllowedValue = FMath::Max(UE_SMALL_NUMBER, WorldSettings->MinGlobalTimeDilation);

			// Unshift disallowed values, if any
			while (SimSpeedOptions.Num() > 0
				&& !FMath::IsWithin(SimSpeedOptions[0], MinAllowedValue, WorldSettings->MaxGlobalTimeDilation))
			{
				UE_LOG(LogMassTimeGame, Warning, TEXT("SimSpeedOptions value %.6f is not in the valid range (%.6f .. %.6f), pruning it"), SimSpeedOptions[0], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation);
				SimSpeedOptions.RemoveAt(0);
			}

			// Pop disallowed values, if any
			while (SimSpeedOptions.Num() > 0
				&& !FMath::IsWithin(SimSpeedOptions[SimSpeedOptions.Num()-1], MinAllowedValue, WorldSettings->MaxGlobalTimeDilation))
			{
				UE_LOG(LogMassTimeGame, Warning, TEXT("SimSpeedOptions value %.6f is not in the valid range (%.6f .. %.6f), pruning it"), SimSpeedOptions[SimSpeedOptions.Num()-1], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation);
				SimSpeedOptions.RemoveAt(SimSpeedOptions.Num()-1);
			}
		}
	}

	// If we don't have at least 1 option, force one into existence
	if (!ensure(SimSpeedOptions.Num() > 0))
	{
		// DO NOT allow zero (or negative) SimTimeDilation
		checkf(SimTimeDilation > 0., TEXT("SimTimeDilation can never be <= 0"));
		SimSpeedOptions.Add(SimTimeDilation);
		SimSpeedIndex = 0;
		UE_LOG(LogMassTimeGame, Warning, TEXT("No valid SimSpeedOptions defined; added one: %.6f"), SimTimeDilation);
	}

	// If we don't have a valid SimSpeedIndex value, compute a default.
	if (!FMath::IsWithin(SimSpeedIndex, 0, SimSpeedOptions.Num()))
	{
		SimSpeedIndex = FindApproximateSimSpeedIndex();
		SimTimeDilation = SimSpeedOptions[SimSpeedIndex];  // Startup: force time dilation to match the found index
	}
}

void UMTGSimTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	check(World);

	// Startup: force the world to use the time dilation setting we want to start with
	AWorldSettings* WorldSettings = World->GetWorldSettings();
	if (ensure(WorldSettings))
	{
		WorldSettings->SetTimeDilation(SimTimeDilation);
	}

	UMassSimulationSubsystem* MassSimulationSubsystem = Collection.InitializeDependency<UMassSimulationSubsystem>();
	if (!ensureMsgf(MassSimulationSubsystem, TEXT("MassSimulationSubsystem is required")))
	{
		return;
	}

	bIsSimPaused = MassSimulationSubsystem->IsSimulationPaused();

	MassSimulationSubsystem->GetOnSimulationPaused().AddUObject(this, &ThisClass::NativeOnSimulationPaused);
	MassSimulationSubsystem->GetOnSimulationResumed().AddUObject(this, &ThisClass::NativeOnSimulationResumed);
}

void UMTGSimTimeSubsystem::Deinitialize()
{
	if (UMassSimulationSubsystem* MassSimulationSubsystem = GetWorld()->GetSubsystem<UMassSimulationSubsystem>())
	{
		MassSimulationSubsystem->GetOnSimulationPaused().RemoveAll(this);
		MassSimulationSubsystem->GetOnSimulationResumed().RemoveAll(this);
	}

	Super::Deinitialize();
}

void UMTGSimTimeSubsystem::Tick(float DeltaTime)
{
	// DeltaTime is sim-dilated
	Super::Tick(DeltaTime);

	// Notice: When the simulation is running, it's going to be incurring
	// A LOT more CPU than when it's paused. Thus, we'll use UNLIKELY here
	// to optimize for that state. Yes, this burns a little CPU when the
	// simulation is paused, but it still seems worthwhile.

	if (UNLIKELY(IsPaused()))
	{
		// While paused, report zero DeltaTime
		SimDeltaTime = 0.;
	}
	else
	{
		// While running, keep track of time (DeltaTime is sim-dilated)
		SimDeltaTime = DeltaTime;
		SimTimeElapsed += DeltaTime;
		++SimTickNumber;
	}

	// Check for external changes to the world time dilation.
	//
	// Theoretically this shouldn't happen...
	//
	// If it does happen we need to account for it in the display
	// as best as possible.
	//
	// At the very least, make sure we're aware it is happening.

	const UWorld* World = GetWorld();
	check(World);

	const AWorldSettings* WorldSettings = World->GetWorldSettings();
	check(WorldSettings);

	if (UNLIKELY(SimTimeDilation != WorldSettings->TimeDilation))
	{
		const float OldTimeDilation = SimTimeDilation;
		SimTimeDilation = WorldSettings->TimeDilation;

		SimSpeedIndex = FindApproximateSimSpeedIndex();

		if (SimSpeedOptions[SimSpeedIndex] == SimTimeDilation)
		{
			// We found the appropriate SimSpeedIndex matching this dilation, so while this is unexpected,
			// it's not necessarily super confusing for the user.
			UE_LOG(LogMassTimeGame, Warning, TEXT("Something changed the world time dilation from %.6f to %.6f! Had to sync."), OldTimeDilation, SimTimeDilation);
		}
		else
		{
			// There is no corresponding SimSpeedIndex to match this time dilation.
			// The controls will say one thing, but the game is doing something else.
			// User confusion will ensue.
			UE_LOG(LogMassTimeGame, Error, TEXT("Something changed the world time dilation from %.6f to %.6f! New value is not defined in SimTimeOptions, SimSpeedIndex is approximated to %d."), OldTimeDilation, SimTimeDilation, SimSpeedIndex);
		}

		OnTimeDilationChanged.Broadcast(this);
	}
}

bool UMTGSimTimeSubsystem::IncreaseSimSpeed()
{
	const UWorld* World = GetWorld();
	check(World);

	UMassSimulationSubsystem* MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
	AWorldSettings* WorldSettings = World->GetWorldSettings();

	if (false == CanIncreaseSimSpeed()
		|| nullptr == MassSimulationSubsystem
		|| nullptr == WorldSettings)
	{
		return false;
	}

	++SimSpeedIndex;
	SimTimeDilation = SimSpeedOptions[SimSpeedIndex];

	UE_LOG(LogMassTimeGame, Verbose, TEXT("Increase Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), SimTimeDilation);

	WorldSettings->SetTimeDilation(SimTimeDilation);
	OnTimeDilationChanged.Broadcast(this);

	return true;
}

bool UMTGSimTimeSubsystem::DecreaseSimSpeed()
{
	const UWorld* World = GetWorld();
	check(World);

	UMassSimulationSubsystem* MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
	AWorldSettings* WorldSettings = World->GetWorldSettings();

	if (false == CanDecreaseSimSpeed()
		|| nullptr == MassSimulationSubsystem
		|| nullptr == WorldSettings)
	{
		return false;
	}

	--SimSpeedIndex;
	SimTimeDilation = SimSpeedOptions[SimSpeedIndex];

	UE_LOG(LogMassTimeGame, Verbose, TEXT("Decrease Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), SimTimeDilation);

	WorldSettings->SetTimeDilation(SimTimeDilation);
	OnTimeDilationChanged.Broadcast(this);

	return true;
}

bool UMTGSimTimeSubsystem::TogglePlayPause()
{
	if (IsPaused())
	{
		return ResumeSimulation();
	}

	return PauseSimulation();
}

bool UMTGSimTimeSubsystem::PauseSimulation()
{
	if (IsPaused())
	{
		// We're already paused, we don't need to do anything.
		return true;
	}

	const UWorld* World = GetWorld();
	check(World);

	UMassSimulationSubsystem* MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();

	if (nullptr == MassSimulationSubsystem)
	{
		return false;
	}

	// We need UMassSimulationSubsystem to actually pause the simulation.
	// Also, this WILL NOT take effect immediately, so we DO NOT immediately mark the sim as resumed.
	// We'll wait for its callback to do that.
	UE_LOG(LogMassTimeGame, Verbose, TEXT("Pause Simulation"));
	MassSimulationSubsystem->PauseSimulation();

	// We requested the pause state to change, now we just need to wait for it to actually change.
	return true;
}

bool UMTGSimTimeSubsystem::ResumeSimulation()
{
	if (false == IsPaused())
	{
		// We're already resumed/playing, we don't need to do anything.
		return true;
	}

	const UWorld* World = GetWorld();
	check(World);

	UMassSimulationSubsystem* MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();

	if (nullptr == MassSimulationSubsystem)
	{
		return false;
	}

	// We need UMassSimulationSubsystem to actually resume the simulation.
	// Also, this WILL NOT take effect immediately, so we DO NOT immediately mark the sim as paused.
	// We'll wait for its callback to do that.
	UE_LOG(LogMassTimeGame, Verbose, TEXT("Resume Simulation"));
	MassSimulationSubsystem->ResumeSimulation();

	// We requested the pause state to change, now we just need to wait for it to actually change.
	return true;
}

int32 UMTGSimTimeSubsystem::FindApproximateSimSpeedIndex()
{
	// Get the closest approximation we can to the current SimTimeDilation value
	// (which may not necessarily be one of the listed speed options)
	// Find the greatest Index whose value is <= SimTimeDilation

	int Index {0};
	while (Index + 1 < SimSpeedOptions.Num()  // Stay in valid range
		&& SimSpeedOptions[Index] < SimTimeDilation)  // Don't pass the current time dilation value
	{
		++Index;
	}

	return Index;
}

void UMTGSimTimeSubsystem::NativeOnSimulationPaused(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem)
{
	// UMassSimulationSubsystem notified us the sim is now paused
	bIsSimPaused = true;
	OnSimulationPaused.Broadcast(this);  // Relay this event
}

void UMTGSimTimeSubsystem::NativeOnSimulationResumed(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem)
{
	// UMassSimulationSubsystem notified us the sim is now resumed
	bIsSimPaused = false;
	OnSimulationResumed.Broadcast(this);  // Relay this event
}
