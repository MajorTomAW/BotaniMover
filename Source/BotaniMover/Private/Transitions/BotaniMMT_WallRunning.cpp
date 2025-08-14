// Copyright © 2025 Playton. All Rights Reserved.


#include "Transitions/BotaniMMT_WallRunning.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "BotaniMoverVLogHelpers.h"
#include "BotaniWallRunMovementSettings.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "Components/CapsuleComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Library/CommonMovementCheckUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/WallRunningMovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_WallRunning)

UDEPRECATED_BotaniMMT_WallRunning::UDEPRECATED_BotaniMMT_WallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WallRunningMovementMode = BotaniMover::ModeNames::WallRunning;
	WallRunningEndMovementMode = DefaultModeNames::Falling;

	bKeepPreviousVelocity = true;
	bKeepPreviousVerticalVelocity = true;
	bAddFloorVelocity = true;

	SharedSettingsClasses.Add(UBotaniWallRunMovementSettings::StaticClass());
}

FTransitionEvalResult UDEPRECATED_BotaniMMT_WallRunning::Evaluate_Implementation(const FSimulationTickParams& Params) const
{
	// Get the default kinematic inputs
	const FCharacterDefaultInputs* KinematicInputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Get the wall running settings
	const UBotaniWallRunMovementSettings* BotaniWallRunSettings = Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();

	auto NoTransition = FTransitionEvalResult::NoTransition;
	FTransitionEvalResult TransitionTo_Falling = WallRunningEndMovementMode;
	FTransitionEvalResult TransitionTo_WallRunning = WallRunningMovementMode;

	// Get the current movement mode name
	const bool bIsFalling = UCommonMovementCheckUtils::IsFalling(Params);
	const bool bIsWallRunning = UWallRunningMovementUtils::IsWallRunning(Params);
	if (!(bIsFalling || bIsWallRunning))
	{
		return NoTransition;
	}

	// Make sure we aren't trying to wall run again too fast
	//@TODO: This is currently disabled, since there are bigger issues: multiple transitions into and out of wall running being called
	/*float LastWallRunTime = 0.f;
	if (bIsFalling && !bIsWallRunning && SimBlackboard->TryGet<float>(BotaniMover::Blackboard::LastWallRunTime, LastWallRunTime))
	{
		if ((Params.TimeStep.BaseSimTimeMs - LastWallRunTime) < GetBotaniWallRunFloatProp(WallRun_MinTimeBetweenRuns))
		{
#if ENABLE_VISUAL_LOG
			{
				using namespace BotaniMover::VLog;

				VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
					FVLogDrawCommand::DrawDebugCapsule(
						Params.MovingComps.UpdatedComponent.Get(),
						FColor::Red,
						FQuat::Identity,
						1.f,
						FString::Printf(TEXT("Transition into FALLING!\nReason: I'm on cooldown from wall running!\n\tLast run: %f ms ago.")
							, Params.TimeStep.BaseSimTimeMs - LastWallRunTime)));
			}
#endif
			return TransitionTo_Falling;
		}
	}*/

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();

	// Check for required/blocked tags
	if (TagsState)
	{
		if (!BotaniWallRunSettings->WallRunningRequiredTags.IsEmpty())
		{
			if (!TagsState->GetMovementTags().HasAllExact(BotaniWallRunSettings->WallRunningRequiredTags))
			{
#if ENABLE_VISUAL_LOG
				{
					using namespace BotaniMover::VLog;

					VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
						FVLogDrawCommand::DrawDebugCapsule(
							Params.MovingComps.UpdatedComponent.Get(),
							FColor::Red,
							FQuat::Identity,
							1.f,
							FString::Printf(TEXT("Transition into FALLING!\nReason: I am missing required tags for wall running!"))));
				}
#endif
				return TransitionTo_Falling;
			}
		}

		if (!BotaniWallRunSettings->WallRunningBlockedTags.IsEmpty())
		{
			if (TagsState->GetMovementTags().HasAnyExact(BotaniWallRunSettings->WallRunningBlockedTags))
			{
#if ENABLE_VISUAL_LOG
				{
					using namespace BotaniMover::VLog;

					VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
						FVLogDrawCommand::DrawDebugCapsule(
							Params.MovingComps.UpdatedComponent.Get(),
							FColor::Red,
							FQuat::Identity,
							1.f,
							FString::Printf(TEXT("Transition into FALLING!\nReason: I have blocked tags for wall running!"))));
				}
#endif
				return TransitionTo_Falling;
			}
		}
	}

	// Check if we are falling too fast
	const FVector Velocity = Params.MovingComps.UpdatedComponent->GetComponentVelocity();
	if ((Velocity.ProjectOnToNormal(Params.MovingComps.MoverComponent->GetUpDirection()).Size() < -BotaniWallRunSettings->WallRun_MaxVerticalSpeed.GetValue()))
	{
#if ENABLE_VISUAL_LOG
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawDebugCapsule(
					Params.MovingComps.UpdatedComponent.Get(),
					FColor::Red,
					FQuat::Identity,
					1.f,
					FString::Printf(TEXT("Transition into FALLING!\nReason: I am falling too fast! (Velocity: %s)"),
						*Velocity.ToCompactString())));
		}
