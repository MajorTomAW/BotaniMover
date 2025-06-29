// Copyright © 2025 Playton. All Rights Reserved.


#include "LayeredMoves/BotaniLM_Jump.h"

#include "CommonBlackboard.h"
#include "CommonMoverComponent.h"
#include "MoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "MoveLibrary/AirMovementUtils.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniLM_Jump)

FBotaniLM_Jump::FBotaniLM_Jump()
{
	DurationMs = 0.f;
	MixMode = EMoveMixMode::OverrideVelocity;

	UpwardsSpeed = 0.f;
	Momentum = FVector::ZeroVector;

	AirControl = 1.f;

	bTruncateOnJumpRelease = true;
	bOverrideHorizontalMomentum = false;
	bOverrideVerticalMomentum = false;
}

FBotaniLM_Jump::~FBotaniLM_Jump()
{
}

bool FBotaniLM_Jump::GenerateMove(
	const FMoverTickStartData& StartState,
	const FMoverTimeStep& TimeStep,
	const UMoverComponent* MoverComp,
	UMoverBlackboard* SimBlackboard,
	FProposedMove& OutProposedMove)
{
	// Grab required data
	const UCommonLegacyMovementSettings* CommonLegacySettings = MoverComp->FindSharedSettings<UCommonLegacyMovementSettings>();
	check(CommonLegacySettings);

	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(SyncState);

	const UCommonMoverComponent* CommonMover = Cast<UCommonMoverComponent>(MoverComp);
	check(CommonMover);

	// Get the inputs
	const FCharacterDefaultInputs* KinematicInputs = StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// If we are no longer falling, set the duration to zero
	if (TimeStep.BaseSimTimeMs != StartSimTimeMs && StartState.SyncState.MovementMode != CommonLegacySettings->AirMovementModeName)
	{
		DurationMs = 0.f;
	}

	bool bJumping = false;

	if (KinematicInputs)
	{
		bJumping = KinematicInputs->bIsJumpPressed;
	}

	// If we are no longer pressing jump, set the duration to zero and end the upwards impulse
	if (bTruncateOnJumpRelease && !bJumping)
	{
		DurationMs = 0.f;
	}

	// Return a zero move if movement is disabled
	if (CommonMover->IsMovementDisabled())
	{
		OutProposedMove.AngularVelocity = FRotator::ZeroRotator;
		OutProposedMove.LinearVelocity = FVector::ZeroVector;

		return true;
	}

	const FVector UpDirection = MoverComp->GetUpDirection();

	// We can either override vertical velocity with the provided momentum
	// or grab it from the sync state.
	FVector UpVelocity;

	if (bOverrideVerticalMomentum)
	{
		UpVelocity = Momentum.ProjectOnToNormal(UpDirection);
	}
	else
	{
		UpVelocity = SyncState->GetVelocity_WorldSpace().ProjectOnToNormal(UpDirection);
	}

	// We can either override move plane velocity with the provided momentum
	// or grab it from the sync state.
	FVector NonUpVelocity;
	
	if (bOverrideHorizontalMomentum)
	{
		NonUpVelocity = Momentum - UpVelocity;
	}
	else
	{
		NonUpVelocity = SyncState->GetVelocity_WorldSpace() - UpVelocity;
	}

	// Apply the jump upwards speed
	UpVelocity += UpDirection * UpwardsSpeed;

	// Start building our move
	FFreeMoveParams Params;

	// Set the inputs
	if (KinematicInputs)
	{
		Params.MoveInputType = KinematicInputs->GetMoveInputType();
		Params.MoveInput = KinematicInputs->GetMoveInput();
	}
	else
	{
		Params.MoveInputType = EMoveInputType::Invalid;
		Params.MoveInput = FVector::ZeroVector;
	}

	// Apply the air control of the jump
	Params.MoveInput *= AirControl;

	FRotator IntendedOrientation_WorldSpace;

	// Do we have orientation intent?
	if (!KinematicInputs || KinematicInputs->OrientationIntent.IsNearlyZero())
	{
		// Default to the previous frame orientation
		IntendedOrientation_WorldSpace = SyncState->GetOrientation_WorldSpace();
	}
	else
	{
		// Use the orientation intent from the inputs
		IntendedOrientation_WorldSpace = KinematicInputs->GetOrientationIntentDir_WorldSpace().ToOrientationRotator();
	}

	// Zero out the vertical component of the move input
	Params.MoveInput = Params.MoveInput.ProjectOnToNormal(MoverComp->GetUpDirection());
	
	// Fill in the rest of our move params
	Params.OrientationIntent = IntendedOrientation_WorldSpace;
	Params.PriorVelocity = NonUpVelocity + UpVelocity;
	Params.PriorOrientation = SyncState->GetOrientation_WorldSpace();
	Params.DeltaSeconds = TimeStep.StepMs * 0.001f;
	Params.TurningRate = CommonLegacySettings->TurningRate;
	Params.TurningBoost = CommonLegacySettings->TurningBoost;
	Params.MaxSpeed = CommonLegacySettings->MaxSpeed;
	Params.Acceleration = CommonLegacySettings->Acceleration;
	Params.Deceleration = 0.0f;

	if (MixMode == EMoveMixMode::OverrideVelocity)
	{
		// Calculate the out proposed move
		OutProposedMove = UAirMovementUtils::ComputeControlledFreeMove(Params);

		// Add velocity change due to gravity
		OutProposedMove.LinearVelocity += UMovementUtils::ComputeVelocityFromGravity(MoverComp->GetGravityAcceleration(), Params.DeltaSeconds);

		// Save the fall time to the blackboard
		if (TimeStep.BaseSimTimeMs == StartSimTimeMs)
		{
			SimBlackboard->Set(CommonBlackboard::LastFallTime, StartSimTimeMs);
		}
	}
	else
	{
		// We do not support other mix modes for this move
		ensureMsgf(false, TEXT("FBotaniLM_Jump does not support mix mode %s!"), *UEnum::GetValueAsString(MixMode));
		return false;
	}

#if ENABLE_VISUAL_LOG
	//@TODO: Add visual logging support
#endif


	return true;
}

FLayeredMoveBase* FBotaniLM_Jump::Clone() const
{
	FBotaniLM_Jump* CopyPtr = new FBotaniLM_Jump(*this);
	return CopyPtr;
}

void FBotaniLM_Jump::NetSerialize(FArchive& Ar)
{
	Super::NetSerialize(Ar);

	Ar << UpwardsSpeed;
	Ar << Momentum;
	Ar << AirControl;
	Ar << bTruncateOnJumpRelease;
}

FString FBotaniLM_Jump::ToSimpleString() const
{
	return FString::Printf(TEXT("Botani-LM Jump"));
}

void FBotaniLM_Jump::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
}
