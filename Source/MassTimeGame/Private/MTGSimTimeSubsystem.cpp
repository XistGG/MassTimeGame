// Copyright (c) 2025 Xist.GG

#include "MTGSimTimeSubsystem.h"

#include "MassSimulationSubsystem.h"
#include "MassTimeGame.h"

UMTGSimTimeSubsystem::UMTGSimTimeSubsystem()
{
	// Override in Config if you want different options
	SimSpeedOptions = {.125f, .25f, .5f, .75f, 1.f, 1.25f, 1.5f, 2.f, 4.f, 8.f};
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
			// Unshift disallowed values, if any
			while (SimSpeedOptions.Num() > 0
				&& !FMath::IsWithin(SimSpeedOptions[0], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation))
			{
				UE_LOG(LogMassTimeGame, Warning, TEXT("SimSpeedOptions value %.6f is not in the valid range (%.6f .. %.6f), pruning it"), SimSpeedOptions[0], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation);
				SimSpeedOptions.RemoveAt(0);
			}

			// Pop disallowed values, if any
			while (SimSpeedOptions.Num() > 0
				&& !FMath::IsWithin(SimSpeedOptions[SimSpeedOptions.Num()-1], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation))
			{
				UE_LOG(LogMassTimeGame, Warning, TEXT("SimSpeedOptions value %.6f is not in the valid range (%.6f .. %.6f), pruning it"), SimSpeedOptions[SimSpeedOptions.Num()-1], WorldSettings->MinGlobalTimeDilation, WorldSettings->MaxGlobalTimeDilation);
				SimSpeedOptions.RemoveAt(SimSpeedOptions.Num()-1);
			}
		}
	}

	// If we don't have at least 1 option, force one into existence
	if (!ensure(SimSpeedOptions.Num() > 0))
	{
		SimSpeedOptions.Add(SimTimeDilation);
		SimSpeedIndex = 0;
		UE_LOG(LogMassTimeGame, Warning, TEXT("No valid SimSpeedOptions defined; added one: %.6f"), SimTimeDilation);
	}

	// If we don't have a valid SimSpeedIndex value, compute a default.
	if (!FMath::IsWithin(SimSpeedIndex, 0, SimSpeedOptions.Num()))
	{
		// By default, set the SimSpeedIndex to whatever index == SimTimeDilation,
		// or the closest approximation thereof.
		SimSpeedIndex = 0;
		while (SimSpeedIndex < SimSpeedOptions.Num()
			&& SimSpeedOptions[SimSpeedIndex] <= SimTimeDilation)
		{
			++SimSpeedIndex;
		}

		// Now explicitly adjust the TimeDilation based on the selected option
		SimTimeDilation = SimSpeedOptions[SimSpeedIndex];
	}
}

void UMTGSimTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	check(World);

	// Sync the WorldSettings with our idea of the TimeDilation
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
	Super::Tick(DeltaTime);

	// TODO find out when we're ticking relative to other objects

	// Notice: When the simulation is running, it's going to be incurring
	// A LOT more CPU than when it's paused. Thus, we'll use UNLIKELY here
	// to optimize for that state. Yes, this burns a little CPU when the
	// simulation is not paused, but it still seems worthwhile.

	if (UNLIKELY(IsPaused()))
	{
		// While paused, report zero DeltaTime
		SimDeltaTime = 0.;
	}
	else
	{
		// While running, keep track of time
		SimDeltaTime = DeltaTime;
		SimTime += DeltaTime;
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

	if (SimTimeDilation != WorldSettings->TimeDilation)
	{
		const float OldTimeDilation = SimTimeDilation;
		SimTimeDilation = WorldSettings->TimeDilation;

		// Try to find what the new SimSpeedIndex is
		SimSpeedIndex = 0;
		while (SimSpeedIndex < SimSpeedOptions.Num()
			&& SimSpeedOptions[SimSpeedIndex] <= SimTimeDilation)
		{
			++SimSpeedIndex;
		}

		if (SimSpeedOptions[SimSpeedIndex] == SimTimeDilation)
		{
			UE_LOG(LogMassTimeGame, Warning, TEXT("Something changed the world time dilation from %.6f to %.6f! Had to sync."), OldTimeDilation, SimTimeDilation);
		}
		else
		{
			UE_LOG(LogMassTimeGame, Error, TEXT("Something changed the world time dilation from %.6f to %.6f! New value is not defined in SimTimeOptions, SimSpeedIndex is approximated to %d."), OldTimeDilation, SimTimeDilation, SimSpeedIndex);
		}
	}
}

bool UMTGSimTimeSubsystem::IncreaseSimSpeed()
{
	UWorld* World = GetWorld();
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

	UE_LOG(LogMassTimeGame, Log, TEXT("Increase Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), SimTimeDilation);

	WorldSettings->SetTimeDilation(SimTimeDilation);

	return true;
}

bool UMTGSimTimeSubsystem::DecreaseSimSpeed()
{
	UWorld* World = GetWorld();
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

	UE_LOG(LogMassTimeGame, Log, TEXT("Decrease Simulation Speed to %d/%d (%0.3fx)"), 1+SimSpeedIndex, SimSpeedOptions.Num(), SimTimeDilation);

	WorldSettings->SetTimeDilation(SimTimeDilation);

	return true;
}

bool UMTGSimTimeSubsystem::TogglePlayPause()
{
	UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(GetWorld());

	if (nullptr == MassSimulationSubsystem)
	{
		return false;
	}

	if (IsPaused())
	{
		UE_LOG(LogMassTimeGame, Log, TEXT("Resume Simulation"));
		MassSimulationSubsystem->ResumeSimulation();
	}
	else
	{
		UE_LOG(LogMassTimeGame, Log, TEXT("Pause Simulation"));
		MassSimulationSubsystem->PauseSimulation();
	}

	return true;
}

void UMTGSimTimeSubsystem::NativeOnSimulationPaused(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem)
{
	bIsSimPaused = true;
	OnSimulationPaused.Broadcast(this);
}

void UMTGSimTimeSubsystem::NativeOnSimulationResumed(TNotNull<UMassSimulationSubsystem*> MassSimulationSubsystem)
{
	bIsSimPaused = false;
	OnSimulationResumed.Broadcast(this);
}
