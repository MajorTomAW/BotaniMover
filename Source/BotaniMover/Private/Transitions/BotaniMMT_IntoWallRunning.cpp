// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_IntoWallRunning.h"

#include "BotaniMoverLogChannels.h"
#include "BotaniMoverVLogHelpers.h"
#include "BotaniWallRunMovementSettings.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "Library/CommonMovementCheckUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_IntoWallRunning)

UBotaniMMT_IntoWallRunning::UBotaniMMT_IntoWallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BlackboardTimeLoggingKey = BotaniMover::Blackboard::LastWallRunStartTime;
#if WITH_EDITORONLY_DATA
	bShouldCreateVisLogEntry = true;
#endif
}


FTransitionEvalResult UBotaniMMT_IntoWallRunning::Evaluate_Implementation(
	const FSimulationTickParams& Params) const
{
	const FTransitionEvalResult NoTransition = FTransitionEvalResult::NoTransition;
	const FTransitionEvalResult WallRunningTransition = FTransitionEvalResult(WallRunningMovementMode);

	// No need to run the entire evaluation if the wall running mode is not set
	if (WallRunningMovementMode.IsNone())
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

	// (Double-) check if we are actually falling?
	if (!UCommonMovementCheckUtils::IsFalling(Params))
	{
		return NoTransition;
	}

	// (Double-) check if we are already wall running?
	if (UWallRunningMovementUtils::IsWallRunning(Params))
	{
		return NoTransition;
	}

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Check the last time we were wall running and make sure we aren't on cooldown
	float LastWallRunTimeMs = 0.f;
	if (SimBlackboard->TryGet<float>(BotaniMover::Blackboard::LastWallRunTime, LastWallRunTimeMs))
	{
		const float CurrentTimeMs = Params.TimeStep.BaseSimTimeMs;
		const float WallRunDeltaTimeMs = CurrentTimeMs - LastWallRunTimeMs;


		if (WallRunDeltaTimeMs < (GetBotaniWallRunFloatProp(WallRun_MinTimeBetweenRuns) * 1000.f))
		{
/*#if ENABLE_VISUAL_LOG @TODO: Don't want to spam the vis log with this!
			{
				using namespace BotaniMover::VLog;

				VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
					FVLogDrawCommand::DrawDebugCapsule(
						Params.MovingComps.UpdatedComponent.Get(),
						FColor::Red,
						Params.MovingComps.UpdatedComponent->GetComponentQuat(),
						1.f,
						FString::Printf(TEXT(""))));
			}
#endif*/

			BOTANIMOVER_WARN("Can't wall run yet, not enough time passed since last wall run! "
				"Last wall run time: %f, current time: %f, actual time passed: %f",
				LastWallRunTimeMs * BotaniMover::Lazy::MsToS, CurrentTimeMs * BotaniMover::Lazy::MsToS, WallRunDeltaTimeMs* BotaniMover::Lazy::MsToS);
			return NoTransition;
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
				BOTANIMOVER_WARN("Can't wall run, missing required tags! "
					"Required tags: %s, current tags: %s",
					*BotaniWallRunSettings->WallRunningRequiredTags.ToString(),
					*TagsState->GetMovementTags().ToString());
				return NoTransition;
			}
		}

		if (!BotaniWallRunSettings->WallRunningBlockedTags.IsEmpty())
		{
			if (TagsState->GetMovementTags().HasAnyExact(BotaniWallRunSettings->WallRunningBlockedTags))
			{
				BOTANIMOVER_WARN("Can't wall run, blocked by tags! "
					"Blocked tags: %s, current tags: %s",
					*BotaniWallRunSettings->WallRunningBlockedTags.ToString(),
					*TagsState->GetMovementTags().ToString());
				return NoTransition;
			}
		}
	}

	// Check if our vertical speed is too fast to start wall running
	const FVector UpDir = Params.MovingComps.MoverComponent->GetUpDirection();
	const FVector Velocity = Params.MovingComps.UpdatedComponent->GetComponentVelocity();

	auto HorizontalSpeedSquared = Velocity.SizeSquared2D();
	auto VerticalSpeedSquared = (Velocity * UpDir).SizeSquared();

	//@TODO: Move this into UBotaniMMT_OutOfWallRunning
	// We can only wall run if we are moving fast enough horizontally
	if (HorizontalSpeedSquared < pow(GetBotaniWallRunFloatProp(WallRun_MinRequiredSpeed), 2.f) &&
		!BotaniWallRunSettings->bAlwaysStayOnWall)
	{
		BOTANIMOVER_WARN("Can't wall run, not enough horizontal speed! "
			"Horizontal speed squared: %f, required: %f",
			HorizontalSpeedSquared, pow(GetBotaniWallRunFloatProp(WallRun_MinRequiredSpeed), 2.f));
		return NoTransition;
	}

	// We can only wall run if we aren't Falling/moving downwards too fast
	if (VerticalSpeedSquared > pow(GetBotaniWallRunFloatProp(WallRun_MaxVerticalSpeed), 2))
	{
		BOTANIMOVER_WARN("Can't wall run, moving downwards too fast! "
			"Vertical speed squared: %f, required: %f",
			VerticalSpeedSquared, pow(GetBotaniWallRunFloatProp(WallRun_MaxVerticalSpeed), 2));
		return NoTransition;
	}

	// Start tracing for walls to run on
	FHitResult WallHit;
	const bool bCanStartWallRunning = CanStartWallRunning(Params, WallHit);

	if (!bCanStartWallRunning ||!WallHit.IsValidBlockingHit())
	{
		return NoTransition;
	}

	// Check if the wall is not too steep to run on
	// But handle it later
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
		BOTANIMOVER_WARN("Can't wall run, wall is too steep! "
			"Wall angle: %f, required: %f",
			Angle, GetBotaniWallRunFloatProp(WallRun_MinRequiredAngle));
		return NoTransition;
	}

	// Check if the player is facing away from the wall
	if (UWallRunningMovementUtils::ShouldFallOffWall(
		WallHit,
		GetBotaniWallRunFloatProp(WallRun_PullAwayAngle),
		StartingSyncState->GetIntent_WorldSpace()))
	{
		BOTANIMOVER_WARN("Can't wall run, player is facing away from the wall! "
			"Wall angle: %f, pull away angle: %f",
			Angle, GetBotaniWallRunFloatProp(WallRun_PullAwayAngle));
		return NoTransition;
	}

	// Make sure we are high enough above the floor to start wall running
	if (!UWallRunningMovementUtils::IsHighEnoughForWallRun(
		Params.MovingComps,
		GetBotaniWallRunFloatProp(WallRun_MinRequiredStaticHeight),
		UpDir))
	{
		BOTANIMOVER_WARN("Can't wall run, not high enough above the floor! "
			"Minimum required height: %f",
			GetBotaniWallRunFloatProp(WallRun_MinRequiredStaticHeight));
		return NoTransition;
	}

	// All checks passed, so we can wall run!
	return WallRunningTransition;
}

void UBotaniMMT_IntoWallRunning::Trigger_Implementation(
	const FSimulationTickParams& Params)
{
	// Get the blackboard
	UMoverBlackboard* SimBlackboard =
		Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Save the last wall run time into the blackboard
	if (IsValid(SimBlackboard))
	{
		SimBlackboard->Set<float>(BotaniMover::Blackboard::LastWallRunTime, Params.TimeStep.BaseSimTimeMs);
	}

#if ENABLE_VISUAL_LOG
	if (bShouldCreateVisLogEntry)
	{
		FWallCheckResult CurrentWall;
		if (IsValid(SimBlackboard) && SimBlackboard->TryGet<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, CurrentWall))
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawArrow(
					Params.MovingComps.UpdatedComponent->GetComponentLocation(),
					CurrentWall.GetHitResult().ImpactPoint,
					FColor::Red,
					FString::Printf(TEXT("Wall Hit!\n\tDistance: %.2f"),
						CurrentWall.GetDistanceToWall())));
		}
		else
		{
			BOTANIMOVER_WARN("No valid wall check result found in the blackboard!");
		}
	}
#endif

	// Save the wall run time, perform events and vis-logging
	Super::Trigger_Implementation(Params);
}

