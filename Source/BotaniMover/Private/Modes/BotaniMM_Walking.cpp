// Copyright © 2025 Playton. All Rights Reserved.


#include "Modes/BotaniMM_Walking.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniMoverInputs.h"
#include "BotaniMoverTags.h"
#include "MoverComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/BotaniMoverComponent.h"
#include "MoveLibrary/GroundMovementUtils.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_Walking)

UBotaniMM_Walking::UBotaniMM_Walking(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SlopeBoostMultiplier = 1.f;
	
	ModeTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Walking;
	SprintingTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Sprinting;
	
	Sprint_MaxSpeed = 1000.f;
	Sprint_Acceleration = 4000.f;
	Sprint_Deceleration = 200.f;
	Sprint_TurningRate = 360.f;
	Sprint_TurnRateBoost = 3.f;
}

void UBotaniMM_Walking::GenerateMove_Implementation(
	const FMoverTickStartData& StartState,
	const FMoverTimeStep& TimeStep,
	FProposedMove& OutProposedMove) const
{
	// Get the inputs
	const FCharacterDefaultInputs* MoveKinematicInputs = StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();
	const FBotaniMoverInputs* BotaniInputs = StartState.InputCmd.InputCollection.FindDataByType<FBotaniMoverInputs>();

	// Get the sync states
	const FMoverDefaultSyncState* StartSyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(StartSyncState);

	// Get the blackboard
	UMoverBlackboard* Blackboard = GetMoverComponent<UMoverComponent>()->GetSimBlackboard_Mutable();
	check(Blackboard);

	// Get the mover component
	UBotaniMoverComponent* BotaniMover = Cast<UBotaniMoverComponent>(GetMoverComponent());
	check(BotaniMover);

	// D: what is up ??
	FVector UpDirection = BotaniMover->GetUpDirection();

	// Get timings
	const float DeltaSeconds = TimeStep.StepMs * 0.001f;

	// Start filling up our move params
	FFloorCheckResult LastFloorResult;
	FVector MovementNormal;

	// Look for a walkable floor on the blackboard so we can walk along slopes
	if (Blackboard->TryGet(CommonBlackboard::LastFloorResult, LastFloorResult)
		&& LastFloorResult.IsWalkableFloor())
	{
		// Use the floor result normal as the movement normal
		MovementNormal = LastFloorResult.HitResult.ImpactNormal;
	}
	else
	{
		// No floor, so default to the up direction
		MovementNormal = UpDirection;
	}

	FRotator IntendedOrientation_WorldSpace;

	// Do we have orientation intent?
	if (!MoveKinematicInputs || MoveKinematicInputs->OrientationIntent.IsNearlyZero())
	{
		// No orientation intent, so default to the last frame's orientation
		IntendedOrientation_WorldSpace = StartSyncState->GetOrientation_WorldSpace();
	}
	else
	{
		// Use the input orientation
		IntendedOrientation_WorldSpace = MoveKinematicInputs->GetOrientationIntentDir_WorldSpace().ToOrientationRotator();
	}

	// Calculate a speed boost scaling so we better retain movement plane speed on steep slopes (up to a point)
	float SlopeBoost = 1.0f / FMath::Max(FMath::Abs(MovementNormal.Dot(FVector::UpVector)), 0.75f);

	// Start building our ground move params
	FGroundMoveParams Params;

	// Do we have an input?
	if (MoveKinematicInputs)
	{
		// Use the move input
		Params.MoveInputType = MoveKinematicInputs->GetMoveInputType();
		Params.MoveInput = MoveKinematicInputs->GetMoveInput();
	}
	else
	{
		// Default to a null input
		Params.MoveInputType = EMoveInputType::None;
		Params.MoveInput = FVector::ZeroVector;
	}

	// Set the rest of the ground move params
	Params.OrientationIntent = IntendedOrientation_WorldSpace;
	Params.PriorVelocity = FVector::VectorPlaneProject(StartSyncState->GetVelocity_WorldSpace(), MovementNormal);
	Params.PriorOrientation = StartSyncState->GetOrientation_WorldSpace();
	Params.GroundNormal = MovementNormal;
	Params.DeltaSeconds = DeltaSeconds;
	Params.WorldToGravityQuat = BotaniMover->GetWorldToGravityTransform();
	Params.UpDirection = UpDirection;
	Params.bUseAccelerationForVelocityMove = CommonLegacySettings->bUseAccelerationForVelocityMove;

	const bool bSprinting = BotaniInputs && BotaniInputs->bIsSprintPressed;

	// Decide whether to use walk or sprinting params
	if (bSprinting)
	{
		// Use the sprinting params
		Params.TurningRate = Sprint_TurningRate;
		Params.TurningBoost = Sprint_TurnRateBoost;
		Params.MaxSpeed = Sprint_MaxSpeed * SlopeBoost;
		Params.Acceleration = Sprint_Acceleration * SlopeBoost;
		Params.Deceleration = Sprint_Deceleration;
	}
	else
	{
		// Use the default legacy walk params
		Params.TurningRate = CommonLegacySettings->TurningRate;
		Params.TurningBoost = CommonLegacySettings->TurningBoost;
		Params.MaxSpeed = CommonLegacySettings->MaxSpeed /** SlopeBoost*/;
		Params.Acceleration = CommonLegacySettings->Acceleration /** SlopeBoost*/;
		Params.Deceleration = CommonLegacySettings->Deceleration;
	}

	// Make sure we don't exceed the max speed
	if (Params.MoveInput.SizeSquared() > 0.f
		&& !UMovementUtils::IsExceedingMaxSpeed(Params.PriorVelocity, Params.MaxSpeed))
	{
		// Default to regular friction
		Params.Friction = CommonLegacySettings->GroundFriction;
	}
	else
	{
		// Use the braking friction to slow down back to the max speed
		Params.Friction = CommonLegacySettings->bUseSeparateBrakingFriction ? CommonLegacySettings->BrakingFriction : CommonLegacySettings->GroundFriction;
		Params.Friction *= CommonLegacySettings->BrakingFrictionFactor;
	}

	// Output the proposed move
	OutProposedMove = UGroundMovementUtils::ComputeControlledGroundMove(Params);
}

