// Author: Tom Werner (MajorT), 2025


#include "Modes/BotaniMM_GroundBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BotaniCommonMovementSettings.h"

#include "BotaniMoverSettings.h"
#include "CommonMoverComponent.h"
#include "IBotaniMoverPhysicalMaterial.h"
#include "MoveLibrary/GroundMovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_GroundBase)

UBotaniMM_GroundBase::UBotaniMM_GroundBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniMoverSettings::StaticClass());
	SharedSettingsClasses.Add(UBotaniCommonMovementSettings::StaticClass());
	GameplayTags.AddTag(Mover_IsOnGround);
}

void UBotaniMM_GroundBase::OnRegistered(const FName ModeName)
{
	Super::OnRegistered(ModeName);

	// Get the botani mover settings
	BotaniMoverSettings = GetMoverComponent()->FindSharedSettings<UBotaniMoverSettings>();
	ensureMsgf(BotaniMoverSettings, TEXT("Failed to find instance of BotaniMoverSettings on %s. Movement may not function properly."),
		*GetPathNameSafe(this));

	// Get the common movement settings
	BotaniMovementSettings = GetMoverComponent()->FindSharedSettings<UBotaniCommonMovementSettings>();
	ensureMsgf(BotaniMovementSettings, TEXT("Failed to find instance of BotaniCommonMovementSettings on %s. Movement may not function properly."),
		*GetPathNameSafe(this));
}

void UBotaniMM_GroundBase::OnUnregistered()
{
	// Release the botani mover settings pointer
	BotaniMoverSettings = nullptr;

	// Release the common movement settings pointer
	BotaniMovementSettings = nullptr;

	Super::OnUnregistered();
}

void UBotaniMM_GroundBase::ApplyMovement(FMoverTickEndData& OutputState)
{
	// Ensure we have cached floor information before moving
	ValidateFloor(
		BotaniMovementSettings->FloorSweepDistance,
		GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine));

	// Initialize the move data
	FCommonMoveData WalkData;
	const FVector UpDirection = MutableMoverComponent->GetUpDirection();

	bool bDidAttemptMovement = false;

	// Initialize the move record
	WalkData.MoveRecord.SetDeltaSeconds(DeltaTime);

	// Apply any movement from a dynamic base
	bool bDidMoveAlongWithBase = ApplyDynamicFloorMovement(OutputState, WalkData.MoveRecord);

	// After handling the dynamic base, check for disabled movement
	if (MutableMoverComponent->IsMovementDisabled())
	{
		CaptureFinalState(CurrentFloor, bDidAttemptMovement, WalkData.MoveRecord);
		return;
	}

	// Calculate the target orientation for the following moves
	bool bIsOrientationChanging = CalculateOrientationChange(WalkData.TargetOrientQuat);

	// Calculate the move delta
	WalkData.OriginalMoveDelta = ProposedMove->LinearVelocity * DeltaTime;
	WalkData.CurrentMoveDelta = WalkData.OriginalMoveDelta;

	const FRotator StartingOrient = StartingSyncState->GetOrientation_WorldSpace();
	FRotator TargetOrient = StartingOrient;
	WalkData.TargetOrientQuat = TargetOrient.Quaternion();
	if (BotaniMovementSettings->bShouldRemainUpright)
	{
		WalkData.TargetOrientQuat = FRotationMatrix::MakeFromZX(UpDirection, WalkData.TargetOrientQuat.GetForwardVector()).ToQuat();
	}

	// Floor check result passed to step-up suboperations, so we can use their final floor results if they did a test
	FOptionalFloorCheckResult StepUpFloorResult;

	// Are we moving or re-orienting?
	if (!WalkData.CurrentMoveDelta.IsNearlyZero() ||bIsOrientationChanging)
	{
		// We are about to move !
		bDidAttemptMovement = true;

		// Apply the first move.
		// This will catch any potential collisions or initial penetration
		bool bMovedFreely = ApplyFirstMove(WalkData);

		// Apply any depenetration in case we started in the frame stuck.
		// This will include any catch-up from the first move
		bool bDepenetration = ApplyDepenetrationOnFirstMove(WalkData);

		if (!bDepenetration)
		{
			// If no depenetration was done, we can check for a ramp
			bool bMovedUpRamp = ApplyRampMove(
				WalkData,
				GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine));

			// Attempt to move up any climbable obstacles
			bool bSteppedUp = ApplyStepUpMove(
				WalkData,
				StepUpFloorResult,
				GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine),
				GetBotaniMoverFloatProp(MaxStepHeight), BotaniMovementSettings->FloorSweepDistance);

			// Did we fail to step up?
			bool bSlidAlongWall = false;
			if (bSteppedUp)
			{
				// Attempt to slide along an unclimbable obstacle
				bSlidAlongWall = ApplySlideAlongWall(
					WalkData,
					GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine),
					GetBotaniMoverFloatProp(MaxStepHeight));
			}

			// Search for the floor we've ended up on
			UFloorQueryUtils::FindFloor(
				MovingComponentSet,
				BotaniMovementSettings->FloorSweepDistance,
				GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine),
				MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
				CurrentFloor);

			// Adjust vertically so we remain in contact with the floor
			bool bAdjustedToFloor = ApplyFloorHeightAdjustment(
				WalkData,
				GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine));

			// Check if we're falling
			if (HandleFalling(
				OutputState,
				WalkData.MoveRecord,
				CurrentFloor.HitResult,
				DeltaMs * WalkData.PercentTimeAppliedSoFar))
			{
				// Handle falling captured our output state, so we can return
				return;
			}
		}
	}
	else
	{
		// We don't need to move this frame, but we may still need to adjust to the floor
		// Search for the floor we're standing on
		UFloorQueryUtils::FindFloor(
			MovingComponentSet,
			BotaniMovementSettings->FloorSweepDistance,
			GetBotaniMoverFloatProp(MaxWalkSlopeAngleCosine),
			MovingComponentSet.UpdatedPrimitive->GetComponentLocation(),
			CurrentFloor);

		// Copy the current floor hit result
		WalkData.MoveHitResult = CurrentFloor.HitResult;

		// Check if we need to adjust to depenetrate from the floor
		bool bAdjustedToFloor = ApplyIdleCorrections(WalkData);

		// Check if we're falling
		if (HandleFalling(OutputState, WalkData.MoveRecord, CurrentFloor.HitResult, DeltaMs * WalkData.PercentTimeAppliedSoFar))
		{
			// Handle falling captured our output state, so we can return
			return;
		}
	}

	// Capture the final movement state
	CaptureFinalState(CurrentFloor, bDidAttemptMovement, WalkData.MoveRecord);
}

