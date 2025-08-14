// Author: Tom Werner (MajorT), 2025


#include "Modes/BotaniMM_WallRunning.h"

#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverInputs.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "BotaniMoverVLogHelpers.h"
#include "BotaniWallRunMovementSettings.h"
#include "IBotaniMoverPhysicalMaterial.h"
#include "MoverComponent.h"
#include "Components/BotaniMoverComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_WallRunning)

UBotaniMM_WallRunning::UBotaniMM_WallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniWallRunMovementSettings::StaticClass());

	ModeTag = BotaniGameplayTags::Mover::Modes::TAG_MM_WallRunning;
	GameplayTags.AddTag(Mover_IsOnGround);

	EffectiveVelocity = FVector::ZeroVector;
}

void UBotaniMM_WallRunning::Deactivate()
{
	Super::Deactivate();
}

bool UBotaniMM_WallRunning::PrepareSimulationData(const FSimulationTickParams& Params)
{
	if (!Super::PrepareSimulationData(Params))
	{
		return false;
	}

	// Get the time
	float LastWallTime = 0.f;
	float TimeSinceLastWallJump = 1000.f;

	if (SimBlackboard->TryGet<float>(BotaniMover::Blackboard::LastWallRunTime, LastWallTime))
	{

	}

	return true;
}

void UBotaniMM_WallRunning::GenerateMove_Implementation(
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

	// Get the wall running settings
	const UBotaniWallRunMovementSettings* BotaniWallRunSettings = BotaniMover->FindSharedSettings<UBotaniWallRunMovementSettings>();
	check(BotaniWallRunSettings);

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
	const float DeltaSeconds = TimeStep.StepMs * BotaniMover::Lazy::MsToS;
	float LastWallTime = 0.f;
	float TimeWallRunning = 1000.f;

	if (Blackboard->TryGet<float>(BotaniMover::Blackboard::LastWallRunTime, LastWallTime))
	{
		TimeWallRunning = (TimeStep.BaseSimTimeMs - LastWallTime) * BotaniMover::Lazy::MsToS;
	}

	// We don't want velocity limits to take the falling velocity component into account, since it is handled
	// separately by the terminal velocity of the environment.
	const FVector StartVelocity = StartSyncState->GetVelocity_WorldSpace();
	const FVector StartHorizontalVelocity = FVector::VectorPlaneProject(StartVelocity, UpDirection);

	// Start gathering all the important move info

	// Check for valid walls to run on
	FVector WallNormal;
	FWallCheckResult LastWallResult;
	if (Blackboard->TryGet<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, LastWallResult)
		&& LastWallResult.IsRunAbleWall())
	{
		WallNormal = LastWallResult.GetHitResult().ImpactNormal;
	}
	else
	{
		// No wall, so transition to falling mode, please!!
		//@TODO: Can we queue a next mode here? Or does it need to be done in SimulationTick ?
		OutProposedMove.PreferredMode = DefaultModeNames::Falling;
		return;
	}

	// Start filling up our move params
	FWallRunMoveParams Params;
	if (MoveKinematicInputs)
	{
		Params.MoveInputType = MoveKinematicInputs->GetMoveInputType();

		constexpr bool bMaintainInputMagnitude = true;
		Params.MoveInput = UPlanarConstraintUtils::ConstrainDirectionToPlane(BotaniMover->GetPlanarConstraint(), MoveKinematicInputs->GetMoveInput_WorldSpace(), bMaintainInputMagnitude);
		//Params.MoveInput = MoveKinematicInputs->GetMoveInput();
	}
	else
	{
		Params.MoveInputType = EMoveInputType::Invalid;
		Params.MoveInput = FVector::ZeroVector;
	}

	// Project movement input onto an orthogonal line to the wall normal and up direction
	Params.MoveInput = FVector::VectorPlaneProject(
		FVector::VectorPlaneProject(Params.MoveInput, WallNormal),
		UpDirection);

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

	IntendedOrientation_WorldSpace = UMovementUtils::ApplyGravityToOrientationIntent(
		IntendedOrientation_WorldSpace,
		BotaniMover->GetWorldToGravityTransform(),
		BotaniMovementSettings->bShouldRemainUpright);

	// Clamp Orientation to wall normal
	if (BotaniMovementSettings->bShouldRemainUpright)
	{
		//@TODO: The thing above, not sure about it tho, as i still need orientation intent for dot product checks
	}

	// Set the move params
	Params.OrientationIntent = IntendedOrientation_WorldSpace;
	Params.PriorVelocity = StartVelocity;
	Params.PriorOrientation = StartSyncState->GetOrientation_WorldSpace();
	Params.DeltaSeconds = DeltaSeconds;
	Params.WorldToGravityQuat = BotaniMover->GetWorldToGravityTransform();
	Params.Acceleration = GetBotaniWallRunFloatProp(WallRun_Acceleration);
	Params.MaxSpeed = GetBotaniWallRunFloatProp(WallRun_MaxSpeed);
	Params.bUseAccelerationForVelocityMove = BotaniMovementSettings->bUseAccelerationForVelocityIntent;

	// Apply the acceleration based on the friction
	const bool bIsMovingTooFast = Params.MoveInput.SizeSquared() <= 0.f
		&& UMovementUtils::IsExceedingMaxSpeed(Params.PriorVelocity, Params.MaxSpeed);

	if (bIsMovingTooFast)
	{
		// Use the braking friction to slow down back to the max speed
		Params.Friction = GetBotaniMoverFloatProp(BrakingFriction);
		ApplyPhysicalWallFriction(Params, LastWallResult, false);
		Params.Friction += GetBotaniMoverFloatProp(BrakingFrictionFactor);

		Params.Deceleration = GetBotaniWallRunFloatProp(WallRun_BrakingDeceleration);
	}
	else
	{
		// Default to regular friction
		Params.Friction = GetBotaniMoverFloatProp(GroundFriction);
		ApplyPhysicalWallFriction(Params, LastWallResult, false);
		Params.Friction += GetBotaniWallRunFloatProp(WallRun_SurfaceFrictionFactor);

		Params.Deceleration = GetBotaniWallRunFloatProp(WallRun_Deceleration);
	}

	// Compute the wall run move
	OutProposedMove = UWallRunningMovementUtils::ComputeControlledWallRunMove(Params);

	// Acceleration in the direction of the velocity
	float TangentAccel = Params.OrientationIntent.Vector() | StartVelocity.GetSafeNormal2D();
	bool bIsVelocityUpwards = StartVelocity.Z > 0.f; //@TODO: This is bad, we should use the up direction instead

	FVector DeltaVelocity = StartVelocity;

	// Apply different gravity scale when moving upwards/downwards
	const float VelocityVerticalComponent = StartVelocity.ProjectOnToNormal(UpDirection).Size();
	const bool bIsMovingUpwards = VelocityVerticalComponent > 0.f;

	const float OverallGravityScale = bIsVelocityUpwards
		?	GetBotaniWallRunFloatProp(WallRun_UpwardsGravityScale)
		:	GetBotaniWallRunFloatProp(WallRun_GravityScale);

	{ // Apply the gravity scale based on the players' velocity
		float GravityScale = 0.f;
		const FRichCurve* GravityVelScaleCurve = BotaniWallRunSettings->WallRun_GravityVelScaleCurve.GetRichCurveConst();
		if (GravityVelScaleCurve && GravityVelScaleCurve->HasAnyData())
		{
			GravityScale = GravityVelScaleCurve->Eval(bIsVelocityUpwards ? 0.f : TangentAccel);
		}

		DeltaVelocity += UMovementUtils::ComputeVelocityFromGravity(
				BotaniMover->GetGravityAcceleration() * GravityScale * OverallGravityScale,
				DeltaSeconds);
	}

	{ // Apply the gravity scale based on how long we have been wall running
		float GravityScale = 0.f;
		const FRichCurve* GravityScaleCurve = BotaniWallRunSettings->WallRun_GravityTimeScaleCurve.GetRichCurveConst();
		if (GravityScaleCurve && GravityScaleCurve->HasAnyData())
		{
			GravityScale *= GravityScaleCurve->Eval(TimeWallRunning);
		}

		DeltaVelocity += UMovementUtils::ComputeVelocityFromGravity(
				BotaniMover->GetGravityAcceleration() * GravityScale * OverallGravityScale,
				DeltaSeconds);
	}

	UE_LOG(LogBotaniMover, Display, TEXT("Delta Vel: %s"), *DeltaVelocity.ToString());

	//@TODO: Terminal speed ?
	if (false)
	{

	}
	else
	{
		UMovementUtils::SetGravityVerticalComponent(
			OutProposedMove.LinearVelocity,
			DeltaVelocity.Dot(UpDirection),
			UpDirection);
	}
}

