// Copyright © 2025 Playton. All Rights Reserved.


#include "Modes/BotaniMM_Falling.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniMoverInputs.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "Components/BotaniMoverComponent.h"

#include "MoverComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MoveLibrary/AirMovementUtils.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/GroundMovementUtils.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_Falling)

UBotaniMM_Falling::UBotaniMM_Falling(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniMoverSettings::StaticClass());

	bCancelVerticalSpeedOnLanding = false;
	EffectiveVelocity = FVector::ZeroVector;
	ModeTag = BotaniGameplayTags::Mover::Modes::TAG_MM_Falling;
}

void UBotaniMM_Falling::Deactivate()
{
	Super::Deactivate();

	// Invalidate blackboard keys
	UMoverBlackboard* Blackboard = GetMoverComponent<UMoverComponent>()->GetSimBlackboard_Mutable();
	if (IsValid(Blackboard))
	{
		Blackboard->Invalidate(BotaniMover::Blackboard::NAME_LastFallTime);
		Blackboard->Invalidate(BotaniMover::Blackboard::NAME_LastGrappleTime);
	}
}

void UBotaniMM_Falling::GenerateMove_Implementation(
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

	// If movement is disabled, do nothing
	if (BotaniMover->IsMovementDisabled())
	{
		OutProposedMove.AngularVelocity = FRotator::ZeroRotator;
		OutProposedMove.LinearVelocity = FVector::ZeroVector;
		return;
	}

	// D: what is up ??
	FVector UpDirection = BotaniMover->GetUpDirection();

	// Get timings
	const float DeltaSeconds = TimeStep.StepMs * 0.001f;
	float LastFallTime = 0.f;
	float TimeFalling = 1000.f;

	if (Blackboard->TryGet(BotaniMover::Blackboard::NAME_LastFallTime, LastFallTime))
	{
		TimeFalling = (TimeStep.BaseSimTimeMs - LastFallTime) * 0.001f;
	}

	// We don't want velocity limits to take the falling velocity component into account, since it is handled 
	// separately by the terminal velocity of the environment.
	const FVector StartVelocity = StartSyncState->GetVelocity_WorldSpace();
	const FVector StartHorizontalVelocity = FVector::VectorPlaneProject(StartVelocity, UpDirection);


	// Special movement states such as gliding, skydiving, grappling or falling
	bool bGliding = false;
	bool bSkydiving = false;

	// Start filling up our move params
	FFreeMoveParams Params;
	if (MoveKinematicInputs)
	{
		Params.MoveInputType = MoveKinematicInputs->GetMoveInputType();

		//constexpr bool bMaintainInputMagnitude = true;
		//Params.MoveInput = UPlanarConstraintUtils::ConstrainDirectionToPlane(BotaniMover->GetPlanarConstraint(), MoveKinematicInputs->GetMoveInput_WorldSpace(), bMaintainInputMagnitude);
		Params.MoveInput = MoveKinematicInputs->GetMoveInput();
	}
	else
	{
		Params.MoveInputType = EMoveInputType::Invalid;
		Params.MoveInput = FVector::ZeroVector;
	}

	// Zero out the vertical input, it will be determined by gravity
	Params.MoveInput = FVector::VectorPlaneProject(Params.MoveInput, UpDirection);

	FRotator IntendedOrientation_WorldSpace;

	// Do we have orientation intent?
	if (!MoveKinematicInputs || MoveKinematicInputs->OrientationIntent.IsNearlyZero())
	{
		// Default to previous frame orient
		IntendedOrientation_WorldSpace = StartSyncState->GetOrientation_WorldSpace();
	}
	else
	{
		// Otherwise, use the input's orientation intent
		IntendedOrientation_WorldSpace = MoveKinematicInputs->GetOrientationIntentDir_WorldSpace().ToOrientationRotator();
	}

	// Set the move params that don't change depending on whether we're gliding, skydiving or just falling
	Params.OrientationIntent = IntendedOrientation_WorldSpace;
	Params.PriorVelocity = StartHorizontalVelocity;
	Params.PriorOrientation = StartSyncState->GetOrientation_WorldSpace();
	Params.DeltaSeconds = DeltaSeconds;

	// Default to regular falling params
	float AirControl = AirControlPercentage;

	//@TODO: bGliding, bSkydiving, bGrappling, bFalling
	if (bGliding)
	{
		
	}
	else if (bSkydiving)
	{
		
	}
	else
	{
		// Default to regular falling params
		Params.TurningRate = CommonLegacySettings->TurningRate;
		Params.TurningBoost = CommonLegacySettings->TurningBoost;
		Params.MaxSpeed = CommonLegacySettings->MaxSpeed;
		Params.Acceleration = CommonLegacySettings->Acceleration;
		Params.Deceleration = CommonLegacySettings->Deceleration;
		Params.WorldToGravityQuat = BotaniMover->GetWorldToGravityTransform();
		Params.bUseAccelerationForVelocityMove = CommonLegacySettings->bUseAccelerationForVelocityMove;
	}

	// Apply the air control
	Params.MoveInput *= AirControl;

	if (!bGliding)
	{
		// Do we want to move towards our velocity while over horizontal terminal velocity?
		if (Params.MoveInput.Dot(StartVelocity) > 0.f && StartHorizontalVelocity.Size() >= TerminalMovementPlaneSpeed)
		{
			// Project the input into the movement plane defined by the velocity
			const FPlane MovementNormalPlane(StartVelocity, StartVelocity.GetSafeNormal());
			Params.MoveInput = Params.MoveInput.ProjectOnTo(MovementNormalPlane);

			// Use the horizontal terminal velocity deceleration so we break faster
			Params.Deceleration = OverTerminalSpeedFallingDeceleration;
		}
	}

	FFloorCheckResult LastFloorResult;

	// Do we have a valid floor hit result?
	if (Blackboard->TryGet(CommonBlackboard::LastFloorResult, LastFloorResult))
	{
		// Are we sliding along a vertical slope?
		if (LastFloorResult.HitResult.IsValidBlockingHit() &&
			LastFloorResult.HitResult.Normal.Dot(UpDirection) > UE::MoverUtils::VERTICAL_SLOPE_NORMAL_MAX_DOT &&
			!LastFloorResult.IsWalkableFloor())
		{
			// Are we trying to speed up into the wall?
			if (FVector::DotProduct(Params.MoveInput, LastFloorResult.HitResult.Normal) < 0.f)
			{
				// Allow movement parallel to the wall, but not into it because that may push us up
				const FVector FallingHitNormal = FVector::VectorPlaneProject(LastFloorResult.HitResult.Normal, -UpDirection);
				Params.MoveInput = FVector::VectorPlaneProject(Params.MoveInput, FallingHitNormal);
			}
		}
	}

	// Compute the free move
	OutProposedMove = UAirMovementUtils::ComputeControlledFreeMove(Params);
	const FVector VelocityWithGravity = StartVelocity + UMovementUtils::ComputeVelocityFromGravity(BotaniMover->GetGravityAcceleration(), DeltaSeconds);

	// If we are going faster than the TerminalVerticalVelocity apply a VerticalFallingDeceleration,
	// otherwise reset Z velocity to before we applied deceleration
	if (VelocityWithGravity.GetAbs().Dot(UpDirection) > TerminalVerticalSpeed)
	{
		if (bShouldClampTerminalVerticalSpeed)
		{
			// Clamp the vertical speed to the terminal speed
			const float ClampedVerticalSpeed = FMath::Sign(VelocityWithGravity.Dot(UpDirection)) * TerminalVerticalSpeed;
			UMovementUtils::SetGravityVerticalComponent(OutProposedMove.LinearVelocity, ClampedVerticalSpeed, UpDirection);
		}
		else
		{
			// Apply deceleration to the vertical component of the velocity
			float DesiredDeceleration = FMath::Abs(TerminalVerticalSpeed - VelocityWithGravity.GetAbs().Dot(UpDirection)) / DeltaSeconds;
			float DecelerationToApply = FMath::Min(DesiredDeceleration, VerticalFallingDeceleration);
			DecelerationToApply = FMath::Sign(VelocityWithGravity.Dot(UpDirection)) * DecelerationToApply * DeltaSeconds;
			FVector MaxUpDirVelocity = VelocityWithGravity * UpDirection - (UpDirection * DecelerationToApply);
			
			UMovementUtils::SetGravityVerticalComponent(
				OutProposedMove.LinearVelocity,
				MaxUpDirVelocity.Dot(UpDirection),
				UpDirection);
		}
	}
	else
	{
		UMovementUtils::SetGravityVerticalComponent(
			OutProposedMove.LinearVelocity,
			VelocityWithGravity.Dot(UpDirection),
			UpDirection);
	}
}

