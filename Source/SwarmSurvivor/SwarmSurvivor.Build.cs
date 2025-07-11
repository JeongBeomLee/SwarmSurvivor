// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SwarmSurvivor : ModuleRules
{
	public SwarmSurvivor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "UMG",
            "Slate",
            "SlateCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] 
        { 
            "EnhancedInput",
            "GameplayTags",
            "AIModule",
            "NavigationSystem"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "SwarmSurvivor",
            "SwarmSurvivor/Core",
            "SwarmSurvivor/Interface",
            "SwarmSurvivor/Data",
            "SwarmSurvivor/Utility"
        });

        PrivateIncludePaths.AddRange(new string[]
        {
            "SwarmSurvivor/Combat",
            "SwarmSurvivor/Character",
            "SwarmSurvivor/Component",
            "SwarmSurvivor/Game",
            "SwarmSurvivor/UI"
        });

        if (Target.Configuration != UnrealTargetConfiguration.Shipping)
        {
            PublicDefinitions.Add("SS_DEBUG=1");
        }
        else
        {
            PublicDefinitions.Add("SS_DEBUG=0");
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
