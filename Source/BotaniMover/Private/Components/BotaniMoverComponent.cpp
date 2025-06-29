// Copyright © 2025 Playton. All Rights Reserved.


#include "Components/BotaniMoverComponent.h"

#include "Modes/BotaniMM_Falling.h"
#include "Modes/BotaniMM_Walking.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMoverComponent)

UBotaniMoverComponent::UBotaniMoverComponent()
{
	// Default movement modes
	MovementModes.Add(DefaultModeNames::Walking, CreateDefaultSubobject<UBotaniMM_Walking>("DefaultWalkingMode"));
	MovementModes.Add(DefaultModeNames::Falling, CreateDefaultSubobject<UBotaniMM_Falling>("DefaultFallingMode"));

	PersistentSyncStateDataTypes.Add(FMoverDataPersistence(FGameplayTagsSyncState::StaticStruct(), false));

	StartingMovementMode = DefaultModeNames::Falling;
}

void UBotaniMoverComponent::BeginPlay()
{
	Super::BeginPlay();
}
