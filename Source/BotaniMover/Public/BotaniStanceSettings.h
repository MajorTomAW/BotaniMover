// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/Settings/StanceSettings.h"
#include "BotaniStanceSettings.generated.h"

/** Collection of settings for stance related behavior. */
UCLASS(MinimalAPI, BlueprintType)
class UBotaniStanceSettings : public UStanceSettings
{
	GENERATED_BODY()

public:
	/** If true, Character can walk off a ledge when crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crouch")
	uint8 bCanWalkOffLedgesWhenCrouching : 1 = true;
};
