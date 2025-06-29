// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BotaniMoverTags.h"
#include "CommonGroundModeBase.h"

#include "BotaniMM_GroundBase.generated.h"

/** Ground movement mode base class for the Botani game. */
UCLASS(Abstract)
class BOTANIMOVER_API UBotaniMM_GroundBase : public UCommonGroundModeBase
{
	GENERATED_BODY()
	
public:
	UBotaniMM_GroundBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
