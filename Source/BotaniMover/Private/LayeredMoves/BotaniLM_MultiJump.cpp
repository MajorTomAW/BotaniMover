// Copyright © 2025 Playton. All Rights Reserved.


#include "LayeredMoves/BotaniLM_MultiJump.h"

#include "MoverComponent.h"
#include "MoverDataModelTypes.h"
#include "MoverSimulationTypes.h"
#include "MoveLibrary/FloorQueryUtils.h"
#include "MoveLibrary/MoverBlackboard.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"


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
	Ar << AirControl;
	Ar << bTruncateOnJumpRelease;
	Ar << bOverrideHorizontalMomentum;
	Ar << bOverrideVerticalMomentum;
}

FString FBotaniLM_MultiJump::ToSimpleString() const
{
	return FString::Printf(TEXT("Botani-LM MultiJump"));
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
	if (const TObjectPtr<const UCommonLegacyMovementSettings> CommonLegacySettings = MoverComp->FindSharedSettings<UCommonLegacyMovementSettings>())
	{
		OutProposedMove.PreferredMode = CommonLegacySettings->AirMovementModeName;
	}

	const FVector UpDir = MoverComp->GetUpDirection();

	const FVector ImpulseVelocity = bOverrideVerticalMomentum
		? UpDir * UpwardsSpeed
		: SyncState->GetVelocity_WorldSpace().ProjectOnToNormal(UpDir);

	switch (MixMode)
	{
	case EMoveMixMode::AdditiveVelocity:
		{
			OutProposedMove.LinearVelocity = ImpulseVelocity;
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
	const UCommonLegacyMovementSettings* CommonLegacySettings = MoverComp->FindSharedSettings<UCommonLegacyMovementSettings>();
	check(CommonLegacySettings);

	const FMoverDefaultSyncState* SyncState = StartState.SyncState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();
	check(SyncState);
	
	const FCharacterDefaultInputs* CharacterInputs = StartState.InputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>();

	// if we're no longer falling, set the duration to zero
	if (TimeStep.BaseSimTimeMs != StartSimTimeMs && StartState.SyncState.MovementMode != CommonLegacySettings->AirMovementModeName)
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
	if (CharacterInputs && CharacterInputs->bIsJumpJustPressed)
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
		if (!CharacterInputs || !CharacterInputs->bIsJumpPressed)
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
