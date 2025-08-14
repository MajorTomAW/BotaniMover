// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EBotaniSlideRailType.generated.h"

/** Enum representing the different types of slide rails that the player can interact with. */
UENUM(BlueprintType)
enum class EBotaniSlideRailType : uint8
{
	/** SlideRail, a rail that allows the player to slide on top of it. */
	SlideRail = 0x00,

	/** ZipRail, a rail that allows the player to zipline along it, typically at high speed. */
	ZipRail = 0x01,
};
