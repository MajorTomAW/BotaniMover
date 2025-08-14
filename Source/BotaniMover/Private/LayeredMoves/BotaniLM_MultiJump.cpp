// Author: Tom Werner (MajorT), 2025


#include "LayeredMoves/BotaniLM_MultiJump.h"

#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverAbilityInputs.h"
#include "BotaniMoverSettings.h"
#include "BotaniMoverVLogHelpers.h"
#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/MoverBlackboard.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniLM_MultiJump)

FBotaniLM_MultiJump::FBotaniLM_MultiJump()
{
	DurationMs = 0.f;
	MixMode = EMoveMixMode::AdditiveVelocity;

	UpwardsSpeed = 0.f;
	Momentum = FVector::ZeroVector;

	AirControl = 1.f;

	bTruncateOnJumpRelease = true;
	bOverrideHorizontalMomentum = false;
	bOverrideVerticalMomentum = false;
}

FBotaniLM_MultiJump::~FBotaniLM_MultiJump()
{
}

FLayeredMoveBase* FBotaniLM_MultiJump::Clone() const
{
	FBotaniLM_MultiJump* CopyPtr = new FBotaniLM_MultiJump(*this);
	return CopyPtr;
}

void FBotaniLM_MultiJump::NetSerialize(FArchive& Ar)
{
	Super::NetSerialize(Ar);

	Ar << Momentum;
	Ar << AirControl.Value; //@TODO: Need to properly serialize FScalableFloat ??
	Ar << bTruncateOnJumpRelease;
	Ar << bOverrideHorizontalMomentum;
	Ar << bOverrideVerticalMomentum;
}

FString FBotaniLM_MultiJump::ToSimpleString() const
{
	return FString::Printf(TEXT("Botani LM: Multi-Jump"));
}

void FBotaniLM_MultiJump::AddReferencedObjects(class FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
}

bool FBotaniLM_MultiJump::WantsToJump(const FMoverInputCmdContext& InputCmd)
{
	return Super::WantsToJump(InputCmd);
}

bool FBotaniLM_MultiJump::PerformJump(
	const FMoverDefaultSyncState* SyncState,
	const FMoverTimeStep& TimeStep,
	const UMoverComponent* MoverComp,
	FProposedMove& OutProposedMove)
{
	TimeOfLastJumpMS = TimeStep.BaseSimTimeMs;
	if (const TObjectPtr<const UBotaniMoverSettings> BotaniMoverSettings = MoverComp->FindSharedSettings<UBotaniMoverSettings>())
	{
		OutProposedMove.PreferredMode = BotaniMoverSettings->AirMovementModeName;
	}

	const FVector UpDir = MoverComp->GetUpDirection();

	// We can either override vertical velocity with the provided momentum or grab it from the sync state.
	FVector UpwardsVelocity = bOverrideVerticalMomentum
		? Momentum.ProjectOnToNormal(UpDir)
		: SyncState->GetVelocity_WorldSpace().ProjectOnToNormal(UpDir);

	// We can either override move plane velocity with the provided momentum or grab it from the sync state.
	const FVector PlaneVelocity = bOverrideHorizontalMomentum
		? Momentum - UpwardsVelocity
		: SyncState->GetVelocity_WorldSpace() - UpwardsVelocity;

	// Apply the upwards speed to the upwards velocity
	UpwardsVelocity += UpDir * UpwardsSpeed;

	// Calculate the final impulse
	const FVector ImpulseVelocity = UpwardsVelocity + PlaneVelocity;

	switch (MixMode)
	{
	case EMoveMixMode::AdditiveVelocity:
		{
			OutProposedMove.LinearVelocity = ImpulseVelocity;
#if ENABLE_VISUAL_LOG
			{
				using namespace BotaniMover::VLog;
				VisLogCommand(MoverComp->GetOwner(),
					FVLogDrawCommand::DrawArrow(
						SyncState->GetLocation_WorldSpace(),
						SyncState->GetLocation_WorldSpace() + ImpulseVelocity,
						FColor::Purple));
			}
#endif
			break;
		}

	case EMoveMixMode::OverrideAll:
	case EMoveMixMode::OverrideVelocity:
		{
			// Jump impulse overrides vertical velocity while maintaining the rest
			const FVector PriorVelocityWS = SyncState->GetVelocity_WorldSpace();

			const FVector StartingNonUpwardsVelocity = bOverrideHorizontalMomentum
				? PriorVelocityWS - PriorVelocityWS.ProjectOnToNormal(UpDir)
				: PriorVelocityWS - ImpulseVelocity;

			OutProposedMove.LinearVelocity = StartingNonUpwardsVelocity + ImpulseVelocity;
#if ENABLE_VISUAL_LOG
			{
				using namespace BotaniMover::VLog;
				VisLogCommand(MoverComp->GetOwner(),
					FVLogDrawCommand::DrawArrow(
						SyncState->GetLocation_WorldSpace(),
						SyncState->GetLocation_WorldSpace() + ImpulseVelocity + StartingNonUpwardsVelocity,
						FColor::Purple));
				VisLogCommand(MoverComp->GetOwner(),
					FVLogDrawCommand::DrawDebugCapsule(MoverComp->GetUpdatedComponent(),
						FColor::Green,
						MoverComp->GetUpdatedComponent()->GetComponentQuat()));
			}
#endif
			break;
		}

	default:
		ensureMsgf(false, TEXT("Multi-Jump layered move has an invalid MixMode and will do nothing."));
		return false;
	}

	return true;
}

