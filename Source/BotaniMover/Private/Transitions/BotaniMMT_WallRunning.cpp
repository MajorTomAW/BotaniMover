// Copyright © 2025 Playton. All Rights Reserved.


#include "Transitions/BotaniMMT_WallRunning.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "BotaniWallRunMovementSettings.h"
#include "GameplayTagSyncState.h"
#include "MoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "Library/CommonMovementCheckUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/WallRunningMovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_WallRunning)

UBotaniMMT_WallRunning::UBotaniMMT_WallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WallRunningMovementMode = BotaniMover::ModeNames::NAME_WallRunning;
	WallRunningEndMovementMode = DefaultModeNames::Falling;
	
	bKeepPreviousVelocity = true;
	bKeepPreviousVerticalVelocity = true;

	SharedSettingsClasses.Add(UBotaniWallRunMovementSettings::StaticClass());
}

FTransitionEvalResult UBotaniMMT_WallRunning::Evaluate_Implementation(const FSimulationTickParams& Params) const
{
	// Get the default kinematic inputs
	const FCharacterDefaultInputs* KinematicInputs = Params.StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// Get the wall running settings
	const UBotaniWallRunMovementSettings* WallRunSettings = Params.MovingComps.MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();

	const FTransitionEvalResult NoTransition = FTransitionEvalResult::NoTransition;
	FTransitionEvalResult BackToFalling(WallRunningEndMovementMode);
	FTransitionEvalResult TransitionToWallRunning(WallRunningMovementMode);

	// Get the current movement mode name
	if (!UCommonMovementCheckUtils::IsFalling(Params.StartState.SyncState))
	{
		return NoTransition;
	}

	// Get the sync state tags
	const FGameplayTagsSyncState* TagsState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FGameplayTagsSyncState>();
	
	// Check for required/blocked tags
	if (TagsState)
	{
		if (!WallRunSettings->WallRunningRequiredTags.IsEmpty())
		{
			if (!TagsState->GetMovementTags().HasAllExact(WallRunSettings->WallRunningRequiredTags))
			{
				return BackToFalling;
			}
		}

		if (!WallRunSettings->WallRunningBlockedTags.IsEmpty())
		{
			if (TagsState->GetMovementTags().HasAnyExact(WallRunSettings->WallRunningBlockedTags))
			{
				return BackToFalling;
			}
		}
	}

	// Check if we are falling too fast
	const FVector Velocity = Params.MovingComps.UpdatedComponent->GetComponentVelocity();
	if ((Velocity.ProjectOnToNormal(Params.MovingComps.MoverComponent->GetUpDirection()).Size() < -WallRunSettings->WallRun_MaxVerticalSpeed))
	{
		UE_LOG(LogBotaniMover, Warning, TEXT("I am falling too fast to wall run!"));
		return NoTransition;
	}

	// Get the blackboard
	const UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard();

	// Get the sync state
	const FMoverDefaultSyncState* StartSyncState = Params.StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(StartSyncState);

	// Get whatever we count as "UP"
	const FVector UpDirection = Params.MovingComps.MoverComponent->GetUpDirection();

	FHitResult OutWallHit;
	const bool bCanStartWallRunning = CanStartWallRunning(Params, OutWallHit);

	if (!bCanStartWallRunning || !OutWallHit.IsValidBlockingHit())
	{
		return NoTransition;
	}

	// Check the angle of the wall
	const float Angle = UWallRunningMovementUtils::GetWallAngle(OutWallHit, UpDirection);
	if (Angle < WallRunSettings->WallRun_MinRequiredAngle)
	{
		UE_LOG(LogBotaniMover, Warning, TEXT("I cannot wall run on this wall! Angle: %f"), Angle);
		return NoTransition;
	}

	// Check if the current velocity is facing away from the wall.
	{
		float VelDot = (Velocity.GetSafeNormal2D() | OutWallHit.Normal);
		if (VelDot > 0.f)
		{
			UE_LOG(LogBotaniMover, Warning, TEXT("I am facing away from the wall! VelDot: %f"), VelDot);
			return NoTransition;
		}

		//float AccelDot = (Params.MovingComps.UpdatedComponent->GetComponentA)
	}

	auto ConditionedReturn = [&]()
	{
		// If we are already falling, don't transition at all
		if (UCommonMovementCheckUtils::IsFalling(Params.StartState.SyncState))
		{
			return NoTransition;
		}

		// If we are not falling, we can transition back to falling
		return BackToFalling;
	};

	if (UWallRunningMovementUtils::ShouldFallOffWall(OutWallHit, WallRunSettings->WallRun_PullAwayAngle, StartSyncState->GetIntent_WorldSpace()))
	{
		UE_LOG(LogBotaniMover, Warning, TEXT("I want to fall off the wall!"));
		return ConditionedReturn();
	}

	// Make sure we are high enough above the floor to start wall running
	if (!UWallRunningMovementUtils::IsHighEnoughForWallRun(Params.MovingComps, WallRunSettings->WallRun_MinRequiredHeight, UpDirection))
	{
		UE_LOG(LogBotaniMover, Warning, TEXT("I am not high enough above the floor to wall run!"));
		return ConditionedReturn();
	}
	

	UE_LOG(LogBotaniMover, Display, TEXT("I want to start wall running !!!!!"));
	
	return TransitionToWallRunning;
}

void UBotaniMMT_WallRunning::Trigger_Implementation(const FSimulationTickParams& Params)
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
	
	// Send the trigger event
	if (TriggerEvent.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = TriggerEvent;
		Payload.EventMagnitude = Params.MovingComps.MoverComponent->GetVelocity().Size();
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Params.MovingComps.MoverComponent->GetOwner(), Payload.EventTag, Payload);
	}
}

bool UBotaniMMT_WallRunning::CanStartWallRunning(
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