bool UBotaniMM_Falling::PrepareSimulationData(const FSimulationTickParams& Params)
{
	if (!Super::PrepareSimulationData(Params))
	{
		return false;
	}

	// Get the timings of grappling
	float LastGrappleTime = 0.f;
	//@TODO;

	return true;
}

void UBotaniMM_Falling::ApplyMovement(FMoverTickEndData& OutputState)
{
	UMoverComponent* MoverComponent = GetMoverComponent();
	
	// Initialize our fall data
	FCommonMoveData FallData;
	FallData.MoveRecord.SetDeltaSeconds(DeltaTime);
	FallData.OriginalMoveDelta = ProposedMove->LinearVelocity * DeltaTime;
	FallData.CurrentMoveDelta = FallData.OriginalMoveDelta;

	// Cache the prior velocity
	const FVector PriorFallingVelocity = StartingSyncState->GetVelocity_WorldSpace();

	// Invalidate the previous floor
	SimBlackboard->Invalidate(CommonBlackboard::LastFloorResult);
	SimBlackboard->Invalidate(CommonBlackboard::LastFoundDynamicMovementBase);

	// Calculate our new rotation
	bool bOrientationChanged = CalculateOrientationChange(FallData.TargetOrientQuat);

	const FVector StartingFallingVelocity = StartingSyncState->GetVelocity_WorldSpace();
	const FVector UpDirection = MoverComponent->GetUpDirection();

	// Move
	UMovementUtils::TrySafeMoveUpdatedComponent(
		MovingComponentSet,
		FallData.CurrentMoveDelta,
		FallData.TargetOrientQuat,
		true,
		FallData.MoveHitResult,
		ETeleportType::None,
		FallData.MoveRecord);

	// Compute final velocity based on how long we actually moved until we get a hit
	FVector NewFallingVelocity = StartingSyncState->GetVelocity_WorldSpace();
	NewFallingVelocity += (ProposedMove->LinearVelocity * -UpDirection * FallData.MoveHitResult.Time);

	// Handle collisions against floors or wall
	FFloorCheckResult LandingFloor;

	// Have we hit something?
	if (FallData.MoveHitResult.IsValidBlockingHit() &&
		MovingComponentSet.UpdatedPrimitive.IsValid())
	{
		// Update the time applied so far
		FallData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(
			FallData.PercentTimeAppliedSoFar,
			FallData.MoveHitResult.Time);

#if ENABLE_VISUAL_LOG
		{
			const FVector ArrowEnd = FallData.MoveHitResult.bBlockingHit
				? FallData.MoveHitResult.Location
				: FallData.MoveHitResult.TraceEnd;

			const FColor ArrowColor = FallData.MoveHitResult.bBlockingHit
				? FColor::Red
				: FColor::Green;

			UE_VLOG_ARROW(this, VLogBotaniMover, Log, FallData.MoveHitResult.TraceStart, ArrowEnd, ArrowColor,
				TEXT("Fall\nStart[%s]\nEnd[%s]\nPct[%f]"),
				*FallData.MoveHitResult.TraceStart.ToCompactString(),
				*ArrowEnd.ToCompactString(),
				FallData.PercentTimeAppliedSoFar);
		}
#endif


		// Have we hit a landing surface?
		if (UAirMovementUtils::IsValidLandingSpot(
			MovingComponentSet,
			MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
			FallData.MoveHitResult,
			CommonLegacySettings->FloorSweepDistance,
			CommonLegacySettings->MaxWalkSlopeCosine,
			LandingFloor))
		{
			// Try to adjust our location so we don't get stuck in the floor
			UGroundMovementUtils::TryMoveToAdjustHeightAboveFloor(
				MoverComponent,
				LandingFloor,
				CommonLegacySettings->MaxWalkSlopeCosine,
				FallData.MoveRecord);

			CaptureFinalState(LandingFloor, DeltaTime * FallData.PercentTimeAppliedSoFar, OutputState, FallData.MoveRecord);
			return;
		}

		// Update the last floor result on the blackboard
		LandingFloor.HitResult = FallData.MoveHitResult;
		SimBlackboard->Set(CommonBlackboard::LastFloorResult, LandingFloor);

		// Tell the mover component to handle a wall impact
		FMoverOnImpactParams ImpactParams(DefaultModeNames::Falling, FallData.MoveHitResult, FallData.CurrentMoveDelta);
		MoverComponent->HandleImpact(ImpactParams);

		// We didn't land on a walkable surface, so let's try to slide along it
		UAirMovementUtils::TryMoveToFallAlongSurface(
			MovingComponentSet,
			FallData.CurrentMoveDelta,
			(1.f - FallData.MoveHitResult.Time),
			FallData.TargetOrientQuat,
			FallData.MoveHitResult.Normal,
			FallData.MoveHitResult,
			true,
			CommonLegacySettings->FloorSweepDistance,
			CommonLegacySettings->MaxWalkSlopeCosine,
			LandingFloor,
			FallData.MoveRecord);

		// Update the time applied so far
		FallData.PercentTimeAppliedSoFar = UpdateTimePercentAppliedSoFar(FallData.PercentTimeAppliedSoFar, FallData.MoveHitResult.Time);

		// Have we now landed on a walkable floor?
		if (LandingFloor.IsWalkableFloor())
		{
			// Try to adjust our location so we don't get stuck in the floor
			UGroundMovementUtils::TryMoveToAdjustHeightAboveFloor(
				MoverComponent,
				LandingFloor,
				CommonLegacySettings->MaxWalkSlopeCosine,
				FallData.MoveRecord);

			// Capture the final state and handle landing
			CaptureFinalState(LandingFloor, DeltaTime * FallData.PercentTimeAppliedSoFar, OutputState, FallData.MoveRecord);
			return;
		}
	}
	else
	{
		// This indicates an unimpeded full move
		FallData.PercentTimeAppliedSoFar = 1.f;
	}

	// Capture the final state of the move
	CaptureFinalState(LandingFloor, DeltaTime * FallData.PercentTimeAppliedSoFar, OutputState, FallData.MoveRecord);
}

