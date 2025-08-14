// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_Jump.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverAbilityInputs.h"
#include "CommonBlackboard.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "LayeredMoves/BotaniLM_MultiJump.h"
#include "MoveLibrary/FloorQueryUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_Jump)

UBotaniMMT_Jump::UBotaniMMT_Jump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	JumpMovementMode = DefaultModeNames::Falling;
	bJumpWhenButtonPressed = true;
}

FTransitionEvalResult UBotaniMMT_Jump::Evaluate_Implementation(
	const FSimulationTickParams& Params) const
{
	// Get the inputs
	const FBotaniMoverAbilityInputs* BotaniAbilityInputs =
		Params.StartState.InputCmd.InputCollection.FindDataByType<FBotaniMoverAbilityInputs>();

	// Do we care about jump button state?
	if (bJumpWhenButtonPressed && BotaniAbilityInputs)
	{
		// Check if jump was just pressed this frame
		if (!BotaniAbilityInputs->bJumpPressedThisFrame)
		{
			return FTransitionEvalResult::NoTransition;
		}
	}

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();

	// Do we have tags to match?
	if (TagsState && !JumpRequiredTags.IsEmpty())
	{
		// CHeck if we have the required tags
		if (!TagsState->GetMovementTags().HasAllExact(JumpRequiredTags))
		{
			return FTransitionEvalResult::NoTransition;
		}
	}

	// Get the botani movement settings
	const UBotaniCommonMovementSettings* BotaniMovementSettings = Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniCommonMovementSettings>();

	// Get the blackboard
	const UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard();

	if (IsValid(SimBlackboard))
	{
		float LastJumpTime = 0.f;
		if (SimBlackboard->TryGet(CommonBlackboard::LastJumpTime, LastJumpTime))
		{
			// Check if we are jumping too fast/soon
			const float TimeSinceLastJump = Params.TimeStep.BaseSimTimeMs - LastJumpTime;
			if (TimeSinceLastJump < GetBotaniMoverFloatProp(MinTimeBetweenJumps) * 1000.f)
			{
				return FTransitionEvalResult::NoTransition;
			}
		}

		if (BotaniMovementSettings->bJumpRequiresGround)
		{
			// Get the current floor check result
			FFloorCheckResult CurrentFloor;

			// Do we have a valid floor check result?
			bool bValidFloor = SimBlackboard->TryGet(CommonBlackboard::LastFloorResult, CurrentFloor);

			// If we don't have a valid floor check result, we can't jump...
			// ...unless we are coyote !!!
			if (!bValidFloor || !CurrentFloor.IsWalkableFloor())
			{
				if (GetBotaniMoverFloatProp(CoyoteTime) > 0.f)
				{
					float LastFallTime = 0.f;
					if (SimBlackboard->TryGet(CommonBlackboard::LastFallTime, LastFallTime))
					{
						const float TimeSinceFall = Params.TimeStep.BaseSimTimeMs - LastFallTime;
						if (TimeSinceFall > GetBotaniMoverFloatProp(CoyoteTime) * 1000.f)
						{
							return FTransitionEvalResult::NoTransition;
						}
					}
					else
					{
						return FTransitionEvalResult::NoTransition;
					}
				}
				else
				{
					return FTransitionEvalResult::NoTransition;
				}
			}
		}
	}

	// All checks passed, we can jump!
	return FTransitionEvalResult(JumpMovementMode);
}