void UBotaniMM_WallRunning::ApplyPhysicalWallFriction(
	FWallRunMoveParams& MoveParams,
	const FWallCheckResult& FloorToUse,
	const bool bOverrideFriction) const
{
	if (const IBotaniMoverPhysicalMaterial* MoverPhysMat
		= Cast<IBotaniMoverPhysicalMaterial>(FloorToUse.GetHitResult().PhysMaterial.Get()))
	{
		MoveParams.Friction = MoverPhysMat->CalculateFrictionCoefficient(MoveParams.Friction);
		MoveParams.Acceleration = MoverPhysMat->GetAccelerationOverride().Get(MoveParams.Acceleration);
		MoveParams.Deceleration = MoverPhysMat->GetDecelerationOverride().Get(MoveParams.Deceleration);
	}
}

void UBotaniMM_WallRunning::ApplyMovement(FMoverTickEndData& OutputState)
{
	// Get the mover component
	UMoverComponent* MoverComponent = GetMoverComponent<UMoverComponent>();
	check(MoverComponent);

	// What is up ??
	const FVector UpDirection = MoverComponent->GetUpDirection();

	// Get the settings
	const UBotaniWallRunMovementSettings* BotaniWallRunSettings = MoverComponent->FindSharedSettings<UBotaniWallRunMovementSettings>();
	check(BotaniWallRunSettings);

	// Initialize our wall running data
	FCommonMoveData WallRunData;
	WallRunData.MoveRecord.SetDeltaSeconds(DeltaTime);
	WallRunData.OriginalMoveDelta = ProposedMove->LinearVelocity * DeltaTime;
	WallRunData.CurrentMoveDelta = WallRunData.OriginalMoveDelta;

	// Get the wall data
	FWallCheckResult CurrentWall;
	if (!SimBlackboard->TryGet<FWallCheckResult>(BotaniMover::Blackboard::LastWallResult, CurrentWall))
	{
		//@TODO: Exit move and queue falling one? But this should be handled by the transition, not here.
		ensure(false);
	}

	const FVector WallTangent = FVector::VectorPlaneProject(
		CurrentWall.GetHitResult().ImpactNormal,
		UpDirection).GetSafeNormal();

	//@TODO: Maybe make sure the target orient quat is only along the up direction?
	const FRotator StartingOrient = StartingSyncState->GetOrientation_WorldSpace();
	FRotator TargetOrient = StartingOrient;
	WallRunData.TargetOrientQuat = TargetOrient.Quaternion();
	if (BotaniMovementSettings->bShouldRemainUpright)
	{
		WallRunData.TargetOrientQuat = FRotationMatrix::MakeFromZX(UpDirection, WallRunData.TargetOrientQuat.GetForwardVector()).ToQuat();
	}

	// Draw a debug line for the wall tangent
#if ENABLE_VISUAL_LOG
	if (BotaniWallRunSettings->bDrawWallRunDebug)
	{
		UKismetSystemLibrary::DrawDebugArrow(
			this,
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentLocation() - (WallTangent * 150.f),
			2.f,
			FLinearColor::Yellow,
			10.f);

		UKismetSystemLibrary::DrawDebugArrow(this,
				CurrentWall.GetHitResult().Location,
				CurrentWall.GetHitResult().Location + CurrentWall.GetHitResult().Normal * 100.f,
				2.f,
				FLinearColor::Green,
				10.f);

		UKismetSystemLibrary::DrawDebugArrow(
			this,
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentLocation() + WallRunData.CurrentMoveDelta,
			2.f,
			FLinearColor::Yellow,
			10.f);
	}
	{
		//@TODO: Not sure about visual logger here, don't want to spam the log with dumb arrows and stuff
		//using namespace BotaniMover::VLog;
		//VisLogCommand(MoverComponent->GetOwner(),.......
	}
#endif

	// Attempt first move
	UMovementUtils::TrySafeMoveUpdatedComponent(
		MovingComponentSet,
		WallRunData.CurrentMoveDelta,
		WallRunData.TargetOrientQuat,
		true,
		WallRunData.MoveHitResult,
		ETeleportType::None,
		WallRunData.MoveRecord);

	const float WallDot = FVector::DotProduct(CurrentWall.GetHitResult().Normal, UpDirection);

	// Apply the wall attraction force to make the player stick onto the wall
	if (WallDot >= 0.f)
	{
		const FVector WallAttractionForce =(
				-CurrentWall.GetHitResult().Normal *
				GetBotaniWallRunFloatProp(WallRun_AttractionForceMagnitude) *
				DeltaTime);

		UMovementUtils::TrySafeMoveUpdatedComponent(
			MovingComponentSet,
			WallAttractionForce,
			WallRunData.TargetOrientQuat,
			true,
			WallRunData.MoveHitResult,
			ETeleportType::None,
			WallRunData.MoveRecord);

		GEngine->AddOnScreenDebugMessage(1112, 1.f, FColor::Blue,
		FString::Printf(TEXT("Attraction Force: %s"), *WallAttractionForce.ToCompactString()));
	}

	CaptureFinalState(CurrentWall, DeltaTime * WallRunData.PercentTimeAppliedSoFar, OutputState, WallRunData.MoveRecord);
}

void UBotaniMM_WallRunning::PostMove(FMoverTickEndData& OutputState)
{
	Super::PostMove(OutputState);
}

void UBotaniMM_WallRunning::CaptureFinalState(
	const FWallCheckResult& WallResult,
	float DeltaSecondsUsed,
	FMoverTickEndData& TickEndData,
	FMovementRecord& Record)
{
	const FVector FinalLocation = MovingComponentSet.UpdatedPrimitive->GetComponentLocation();

	// Save the current time as the last wall run time
	SimBlackboard->Set<float>(BotaniMover::Blackboard::LastWallRunTime, CurrentSimulationTime);

	/*// Check for refunds
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

	Record.SetDeltaSeconds(DeltaSecondsUsed);*/

	OutDefaultSyncState->SetTransforms_WorldSpace(
			MovingComponentSet.UpdatedComponent->GetComponentLocation(),
			MovingComponentSet.UpdatedComponent->GetComponentRotation(),
			Record.GetRelevantVelocity(),
			nullptr);

	// Set the component's velocity
	MovingComponentSet.UpdatedComponent->ComponentVelocity = OutDefaultSyncState->GetVelocity_WorldSpace();
}
