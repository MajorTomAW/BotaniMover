// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_WallJump.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverAbilityInputs.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "BotaniMoverVLogHelpers.h"
#include "BotaniWallRunMovementSettings.h"
#include "CommonBlackboard.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "MoverSimulationTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LayeredMoves/BotaniLM_MultiJump.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_WallJump)

UBotaniMMT_WallJump::UBotaniMMT_WallJump(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WallJumpMovementMode = DefaultModeNames::Falling;
	bJumpWhenButtonPressed = true;
	//BlackboardTimeLoggingKey = BotaniMover::Blackboard::LastWallJumpTime; We already hardcoded that bb key, maybe we have an additional one
}

FTransitionEvalResult UBotaniMMT_WallJump::Evaluate_Implementation(
	const FSimulationTickParams& Params) const
{
	// Get the botani wall run settings
	const UBotaniWallRunMovementSettings* BotaniWallRunSettings =
		Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();
	check(BotaniWallRunSettings);

	// Early out if wall jump is not allowed
	if (!BotaniWallRunSettings->bAllowWallJump)
	{
		return FTransitionEvalResult::NoTransition;
	}

	// Get the inputs
	const FBotaniMoverAbilityInputs* AbilityInputs =
		Params.StartState.InputCmd.InputCollection.FindDataByType<FBotaniMoverAbilityInputs>();

	// Do we care about jump button state?
	if (bJumpWhenButtonPressed && AbilityInputs)
	{
		// Check if the jump was just pressed this frame
		if (!AbilityInputs->bJumpPressedThisFrame)
		{
			return FTransitionEvalResult::NoTransition;
		}
	}

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState =
		Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();

	// Check for required/blocked tags
	if (TagsState)
	{
		if (!WallJumpRequiredTags.IsEmpty())
		{
			// Check if we have all the required tags
			if (!TagsState->GetMovementTags().HasAllExact(WallJumpRequiredTags))
			{
				return FTransitionEvalResult::NoTransition;
			}
		}

		if (!WallJumpBlockedTags.IsEmpty())
		{
			// Check if we have any of the blocked tags
			if (TagsState->GetMovementTags().HasAnyExact(WallJumpBlockedTags))
			{
				return FTransitionEvalResult::NoTransition;
			}
		}
	}

	// Get the blackboard
	const UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard();

	if (IsValid(SimBlackboard))
	{
		//@TODO: Might check for wall jump time here?

		// Get the current wall check result
		FWallCheckResult CurrentWall;

		// Do we have a valid wall check result?
		const bool bValidWall = SimBlackboard->TryGet<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, CurrentWall);

		// If we don't have a valid wall, we cannot wall jump
		if (!bValidWall ||!CurrentWall.IsRunAbleWall())
		{
			return FTransitionEvalResult::NoTransition;
		}
	}

	// All checks passed, so we can wall jump!
	return WallJumpMovementMode;
}

void UBotaniMMT_WallJump::Trigger_Implementation(
	const FSimulationTickParams& Params)
{
	// Get the botani wall run settings
	const UBotaniWallRunMovementSettings* BotaniWallRunSettings =
		Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();
	check(BotaniWallRunSettings);

	// Get the movement settings
	const UBotaniCommonMovementSettings* BotaniMovementSettings =
		Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniCommonMovementSettings>();
	check(BotaniMovementSettings);

	// Get the kinematic inputs
	const FCharacterDefaultInputs* KinematicInputs =
		Params.StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState =
		Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();
	check(TagsState);

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Compute the inherited velocity
	FVector InheritedVelocity = FVector::ZeroVector;

	// Get the current wall (in our case)
	FWallCheckResult CurrentWall;
	if (ensure(IsValid(SimBlackboard)))
	{
		const bool bValidWall = SimBlackboard->TryGet<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, CurrentWall);

		if (bValidWall && CurrentWall.IsRunAbleWall())
		{
			// We're jumping off a wall-runnable wall,
			// so save the last falling time to the blackboard
			SimBlackboard->Set(CommonBlackboard::LastFallTime, Params.TimeStep.BaseSimTimeMs);

			if (bWallJumpAddsFloorVelocity.Get(BotaniWallRunSettings->bWallJumpAddsFloorVelocity) && CurrentWall.GetHitResult().GetActor())
			{
				// Add the wall velocity to the inherited velocity
				InheritedVelocity = CurrentWall.GetHitResult().GetActor()->GetVelocity();
			}
		}

		// Save the last wall jump time to the blackboard
		SimBlackboard->Set(BotaniMover::Blackboard::LastWallJumpTime, Params.TimeStep.BaseSimTimeMs);
	}

	// Preserve any momentum from our current base (if any)
	FVector ClampedVelocity = bWallJumpKeepsPreviousVelocity.Get(BotaniWallRunSettings->bWallJumpKeepsPreviousVelocity)
		? Params.MovingComps.UpdatedComponent->GetComponentVelocity()
		: FVector::ZeroVector;

	// Should we reset the vertical velocity??
	if (!bWallJumpKeepsPreviousVerticalVelocity.Get(BotaniWallRunSettings->bWallJumpKeepsPreviousVerticalVelocity))
	{
		ClampedVelocity = FVector::VectorPlaneProject(ClampedVelocity, Params.MovingComps.MoverComponent->GetUpDirection());
	}

	//@TODO: Clamp previous velocity?


	// Add both velocities together
	InheritedVelocity += ClampedVelocity;

	// Rotate the arcade force towards the orientation intent
	const FRotator OrientationIntent = KinematicInputs ? KinematicInputs->GetOrientationIntentDir_WorldSpace().ToOrientationRotator() : FRotator::ZeroRotator;
	const FVector ArcadeForce = OrientationIntent.RotateVector(BotaniWallRunSettings->WallJump_ArcadeForce);

	// Add velocity based on the wall normal and the jump speed
	const FVector JumpVelocity =
		( CurrentWall.GetHitResult().ImpactNormal.GetSafeNormal() * GetBotaniWallRunFloatProp(WallJump_ForceMagnitude) ) +
		ArcadeForce;

