// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_OutOfWallRunning.h"

#include "BotaniMoverLogChannels.h"
#include "BotaniMoverVLogHelpers.h"
#include "BotaniWallRunMovementSettings.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "MoveLibrary/WallRunningMovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_OutOfWallRunning)

UBotaniMMT_OutOfWallRunning::UBotaniMMT_OutOfWallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BlackboardTimeLoggingKey = BotaniMover::Blackboard::LastWallRunTime;
#if WITH_EDITORONLY_DATA
	bShouldCreateVisLogEntry = true;
#endif
}

FTransitionEvalResult UBotaniMMT_OutOfWallRunning::Evaluate_Implementation(
	const FSimulationTickParams& Params) const
{
	const FTransitionEvalResult NoTransition = FTransitionEvalResult::NoTransition;
	const FTransitionEvalResult FallingTransition = FTransitionEvalResult(FallingMovementMode);

	// No need to run the entire evaluation if the wall running mode is not set
	if (FallingMovementMode.IsNone())
	{
		return NoTransition;
	}

	// Get the start state
	const FMoverTickStartData& StartState = Params.StartState;
	const FMoverDefaultSyncState* StartingSyncState =
		StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	if (!StartingSyncState)
	{
		return NoTransition;
	}

	// (Double-) check if we are actually wall running?
	if (!UWallRunningMovementUtils::IsWallRunning(Params))
	{
		return NoTransition;
	}

	// Get the blackboard
	UMoverBlackboard* SimBlackboard =
		Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Check for overall maximum wall run time
	float LastWallRunStartTime = 0.f;
	if (SimBlackboard->TryGet<float>(BotaniMover::Blackboard::LastWallRunStartTime, LastWallRunStartTime))
	{
		if (BotaniWallRunSettings->WallRun_MaxTime.IsSet())
		{
			const float CurrentTime = Params.TimeStep.BaseSimTimeMs;
			const float WallRunDuration = CurrentTime - LastWallRunStartTime;

			// Omg "GetValue().GetValue()" (first one is for TOptional, second one for FScalableFloat)
			const float MaxWallRunDuration =
				BotaniWallRunSettings->WallRun_MaxTime.GetValue().GetValue() * BotaniMover::Lazy::SToMs; // Convert seconds to milliseconds

			// As we are using "TOptional" it wouldn't make sense checking against negative values here,
			// but I need to be able to assign my curve table values to it, which makes TOptional in that context useless.
			if ((MaxWallRunDuration > 0.f) &&
				(WallRunDuration >= MaxWallRunDuration))
			{
				BOTANIMOVER_WARN("Can't continue wall running, max time exceeded! WallRunDuration: %.3fs, MaxTime: %.3fs",
					WallRunDuration * BotaniMover::Lazy::MsToS, MaxWallRunDuration * BotaniMover::Lazy::MsToS);

				return FallingTransition;
			}
		}
	}

	// Get the gameplay tags sync state
	if (const FGameplayTagsSyncState* TagsState =
		StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>())
	{
		if (!BotaniWallRunSettings->WallRunningRequiredTags.IsEmpty())
		{
			if (!TagsState->GetMovementTags().HasAllExact(BotaniWallRunSettings->WallRunningRequiredTags))
			{
				BOTANIMOVER_WARN("Can't continue wall running, missing required tags! "
					"Required tags: %s, current tags: %s",
					*BotaniWallRunSettings->WallRunningRequiredTags.ToString(),
					*TagsState->GetMovementTags().ToString());
				return FallingTransition;
			}
		}

		if (!BotaniWallRunSettings->WallRunningBlockedTags.IsEmpty())
		{
			if (TagsState->GetMovementTags().HasAnyExact(BotaniWallRunSettings->WallRunningBlockedTags))
			{
				BOTANIMOVER_WARN("Can't continue wall running, blocked by tags! "
					"Blocked tags: %s, current tags: %s",
					*BotaniWallRunSettings->WallRunningBlockedTags.ToString(),
					*TagsState->GetMovementTags().ToString());
				return FallingTransition;
			}
		}
	}

	// Now that we checked for early-out conditions, we can start checking for the wall to run on
	FHitResult WallHit;
	const bool bCanStartWallRunning = CanStartWallRunning(Params, WallHit);

	if (!bCanStartWallRunning ||!WallHit.IsValidBlockingHit())
	{
		return FallingTransition;
	}

	// Check if the wall is not too steep to run on
	// But handle it later
	const FVector UpDir = Params.MovingComps.MoverComponent->GetUpDirection();
	const float Angle = UWallRunningMovementUtils::GetWallAngle(WallHit, UpDir);
	const bool bIsWallTooSteep = Angle < GetBotaniWallRunFloatProp(WallRun_MinRequiredAngle);

	FWallCheckResult CurrentWall;
	CurrentWall.SetFromHitResult(WallHit, WallHit.Distance, (bCanStartWallRunning && !bIsWallTooSteep));

	// Save the wall result to the blackboard
	if (ensure(IsValid(SimBlackboard)))
	{
		SimBlackboard->Set<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, CurrentWall);
	}

	// Handle to steep walls now
	if (bIsWallTooSteep)
	{
		BOTANIMOVER_WARN("Can't continue wall running, wall is too steep! "
			"Wall angle: %f, required: %f",
			Angle, GetBotaniWallRunFloatProp(WallRun_MinRequiredAngle));
		return FallingTransition;
	}

	// Check if the player is facing away from the wall
	if (UWallRunningMovementUtils::ShouldFallOffWall(
		WallHit,
		GetBotaniWallRunFloatProp(WallRun_PullAwayAngle),
		StartingSyncState->GetIntent_WorldSpace()) &&
		!BotaniWallRunSettings->bAlwaysStayOnWall)
	{
		BOTANIMOVER_WARN("Can't continue wall running, player is facing away from the wall! "
			"Wall angle: %f, pull away angle: %f",
			Angle, GetBotaniWallRunFloatProp(WallRun_PullAwayAngle));
		return FallingTransition;
	}

	// Make sure we are high enough above the floor to start wall running
	if (!UWallRunningMovementUtils::IsHighEnoughForWallRun(
		Params.MovingComps,
		GetBotaniWallRunFloatProp(WallRun_MinRequiredDynamicHeight),
		UpDir))
	{
		BOTANIMOVER_WARN("Can't continue wall running, not high enough above the floor! "
			"Minimum required height: %f",
			GetBotaniWallRunFloatProp(WallRun_MinRequiredDynamicHeight));
		return FallingTransition;
	}

	// All checks passed, so we can continue wall running!
	// (No transitioning)
	return NoTransition;
}

void UBotaniMMT_OutOfWallRunning::Trigger_Implementation(
	const FSimulationTickParams& Params)
{
	// Save the wall run time, perform events and vis-logging
	Super::Trigger_Implementation(Params);
}
