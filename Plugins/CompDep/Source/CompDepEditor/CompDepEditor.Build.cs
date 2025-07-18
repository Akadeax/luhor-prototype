// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CompDepEditor : ModuleRules
{
	public CompDepEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			[
				"Core",
				"CompDep"
			]
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"ToolMenus",
				"Blutility",
				"UMG",
				"UMGEditor",
			}
		);
	}
}
