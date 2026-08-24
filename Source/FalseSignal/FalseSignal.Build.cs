// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FalseSignal : ModuleRules
{
	public FalseSignal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"FalseSignal",
			"FalseSignal/Variant_Horror",
			"FalseSignal/Variant_Horror/UI",
			"FalseSignal/Variant_Shooter",
			"FalseSignal/Variant_Shooter/AI",
			"FalseSignal/Variant_Shooter/UI",
			"FalseSignal/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
