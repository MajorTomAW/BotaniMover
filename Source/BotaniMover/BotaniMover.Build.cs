// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BotaniMover : ModuleRules
{
	public BotaniMover(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange( new []
		{
			"Core",
			"Mover",
			"CommonMover",
			"GameplayTags",
			"ModularGameplay",
			"MotionWarping",
			"GameplayAbilities",
		});


		PrivateDependencyModuleNames.AddRange( new []
		{
			"CoreUObject",
			"Engine",
			"PhysicsCore",
		});
	}
}
