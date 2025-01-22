// Copyright (c) 2025 Xist.GG

using UnrealBuildTool;

public class MassTimeGame : ModuleRules
{
	public MassTimeGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePathModuleNames.AddRange(new string[] { "MassTimeGame" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", "AIModule", "Niagara", "EnhancedInput" });
        PrivateDependencyModuleNames.AddRange(new string[] { "MassSimulation", "UMG", "Slate" });
	}
}