void UBotaniMM_Falling::PostMove(FMoverTickEndData& OutputState)
{
	Super::PostMove(OutputState);

	// Get the timings
	float LastFallTime = 0.f;
	float LastJumpTime = 0.f;
	float TimeFalling = 1000.f;

	if (IsValid(SimBlackboard))
	{
		if (SimBlackboard->TryGet<float>(BotaniMover::Blackboard::NAME_LastFallTime, LastFallTime))
		{
			TimeFalling = CurrentSimulationTime - LastFallTime;
		}

		if (SimBlackboard->TryGet<float>(BotaniMover::Blackboard::NAME_LastJumpTime, LastJumpTime))
		{
			
		}
	}
}

void UBotaniMM_Falling::CaptureFinalState(
	const FFloorCheckResult& FloorResult,
	float DeltaSecondsUsed,
	FMoverTickEndData& TickEndData,
	FMovementRecord& Record)
{
	const FVector FinalLocation = MovingComponentSet.UpdatedPrimitive->GetComponentLocation();

	// Check for refunds
	// If we have this amount of time (or more) remaining, give it to the next simulation step.
	constexpr float MinRemainingSecondsToRefund = 0.0001f;

	if ((DeltaTime - DeltaSecondsUsed) >= MinRemainingSecondsToRefund)
	{
		const float PctOfTimeRemaining = (1.f - (DeltaSecondsUsed / DeltaTime));
		TickEndData.MovementEndState.RemainingMs = PctOfTimeRemaining * DeltaTime * 1000.f;
	}
	else
	{
		TickEndData.MovementEndState.RemainingMs = 0.f;
	}

	Record.SetDeltaSeconds(DeltaSecondsUsed);

	EffectiveVelocity = Record.GetRelevantVelocity();
	// TODO: Update Main/large movement record with substeps from our local record

	FRelativeBaseInfo MovementBaseInfo;
	ProcessLanded(FloorResult, EffectiveVelocity, MovementBaseInfo, TickEndData);

	if (MovementBaseInfo.HasRelativeInfo())
	{
		SimBlackboard->Set(CommonBlackboard::LastFoundDynamicMovementBase, MovementBaseInfo);

		OutDefaultSyncState->SetTransforms_WorldSpace(
			FinalLocation,
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			EffectiveVelocity,
			MovementBaseInfo.MovementBase.Get(),
			MovementBaseInfo.BoneName);
	}
	else
	{
		OutDefaultSyncState->SetTransforms_WorldSpace(
			FinalLocation,
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			EffectiveVelocity,
			nullptr); // no movement base
	}

	// Set the component's velocity
	MovingComponentSet.UpdatedComponent->ComponentVelocity = EffectiveVelocity;
}

