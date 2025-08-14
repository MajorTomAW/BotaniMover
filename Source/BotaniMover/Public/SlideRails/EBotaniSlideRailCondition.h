// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EBotaniSlideRailCondition.generated.h"

/** Enum representing the condition how the player can use a slide rail. */
UENUM(BlueprintType)
enum class EBotaniSlideRailCondition : uint8
{
	/** The player explicitly needs to interact with the slide rail to use it. */
	ExplicitInteract = 0x00,

	/** The player can use the slide rail without any interaction, just by moving into it. */
	AutoUse = 0x01,

	Any = 0x02, // Used for any condition, e.g. when checking if the player can use any slide rail
};
