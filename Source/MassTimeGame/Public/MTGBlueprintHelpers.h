// Copyright (c) 2025 Xist.GG

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MTGBlueprintHelpers.generated.h"

class UMTGSimTimeSubsystem;

UCLASS(meta=(BlueprintThreadSafe, DisplayName="MTG Blueprint Helpers"))
class MASSTIMEGAME_API UMTGBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Is the Mass Simulation currently Paused?
	 * @param WorldContextObject Any world object so we can get the Mass Simulation Subsystem
	 * @return True if the simulation is currently paused, else False
	 */
	UFUNCTION(BlueprintPure, Category="MassTimeGame", meta=(WorldContext="WorldContextObject"))
	static bool IsSimulationPaused(const UObject* WorldContextObject);
};
