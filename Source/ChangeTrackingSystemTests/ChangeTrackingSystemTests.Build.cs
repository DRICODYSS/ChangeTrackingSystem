// Copyright DRICODYSS. All Rights Reserved.

using UnrealBuildTool;

public class ChangeTrackingSystemTests : ModuleRules
{
	public ChangeTrackingSystemTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ChangeTrackingSystem",
				"AutomationController",
				"Projects"
			}
		);

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}
	}
}
