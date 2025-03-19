// Copyright (c) 2025 Xist.GG

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MTGBlueprintHelpers.generated.h"

/**
 * MTG Blueprint Helpers
 *
 * Thread-safe Animation Blueprint Helper library.
 */
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

	/**
	 * Get the current Simulation Time Dilation factor.
	 * @param WorldContextObject Any world object so we can get the MTG Sim Time Subsystem
	 * @return Current Sim Time Dilation factor (1 = real time, <1 = slow time, >1 = fast time)
	 */
	UFUNCTION(BlueprintPure, Category="MassTimeGame", meta=(WorldContext="WorldContextObject"))
	static float GetSimTimeDilation(const UObject* WorldContextObject);
	
};
