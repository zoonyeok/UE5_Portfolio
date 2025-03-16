// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Z1 : ModuleRules
{
	public Z1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
           new string[] {
            "Z1"
           }
       );

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            //"GameFramework",
            "AIModule",
            "GameplayTasks",
            "Slate", 
            "SlateCore",
            "UMG",
            "GameplayTags",
            "OnlineSubsystem",
            "DeveloperSettings",
            "SignificanceManager",
            "EnhancedInput",
            "Niagara",
            "CoreOnline"
        });

   //     PrivateDependencyModuleNames.AddRange(new string[]
   //      {
   //         "MoviePlayer", // loading screen
			//"RenderCore" // for PSO caching code access
   //      });
    }
}
