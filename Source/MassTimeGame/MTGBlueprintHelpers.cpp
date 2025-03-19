// Copyright (c) 2025 Xist.GG

#include "MTGBlueprintHelpers.h"

#include "MassSimulationSubsystem.h"
#include "MTGSimTimeSubsystem.h"
#include "Engine/World.h"

bool UMTGBlueprintHelpers::IsSimulationPaused(const UObject* WorldContextObject)
{
	// NOTICE: We promised the Anim BP this function is thread safe!
	// We're just reading a boolean value here...

	bool bIsPaused {false};
	if (WorldContextObject)
	{
		if (const UMassSimulationSubsystem* MassSimulationSubsystem = UWorld::GetSubsystem<UMassSimulationSubsystem>(WorldContextObject->GetWorld()))
		{
			bIsPaused = MassSimulationSubsystem->IsSimulationPaused();
		}
	}
	return bIsPaused;
}

float UMTGBlueprintHelpers::GetSimTimeDilation(const UObject* WorldContextObject)
{
	// NOTICE: We promised the Anim BP this function is thread safe!
	// We're just reading a float value here...

	float TimeDilation = 1.;
	if (WorldContextObject)
	{
		if (const UMTGSimTimeSubsystem* SimTimeSubsystem = UWorld::GetSubsystem<UMTGSimTimeSubsystem>(WorldContextObject->GetWorld()))
		{
			TimeDilation = SimTimeSubsystem->GetSimTimeDilation();
		}
	}
	return TimeDilation;
}
