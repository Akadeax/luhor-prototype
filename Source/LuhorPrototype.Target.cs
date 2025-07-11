// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LuhorPrototypeTarget : TargetRules
{
	public LuhorPrototypeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
		ExtraModuleNames.Add("LuhorPrototype");
	}
}
