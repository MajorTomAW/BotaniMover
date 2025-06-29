// Copyright © 2025 Playton. All Rights Reserved.


#include "Modes/BotaniMM_GroundBase.h"

#include "BotaniMoverSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_GroundBase)

UBotaniMM_GroundBase::UBotaniMM_GroundBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniMoverSettings::StaticClass());
}