void UBotaniMM_Falling::ProcessLanded(
	const FFloorCheckResult& FloorResult,
	FVector& Velocity,
	FRelativeBaseInfo& BaseInfo,
	FMoverTickEndData& TickEndData) const
{
	FName NextMovementMode = NAME_None;

	// Can we walk on the floor we landed on?
	if (FloorResult.IsWalkableFloor())
	{
		// Send the landing event to the owning actor
		if (LandingEventTag.IsValid())
		{
			// Encode the velocity as the event magnitude
			FGameplayEventData Payload;
			Payload.EventMagnitude = Velocity.Size();
			Payload.EventTag = LandingEventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MutableMoverComponent->GetOwner(), LandingEventTag, Payload);
		}

		// Cancel vertical speed if we should
		if (bCancelVerticalSpeedOnLanding)
		{
			const FPlane MovementPlane(FVector::ZeroVector, MutableMoverComponent->GetUpDirection());
			Velocity = UMovementUtils::ConstrainToPlane(Velocity, MovementPlane, false);
		}
		else
		{
			Velocity = FVector::VectorPlaneProject(Velocity, FloorResult.HitResult.Normal);
		}

		// Switch to ground movement mode (usually walking) and cache any floor / movement base info
		NextMovementMode = CommonLegacySettings->GroundMovementModeName;
		SimBlackboard->Set(CommonBlackboard::LastFloorResult, FloorResult);

		if (UBasedMovementUtils::IsADynamicBase(FloorResult.HitResult.GetComponent()))
		{
			BaseInfo.SetFromFloorResult(FloorResult);
		}
	}

	// We could check for other surfaces here (i.e., when swimming is implemented,
	// we can check the floor hit here and see if we need to go into swimming)

	// This would also be a good spot for implementing some falling physics interactions
	// (i.e., falling into a movable object and pushing it based off of this actors' velocity)
	
	// If a new mode was set, go ahead and switch to it after this tick and broadcast we landed
	if (!NextMovementMode.IsNone())
	{
		TickEndData.MovementEndState.NextModeName = NextMovementMode;
		MutableMoverComponent->OnLanded(NextMovementMode, FloorResult.HitResult);
	}
}