void UBotaniMMT_Jump::Trigger_Implementation(
	const FSimulationTickParams& Params)
{
	// Get the movement settings
	const UBotaniCommonMovementSettings* BotaniMovementSettings = Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniCommonMovementSettings>();
	check(BotaniMovementSettings);

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();
	check(TagsState);

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Compute the inherited velocity
	FVector InheritedVelocity = FVector::ZeroVector;

	// Get the current floor
	FFloorCheckResult CurrentFloor;
	if (IsValid(SimBlackboard))
	{
		if (SimBlackboard->TryGet(CommonBlackboard::LastFloorResult, CurrentFloor))
		{
			if (CurrentFloor.IsWalkableFloor())
			{
				// We're jumping off a walkable floor,
				// so save the last falling time to the blackboard
				SimBlackboard->Set(CommonBlackboard::LastFallTime, Params.TimeStep.BaseSimTimeMs);

				if (bJumpAddsFloorVelocity.Get(BotaniMovementSettings->bJumpAddsFloorVelocity) && CurrentFloor.HitResult.GetActor())
				{
					// Add the floor velocity to the inherited velocity
					InheritedVelocity = CurrentFloor.HitResult.GetActor()->GetVelocity();
				}
			}
		}

		// Save the last jump time to the blackboard
		SimBlackboard->Set(CommonBlackboard::LastJumpTime, Params.TimeStep.BaseSimTimeMs);
	}

	// Preserve any momentum from our current base
	FVector ClampedVelocity = bJumpKeepsPreviousVelocity.Get(BotaniMovementSettings->bJumpKeepsPreviousVelocity)
		? Params.MovingComps.UpdatedComponent->GetComponentVelocity()
		: FVector::ZeroVector;

	// Should we reset the vertical velocity?
	if (!bJumpKeepsPreviousVerticalVelocity.Get(BotaniMovementSettings->bJumpKeepsPreviousVerticalVelocity))
	{
		ClampedVelocity = FVector::VectorPlaneProject(ClampedVelocity, Params.MovingComps.MoverComponent->GetUpDirection());
		//ClampedVelocity.Z = 0.f; // Reset the vertical component
	}

	if (GetBotaniMoverFloatProp(MaxJumpPreviousVelocity) >= 0.f)
	{
		ClampedVelocity = ClampedVelocity.GetClampedToMaxSize(GetBotaniMoverFloatProp(MaxJumpPreviousVelocity));
	}

	InheritedVelocity += ClampedVelocity;

	// Choose the vertical impulse to use
	const float UpwardsSpeed =
		GetBotaniMoverFloatProp(JumpVerticalImpulse) +
		(TagsState->HasTagExact(BotaniMovementSettings->ExtraJumpVerticalImpulseTag)
			? GetBotaniMoverFloatProp(ExtraJumpVerticalImpulse)
			: 0.0f);

	// Create the jump layered move
	TSharedPtr<FBotaniLM_MultiJump> JumpMove = MakeShared<FBotaniLM_MultiJump>();
	JumpMove->UpwardsSpeed = UpwardsSpeed;
	JumpMove->Momentum = InheritedVelocity;
	JumpMove->AirControl = GetBotaniMoverFloatProp(JumpAirControlPct);
	JumpMove->DurationMs = GetBotaniMoverFloatProp(JumpHoldTime) * 1000.0f;
	JumpMove->bTruncateOnJumpRelease = BotaniMovementSettings->bTruncateOnJumpRelease;
	JumpMove->bOverrideHorizontalMomentum = bJumpOverridesMovementPlaneVelocity.Get(BotaniMovementSettings->bJumpOverridesMovementPlaneVelocity);
	JumpMove->bOverrideVerticalMomentum = bJumpOverridesVerticalVelocity.Get(BotaniMovementSettings->bJumpOverridesVerticalVelocity);

	JumpMove->FinishVelocitySettings.FinishVelocityMode = ELayeredMoveFinishVelocityMode::MaintainLastRootMotionVelocity;

	// Queue the layered move to the mover component
	Params.MovingComps.MoverComponent->QueueLayeredMove(JumpMove);

	// Check if we should log the jump time in an additional blackboard key
	if (BlackboardTimeLoggingKey != NAME_None)
	{
		SimBlackboard->Set(BlackboardTimeLoggingKey, Params.TimeStep.BaseSimTimeMs);
	}

	// Do we want to send a trigger event?
	if (TriggerEventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = TriggerEventTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Params.MovingComps.MoverComponent->GetOwner(), TriggerEventTag, Payload);
	}

#if ENABLE_VISUAL_LOG
	//@TODO: Add a visual log entry for the jump
#endif
}
