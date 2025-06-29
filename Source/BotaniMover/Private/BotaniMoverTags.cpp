// Copyright © 2025 Playton. All Rights Reserved.


#include "BotaniMoverTags.h"

#define BOTANIMOVER_DEFINE(TagName, Tag, Comment) \
	BOTANIMOVER_API UE_DEFINE_GAMEPLAY_TAG_COMMENT(TagName, Tag, Comment)


namespace BotaniGameplayTags::Mover
{
	namespace Modes
	{
		BOTANIMOVER_DEFINE(TAG_MM_Walking, "BotaniMover.Mode.Walking", "Walking mode, used for normal movement.");
		BOTANIMOVER_DEFINE(TAG_MM_Sprinting, "BotaniMover.Mode.Sprinting", "Sprinting mode, used for fast movement.");
		BOTANIMOVER_DEFINE(TAG_MM_Crouching, "BotaniMover.Mode.Crouching", "Crouching mode, used for stealth or low-profile movement.");
		BOTANIMOVER_DEFINE(TAG_MM_Falling, "BotaniMover.Mode.Falling", "Falling mode, used when the character is in free fall.");
		BOTANIMOVER_DEFINE(TAG_MM_Sliding, "BotaniMover.Mode.Sliding", "Sliding mode, used for fast movement on the ground.");
		BOTANIMOVER_DEFINE(TAG_MM_PowerSliding, "BotaniMover.Mode.PowerSliding", "Power sliding mode, used for fast movement on the ground with a boost.");
		BOTANIMOVER_DEFINE(TAG_MM_WallRunning, "BotaniMover.Mode.WallRunning", "Wall running mode, used for fast movement along walls.");
		BOTANIMOVER_DEFINE(TAG_MM_Grappling, "BotaniMover.Mode.Grappling", "Grappling mode, used for fast movement using a grappling hook.");
	}
}

#undef BOTANIMOVER_DEFINE