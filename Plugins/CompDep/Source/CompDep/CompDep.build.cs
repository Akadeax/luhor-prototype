// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CompDep : ModuleRules
{
	public CompDep(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			[
				"Core", "CoreUObject", "Engine"
			]
		);
	}
}