// Copyright (c) 2025 Xist.GG

using UnrealBuildTool;
using System.Collections.Generic;

public class MassTimeGameEditorTarget : TargetRules
{
	public MassTimeGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("MassTimeGame");
	}
}
