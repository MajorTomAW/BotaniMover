// Author: Tom Werner (MajorT), 2025

#pragma once

#include "NativeGameplayTags.h"

#define BOTANIMOVER_DECLARE(TagName) \
	BOTANIMOVER_API extern FNativeGameplayTag TagName;

namespace BotaniGameplayTags::Mover
{
	namespace Modes
	{
		BOTANIMOVER_DECLARE(TAG_MM_Walking)
		BOTANIMOVER_DECLARE(TAG_MM_Sprinting)
		BOTANIMOVER_DECLARE(TAG_MM_Crouching)
		BOTANIMOVER_DECLARE(TAG_MM_Falling)
		BOTANIMOVER_DECLARE(TAG_MM_Sliding)
		BOTANIMOVER_DECLARE(TAG_MM_PowerSliding)
		BOTANIMOVER_DECLARE(TAG_MM_WallRunning)
		BOTANIMOVER_DECLARE(TAG_MM_Grappling)
	}

	namespace Transitions
	{
	}

	namespace Restrictions
	{
		BOTANIMOVER_DECLARE(TAG_Restriction_CannotMove)
	}
}

#undef BOTANIMOVER_DECLARE
