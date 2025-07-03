// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BotaniMoverTags.h"
#include "MovementMode.h"
#include "UObject/Object.h"

#include "BotaniMoverSettings.generated.h"

/** BotaniMover-specific movement mode names. */
namespace BotaniMover::ModeNames
{
	const FName NAME_Grappling = TEXT("Grappling");
	const FName NAME_Teleport = TEXT("Teleport");
	const FName NAME_WallRunning = TEXT("WallRunning");
}

/** BotaniMover-specific blackboard keys. */
namespace BotaniMover::Blackboard
{
	const FName NAME_LastFallTime = TEXT("LastFallTime");
	const FName NAME_LastJumpTime = TEXT("LastJumpTime");
	const FName NAME_GrappleTarget = TEXT("GrappleTarget");
	const FName NAME_GrappleNormal = TEXT("GrappleNormal");
	const FName NAME_GrappleStartTime = TEXT("GrappleStartTime");
	const FName NAME_LastGrappleTime = TEXT("LastGrappleTime");
}

/** Shared settings for Botani mover. */
UCLASS(BlueprintType)
class BOTANIMOVER_API UBotaniMoverSettings
	: public UObject
	, public IMovementSettingsInterface
{
	GENERATED_BODY()

public:
	UBotaniMoverSettings();

	//~ Begin IMovementSettingsInterface
	virtual FString GetDisplayName() const override { return GetName(); }
	//~ End IMovementSettingsInterface

public:
	/** MovementMode name to use when grappling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Modes)
	FName GrapplingModeName = BotaniMover::ModeNames::NAME_Grappling;

	/** MovementMode name to use when teleporting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Modes)
	FName TeleportModeName = BotaniMover::ModeNames::NAME_Teleport;

	/** GameplayTag to use when falling or jumping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag FallingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Falling;

	/** GameplayTag to use when walking on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag WalkingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Walking;

	/** GameplayTag to use when sprinting on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag SprintingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Sprinting;

	/** GameplayTag to use when crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag CrouchingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Crouching;

	/** GameplayTag to use when sliding on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag SlidingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Sliding;

	/** GameplayTag to use when power sliding on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag PowerSlidingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_PowerSliding;

	/** GameplayTag to use when grappling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag GrapplingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Grappling;

	/** GameplayTag to use when wall running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag WallRunningTag = BotaniGameplayTags::Mover::Modes::TAG_MM_WallRunning;
};