#endif

		return TransitionTo_Falling;
	}

	// Get the sync state
	const FMoverDefaultSyncState* StartSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(StartSyncState);

	// Get whatever we count as "UP"
	const FVector UpDirection = Params.MovingComps.MoverComponent->GetUpDirection();
	const float VelocityVerticalComponent = FVector::VectorPlaneProject(Velocity, UpDirection).Z;

	FHitResult OutWallHit;
	const bool bCanStartWallRunning = CanStartWallRunning(Params, OutWallHit);

	if (!bCanStartWallRunning || !OutWallHit.IsValidBlockingHit())
	{
#if ENABLE_VISUAL_LOG
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawDebugCapsule(
					Params.MovingComps.UpdatedComponent.Get(),
					FColor::Red,
					FQuat::Identity,
					1.f,
					FString::Printf(TEXT("Transition into FALLING!\nReason: I cannot wall run on this wall!"))));
		}
#endif

		return TransitionTo_Falling;
	}

	// Check the angle of the wall
	const float Angle = UWallRunningMovementUtils::GetWallAngle(OutWallHit, UpDirection);
	const bool bIsWallTooSteep = Angle < GetBotaniWallRunFloatProp(WallRun_MinRequiredAngle);

	FWallCheckResult CurrentWall;
	CurrentWall.SetFromHitResult(OutWallHit, OutWallHit.Distance, (bCanStartWallRunning && !bIsWallTooSteep));

	// We are wall running (or we want to)
	// So save the wall hit result to the blackboard
	if (ensure(IsValid(SimBlackboard)))
	{
		// Save the wall hit result to the blackboard
		SimBlackboard->Set(BotaniMover::Blackboard::LastWallResult, CurrentWall);
	}

	if (bIsWallTooSteep)
	{
#if ENABLE_VISUAL_LOG
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawDebugCapsule(
					Params.MovingComps.UpdatedComponent.Get(),
					FColor::Red,
					FQuat::Identity,
					1.f,
					FString::Printf(TEXT("Transition into FALLING!\nReason: Wall is too steep! Angle: %f"), Angle)));
		}
#endif

		return TransitionTo_Falling;
	}

	// Check if the current velocity is facing away from the wall.
	{
		/*float VelDot = (Velocity.GetSafeNormal2D() | OutWallHit.Normal);
		if (VelDot > 0.f)
		{
			UE_LOG(LogBotaniMover, Warning, TEXT("I am facing away from the wall! VelDot: %f"), VelDot);
			return TransitionTo_Falling;
		}*/

		//float AccelDot = (Params.MovingComps.UpdatedComponent->GetComponentA)
	}

	if (UWallRunningMovementUtils::ShouldFallOffWall(OutWallHit, GetBotaniWallRunFloatProp(WallRun_PullAwayAngle), StartSyncState->GetIntent_WorldSpace())
		&& !UCommonMovementCheckUtils::IsFalling(Params))
	{
#if ENABLE_VISUAL_LOG
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawDebugCapsule(
					Params.MovingComps.UpdatedComponent.Get(),
					FColor::Red,
					FQuat::Identity,
					1.f,
					FString::Printf(TEXT("Transition into FALLING!\nReason: I want to fall off the wall as I am facing away from it!"))));
		}
#endif

		return TransitionTo_Falling;
	}

	/* Make sure we are high enough above the floor to start wall running.
	 *
	 * Height checks should only be done if we aren't moving upwards.
	 * The player could slide upwards until high enough and then start wall running !!
	 */
	const float MinHeight = (VelocityVerticalComponent > 0.f)
		? GetBotaniWallRunFloatProp(WallRun_MinRequiredDynamicHeight)
		: GetBotaniWallRunFloatProp(WallRun_MinRequiredStaticHeight);

	if (!UWallRunningMovementUtils::IsHighEnoughForWallRun(Params.MovingComps, MinHeight, UpDirection)
		&& !UCommonMovementCheckUtils::IsFalling(Params))
	{
#if ENABLE_VISUAL_LOG
		{
			using namespace BotaniMover::VLog;

			VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
				FVLogDrawCommand::DrawDebugCapsule(
					Params.MovingComps.UpdatedComponent.Get(),
					FColor::Red,
					FQuat::Identity,
					1.f,
					FString::Printf(TEXT("Transition into FALLING!\nReason: I am not high enough above the floor to wall run! (Required: %s)"),
						*FString::SanitizeFloat(MinHeight))));
		}
