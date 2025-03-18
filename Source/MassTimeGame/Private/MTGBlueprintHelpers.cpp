// Copyright (c) 2025 Xist.GG

#include "MTGBlueprintHelpers.h"

#include "MassSimulationSubsystem.h"
#include "Engine/World.h"

bool UMTGBlueprintHelpers::IsSimulationPaused(const UObject* WorldContextObject)
{
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