#if ENABLE_VISUAL_LOG
	// Draw 3 debug arrows for the Arcade force, one arrow per axis
	{
		using namespace BotaniMover::VLog;

		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawArrow(
				Params.MovingComps.UpdatedComponent->GetComponentLocation(),
				Params.MovingComps.UpdatedComponent->GetComponentLocation() + FVector(JumpVelocity.X, 0.f, 0.f),
				FColor::Red));
		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawArrow(
				Params.MovingComps.UpdatedComponent->GetComponentLocation(),
				Params.MovingComps.UpdatedComponent->GetComponentLocation() + FVector(0.f, JumpVelocity.Y,  0.f),
				FColor::Green));
		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawArrow(
				Params.MovingComps.UpdatedComponent->GetComponentLocation(),
				Params.MovingComps.UpdatedComponent->GetComponentLocation() + FVector(0.f, 0.f, JumpVelocity.Z),
				FColor::Blue));
		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawArrow(
				Params.MovingComps.UpdatedComponent->GetComponentLocation(),
				Params.MovingComps.UpdatedComponent->GetComponentLocation() + InheritedVelocity + JumpVelocity,
				FColor::Yellow));
		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawDebugCapsule(
				Params.MovingComps.UpdatedComponent.Get(),
				FColor::Green,
				Params.MovingComps.UpdatedComponent->GetComponentQuat()));
	}
#endif


	// Create the jump layered move
	TSharedPtr<FBotaniLM_MultiJump> JumpMove = MakeShared<FBotaniLM_MultiJump>();
	JumpMove->UpwardsSpeed = 100.f; // No upwards speed, we are jumping off a wall @TODO: Make this variable
	JumpMove->Momentum = InheritedVelocity + JumpVelocity;
	JumpMove->AirControl = GetBotaniMoverFloatProp(JumpAirControlPct);
	JumpMove->DurationMs = GetBotaniMoverFloatProp(JumpHoldTime) * 1000.0f;
	JumpMove->bTruncateOnJumpRelease = BotaniMovementSettings->bTruncateOnJumpRelease;
	JumpMove->bOverrideHorizontalMomentum = bWallJumpOverridesMovementPlaneVelocity.Get(!BotaniWallRunSettings->bWallJumpKeepsPreviousVelocity);
	JumpMove->bOverrideVerticalMomentum = bWallJumpOverridesVerticalVelocity.Get(!BotaniWallRunSettings->bWallJumpKeepsPreviousVerticalVelocity);

	JumpMove->FinishVelocitySettings.FinishVelocityMode = ELayeredMoveFinishVelocityMode::MaintainLastRootMotionVelocity;

	// Queue the layered move to the mover component
	Params.MovingComps.MoverComponent->QueueLayeredMove(JumpMove);

	// Check if we should log the wall jump time in an additional blackboard key
	if (BlackboardTimeLoggingKey != NAME_None)
	{
		SimBlackboard->Set(BlackboardTimeLoggingKey, Params.TimeStep.BaseSimTimeMs);
	}

	// Send a trigger gameplay event
	if (TriggerEventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = TriggerEventTag;
		Payload.EventMagnitude = JumpMove->Momentum.Size();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Params.MovingComps.MoverComponent->GetOwner(), TriggerEventTag, Payload);
	}

#if ENABLE_VISUAL_LOG
	{
		const FString OverrideHorizontal = JumpMove->bOverrideHorizontalMomentum ? TEXT("Override") : TEXT("Keep");
		const FString OverrideVertical = JumpMove->bOverrideVerticalMomentum ? TEXT("Override") : TEXT("Keep");

		UE_VLOG(Params.MovingComps.MoverComponent->GetOwner(),
			VLogBotaniMover,
			Log,
			TEXT("WallJump Transition\nMomentum[%s]\nAirControl[%f]\nOverride Horizontal[%s]\nOverride Vertical[%s]"),
			*JumpMove->Momentum.ToString(),
			JumpMove->AirControl.GetValue(),
			*OverrideHorizontal,
			*OverrideVertical);
	}
#endif
}