void UBotaniMM_GroundBase::ValidateFloor(float FloorSweepDistance, float MaxWalkableSlopeCosine)
{
	Super::ValidateFloor(FloorSweepDistance, MaxWalkableSlopeCosine);
}

void UBotaniMM_GroundBase::ApplyPhysicalGroundFriction(
	FGroundMoveParams& MoveParams,
	const FFloorCheckResult& FloorToUse,
	const bool bOverrideFriction) const
{
	if (const IBotaniMoverPhysicalMaterial* MoverPhysMat
		= Cast<IBotaniMoverPhysicalMaterial>(FloorToUse.HitResult.PhysMaterial.Get()))
	{
		MoveParams.Friction = MoverPhysMat->CalculateFrictionCoefficient(MoveParams.Friction);
		MoveParams.Acceleration = MoverPhysMat->GetAccelerationOverride().Get(MoveParams.Acceleration);
		MoveParams.Deceleration = MoverPhysMat->GetDecelerationOverride().Get(MoveParams.Deceleration);
	}
}

float UBotaniMM_GroundBase::GetEffectiveMaxSpeed(
	const float& InMaxSpeed,
	const FMoverTickStartData& StartState) const
{
	if (const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		// If the stop movement tag is active, we can't move
		if (AbilitySystem->HasMatchingGameplayTag(BotaniMoverSettings->StopMovementTag))
		{
			return 0.f;
		}
	}

	// Otherwise, return the input max speed
	return InMaxSpeed;
}

UAbilitySystemComponent* UBotaniMM_GroundBase::GetAbilitySystemComponent() const
{
	UBotaniMM_GroundBase* MutableThis = const_cast<UBotaniMM_GroundBase*>(this);
	if (!MutableThis->AbilitySystemComponent.IsValid() && GetMoverComponent())
	{
		if (const AActor* Owner = GetMoverComponent()->GetOwner())
		{
			if (UAbilitySystemComponent* AbilitySystem =
				UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
			{

				MutableThis->AbilitySystemComponent = AbilitySystem;
			}
		}
	}

	return MutableThis->AbilitySystemComponent.Get();
}