bool FBotaniLM_MultiJump::GenerateMove(
	const FMoverTickStartData& StartState,
	const FMoverTimeStep& TimeStep,
	const UMoverComponent* MoverComp,
	UMoverBlackboard* SimBlackboard,
	FProposedMove& OutProposedMove)
{
	const UBotaniMoverSettings* BotaniMoverSettings = MoverComp->FindSharedSettings<UBotaniMoverSettings>();
	check(BotaniMoverSettings);

	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(SyncState);

	const FBotaniMoverAbilityInputs* BotaniAbilityInputs = StartState.InputCmd.InputCollection.FindDataByType<FBotaniMoverAbilityInputs>();

	// if we're no longer falling, set the duration to zero
	if (TimeStep.BaseSimTimeMs != StartSimTimeMs && StartState.SyncState.MovementMode != BotaniMoverSettings->AirMovementModeName)
	{
		DurationMs = 0.0f;
	}

	OutProposedMove.MixMode = MixMode;

	FFloorCheckResult FloorHitResult;
	bool bValidBlackboard = SimBlackboard->TryGet(CommonBlackboard::LastFloorResult, OUT FloorHitResult);

	if (StartSimTimeMs == TimeStep.BaseSimTimeMs)
	{
		JumpsInAirRemaining = MaximumInAirJumps;
	}

	bool bPerformedJump = false;
	if (BotaniAbilityInputs && BotaniAbilityInputs->bJumpPressedThisFrame)
	{
		if (StartSimTimeMs == TimeStep.BaseSimTimeMs)
		{
			// if this was the first jump and its a valid floor we do the initial jump from walking and back out so we don't get extra jump
			if (bValidBlackboard && FloorHitResult.IsWalkableFloor())
			{
				bPerformedJump = PerformJump(SyncState, TimeStep, MoverComp, OutProposedMove);
				return bPerformedJump;
			}
		}

		// perform in air jump
		if (TimeStep.BaseSimTimeMs > TimeOfLastJumpMS && JumpsInAirRemaining > 0)
		{
			JumpsInAirRemaining--;
			bPerformedJump = PerformJump(SyncState, TimeStep, MoverComp, OutProposedMove);
		}
		else
		{
			// setting mix mode to additive when we're not adding any jump input so air movement acts as expected
			OutProposedMove.MixMode = EMoveMixMode::AdditiveVelocity;
		}
	}
	else
	{
		// setting mix mode to additive when we're not adding any jump input so air movement acts as expected
		OutProposedMove.MixMode = EMoveMixMode::AdditiveVelocity;

		// if we're no longer pressing jump, set the duration to zero to end the upwards impulse
		if (!BotaniAbilityInputs || !BotaniAbilityInputs->bIsJumpPressed)
		{
			DurationMs = 0.0f;
		}
	}

	// if we hit a valid floor and it's not the start of the move (since we could start this move on the ground) end this move
	/*if (/*(bValidBlackboard && FloorHitResult.IsWalkableFloor() && StartSimTimeMs < TimeStep.BaseSimTimeMs) || #1#JumpsInAirRemaining <= 0)
	{
		DurationMs = 0;
	}*/

	return bPerformedJump;
}
