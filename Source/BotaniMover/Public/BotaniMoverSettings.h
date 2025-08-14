// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMoverTags.h"
#include "MovementMode.h"
#include "UObject/Object.h"
#include "NativeGameplayTags.h"

#include "BotaniMoverSettings.generated.h"

BOTANIMOVER_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Mover_IsFat);

namespace BotaniMover
{
	/** BotaniMover-specific movement mode names. */
	namespace ModeNames
	{
		const FName Grappling = TEXT("Grappling");
		const FName Teleport = TEXT("Teleport");
		const FName WallRunning = TEXT("WallRunning");
	}

	/** BotaniMover-specific blackboard keys. */
	namespace Blackboard
	{
		const FName LastFallTime = TEXT("LastFallTime"); // time for the last fall
		const FName LastWallRunTime = TEXT("LastWallRunTime"); // time for the last wall running interaction
		const FName LastWallRunStartTime = TEXT("LastWallRunStartTime"); // time when the last wall run was started
		const FName LastWallJumpTime = TEXT("LastWallJumpTime"); // time when the last wall jump was triggered
		const FName LastWallResult = TEXT("LastWallResult"); // last successful result for a wall trace
		const FName LastJumpTime = TEXT("LastJumpTime"); // time when the last jump was triggered

		const FName GrappleTarget = TEXT("GrappleTarget");
		const FName GrappleNormal = TEXT("GrappleNormal");
		const FName GrappleStartTime = TEXT("GrappleStartTime");
		const FName LastGrappleTime = TEXT("LastGrappleTime");
	}

	/** Lazyness */
	namespace Lazy
	{
		constexpr double MsToS = 0.001; // 1 millisecond to seconds conversion factor
		constexpr double SToMs = 1000.f; // 1 second to milliseconds conversion factor
	}
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
	/** Name of the movement mode to use when on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes")
	FName GroundMovementModeName = DefaultModeNames::Walking;

	/** Name of the movement mode to use when in the air. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes")
	FName AirMovementModeName = DefaultModeNames::Falling;

	/** MovementMode name to use when grappling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes")
	FName GrapplingModeName = BotaniMover::ModeNames::Grappling;

	/** MovementMode name to use when teleporting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes")
	FName TeleportModeName = BotaniMover::ModeNames::Teleport;

	/** GameplayTag to use when falling or jumping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag FallingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Falling;

	/** GameplayTag to use when walking on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag WalkingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Walking;

	/** GameplayTag to use when sprinting on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag SprintingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Sprinting;

	/** GameplayTag to use when crouching. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag CrouchingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Crouching;

	/** GameplayTag to use when sliding on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag SlidingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Sliding;

	/** GameplayTag to use when power sliding on the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag PowerSlidingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_PowerSliding;

	/** GameplayTag to use when grappling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag GrapplingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Grappling;

	/** GameplayTag to use when wall running. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Modes|Tags")
	FGameplayTag WallRunningTag = BotaniGameplayTags::Mover::Modes::TAG_MM_WallRunning;

	/** GameplayTag that will stop movement if present on the actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Move Restrictions")
	FGameplayTag StopMovementTag = BotaniGameplayTags::Mover::Restrictions::TAG_Restriction_CannotMove;
};
