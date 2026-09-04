// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Zombiemaid69 : ModuleRules
{
	public Zombiemaid69(ReadOnlyTargetRules Target) : base(Target)
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
			"Zombiemaid69",
			"Zombiemaid69/Variant_Horror",
			"Zombiemaid69/Variant_Horror/UI",
			"Zombiemaid69/Variant_Shooter",
			"Zombiemaid69/Variant_Shooter/AI",
			"Zombiemaid69/Variant_Shooter/UI",
			"Zombiemaid69/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
