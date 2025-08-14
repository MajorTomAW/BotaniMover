// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMM_Base.h"

#include "BotaniMM_WallRunning.generated.h"

struct FWallRunMoveParams;
struct FWallCheckResult;
/** Wall Running mode for Botani game. */
UCLASS(DisplayName="Botani MM: Wall Running", MinimalAPI)
class UBotaniMM_WallRunning : public UBotaniMM_Base
{
	GENERATED_BODY()

public:
	UBotaniMM_WallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UCommonMovementMode Interface

	/** Clears blackboard fields on deactivation */
	virtual void Deactivate() override;

	/** Generates the movement data that will be consumed by the simulation tick */
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

protected:
	/** Applies the physical ground friction to the move parameters based on the physical material of the floor. */
	virtual void ApplyPhysicalWallFriction(FWallRunMoveParams& MoveParams, const FWallCheckResult& FloorToUse, const bool bOverrideFriction = true) const;

	/** Gets additional falling data */
	virtual bool PrepareSimulationData(const FSimulationTickParams& Params) override;

	/** Handles most of the actual movement, including collision recovery  */
	virtual void ApplyMovement(FMoverTickEndData& OutputState) override;

	/** Handles any additional behaviors after the updated component's final position and velocity have been computed */
	virtual void PostMove(FMoverTickEndData& OutputState) override;

	//~ End UCommonMovementMode Interface


	/** Captures the final movement values and sends it to the Output Sync State */
	void CaptureFinalState(const FWallCheckResult& WallResult, float DeltaSecondsUsed, FMoverTickEndData& TickEndData, FMovementRecord& Record);

protected:
	/** Effective Velocity calculated this frame */
	FVector EffectiveVelocity;
};