#endif

		return TransitionTo_Falling;
	}

	// All checks passed, we can wall run in case we aren't already doing so!!
	if (UWallRunningMovementUtils::IsWallRunning(Params))
	{
		return NoTransition;
	}

#if ENABLE_VISUAL_LOG
	{
		using namespace BotaniMover::VLog;

		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawDebugCapsule(
				Params.MovingComps.UpdatedComponent.Get(),
				FColor::Green,
				FQuat::Identity,
				1.f,
				FString::Printf(TEXT("Transition into WALL RUNNING!\nReason: I can wall run on this wall! (Angle: %f)"), Angle)));
	}
#endif

	return TransitionTo_WallRunning;
}

void UDEPRECATED_BotaniMMT_WallRunning::Trigger_Implementation(const FSimulationTickParams& Params)
{
	// We do not really need any of this here.
	// Instead, we will manage this in the movement mode itself, rather than the transition.

	// Get the movement settings
	/*const UCommonLegacyMovementSettings* CommonLegacySettings = Params.MovingComps.MoverComponent->FindSharedSettings<UCommonLegacyMovementSettings>();
	check(CommonLegacySettings);

	// Get the tags sync state
	const FGameplayTagsSyncState* TagsState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();
	check(TagsState);

	// Get the sync state
	FMoverDefaultSyncState* StartSyncState = Params.StartState.SyncState.SyncStateCollection.FindMutableDataByType<FMoverDefaultSyncState>();
	check(StartSyncState);

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();

	// Compute the inherited velocity
	FVector InheritedVelocity = FVector::ZeroVector;

	// Get the current floor (wall in our case)
	FFloorCheckResult CurrentFloor;
	if (IsValid(SimBlackboard))
	{
		if (SimBlackboard->TryGet(CommonBlackboard::LastFloorResult, CurrentFloor))
		{
			if (CurrentFloor.IsWalkableFloor())
			{
				if (bAddFloorVelocity && CurrentFloor.HitResult.GetActor())
				{
					// Add the floor velocity to the inherited velocity
					InheritedVelocity = CurrentFloor.HitResult.GetActor()->GetVelocity();
				}
			}
		}
	}

	// Preserve any velocity
	FVector ClampedVelocity = bKeepPreviousVelocity
		? Params.MovingComps.UpdatedComponent->GetComponentVelocity()
		: FVector::ZeroVector;

	// Should we reset the vertical velocity??
	if (!bKeepPreviousVerticalVelocity)
	{
		ClampedVelocity = FVector::VectorPlaneProject(ClampedVelocity, Params.MovingComps.MoverComponent->GetUpDirection());
	}

	// Clamp the vertical speed
	if (WallRun_MaxVerticalSpeed >= 0.f)
	{
		const FVector UpDirection = Params.MovingComps.MoverComponent->GetUpDirection();
		const float VerticalSpeed = ClampedVelocity.ProjectOnToNormal(UpDirection).Size();
		if (VerticalSpeed > WallRun_MaxVerticalSpeed)
		{
			ClampedVelocity -= UpDirection * (VerticalSpeed - WallRun_MaxVerticalSpeed);
		}
	}

	// Sum up all the velocities
	InheritedVelocity += ClampedVelocity;*/

	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();
	if (IsValid(SimBlackboard))
	{
		// Save the last wall run time to the blackboard
		SimBlackboard->Set<float>(BotaniMover::Blackboard::LastWallRunTime, Params.TimeStep.BaseSimTimeMs);
	}

	// Send the trigger event
	if (TriggerEvent.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = TriggerEvent;
		Payload.EventMagnitude = Params.MovingComps.MoverComponent->GetVelocity().Size();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Params.MovingComps.MoverComponent->GetOwner(), Payload.EventTag, Payload);
	}
}

bool UDEPRECATED_BotaniMMT_WallRunning::CanStartWallRunning(
	const FSimulationTickParams& Params,
	FHitResult& OutWallHit) const
{
	// Get the wall running settings
	const UBotaniWallRunMovementSettings* WallRunSettings = Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();

	// Check for valid walls to run on
	FHitResult WallHit;
	const bool bResult = UWallRunningMovementUtils::PerformWallTrace(
		Params.MovingComps,
		WallHit,
		WallRunSettings->WallTraceVectorsHeadDelta,
		WallRunSettings->WallTraceVectorsTailDelta);

	if (bResult)
	{
		OutWallHit = WallHit;
	}

	return bResult;
}
