// Copyright (c) 2025 Xist.GG

using UnrealBuildTool;
using System.Collections.Generic;

public class MassTimeGameTarget : TargetRules
{
	public MassTimeGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("MassTimeGame");
	}
}