void UBotaniMM_Walking::PostMove(FMoverTickEndData& OutputState)
{
	Super::PostMove(OutputState);

	// Add the sprinting tag if necessary
	OutTagsSyncState->AddTag(SprintingTag);

	// Have we started sprinting?
	if (OutTagsSyncState->HasTagExact(SprintingTag)
		&& !TagsSyncState->HasTagExact(SprintingTag))
	{
		// Send the sprinting event
		if (SprintStartEventTag.IsValid())
		{
			FGameplayEventData Payload;
			Payload.EventTag = SprintStartEventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MutableMoverComponent->GetOwner(), SprintStartEventTag, Payload);
		}
	}
	// Are we done sprinting?
	else if (!OutTagsSyncState->HasTagExact(SprintingTag) &&
		TagsSyncState->HasTagExact(SprintingTag))
	{
		// Send the sprinting end event
		if (SprintStopEventTag.IsValid())
		{
			FGameplayEventData Payload;
			Payload.EventTag = SprintStopEventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MutableMoverComponent->GetOwner(), SprintStopEventTag, Payload);
		}
	}
}

bool UBotaniMM_Walking::HandleFalling(
	FMoverTickEndData& OutputState,
	FMovementRecord& MoveRecord,
	FHitResult& Hit,
	float TimeAppliedSoFar)
{
	if (!Super::HandleFalling(OutputState, MoveRecord, Hit, TimeAppliedSoFar))
	{
		return false;
	}

	// Send the sprint end event
	if (SprintStopEventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = SprintStopEventTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MutableMoverComponent->GetOwner(), SprintStopEventTag, Payload);
	}

	return true;
}

bool UBotaniMM_Walking::CheckIfMovementDisabled()
{
	return false;
}
