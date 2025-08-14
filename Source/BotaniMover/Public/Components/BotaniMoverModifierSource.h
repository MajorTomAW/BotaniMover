// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "BotaniMoverModifierSource.generated.h"

UENUM(BlueprintType)
enum class EMoverModifierNetFilter : uint8
{
	None,

	/** Modifier is only applied to the local player */
	LocalOnly,

	/** Modifier is only applied server-side. */
	ServerOnly,

	/** Modifier is applied to all clients, but not the server. */
	ClientOnly,
};

UCLASS(BlueprintType, Blueprintable, Meta = (BlueprintSpawnableComponent))
class UBotaniMoverModifierSource : public UActorComponent
{
	GENERATED_BODY()

public:
	UBotaniMoverModifierSource(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** How the modifier should be filtered for network replication. */
	UPROPERTY(EditDefaultsOnly, Category=MoverModifier)
	EMoverModifierNetFilter NetFilter = EMoverModifierNetFilter::None;
};
