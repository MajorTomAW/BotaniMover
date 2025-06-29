// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BotaniMover : ModuleRules
{
	public BotaniMover(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange( new []
		{ 
			"Core",
			"Mover",
			"CommonMover",
			"GameplayTags",
		});
			
		
		PrivateDependencyModuleNames.AddRange( new []
		{ 
			"CoreUObject", 
			"Engine", 
			"GameplayAbilities",
		});
	}
}
