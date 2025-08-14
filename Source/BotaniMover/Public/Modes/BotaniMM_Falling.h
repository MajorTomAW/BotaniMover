// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMM_Base.h"

#include "ScalableFloat.h"
#include "DefaultMovementSet/Modes/FallingMode.h"
#include "MoveLibrary/BasedMovementUtils.h"

#include "BotaniMM_Falling.generated.h"

/** Specialized Falling Mode */
UCLASS(DisplayName="Botani MM: Falling Mode")
class BOTANIMOVER_API UBotaniMM_Falling : public UBotaniMM_Base
{
	GENERATED_BODY()

public:
	UBotaniMM_Falling(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UCommonMovementMode Interface

	/** Clears blackboard fields on deactivation */
	virtual void Deactivate() override;

	/** Generates the movement data that will be consumed by the simulation tick */
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

protected:

	/** Gets additional falling data */
	virtual bool PrepareSimulationData(const FSimulationTickParams& Params) override;

	/** Handles most of the actual movement, including collision recovery  */
	virtual void ApplyMovement(FMoverTickEndData& OutputState) override;

	/** Handles any additional behaviors after the updated component's final position and velocity have been computed */
	virtual void PostMove(FMoverTickEndData& OutputState) override;

	/** Captures the final movement values and sends it to the Output Sync State */
	void CaptureFinalState(const FFloorCheckResult& FloorResult, float DeltaSecondsUsed, FMoverTickEndData& TickEndData, FMovementRecord& Record);

	/**
	 * Called at the end of the tick in falling mode. Handles checking any landings that should occur and switching to specific modes
	 * (i.e. landing on a walkable surface would switch to the walking movement mode)
	 */
	UFUNCTION(BlueprintCallable, Category = Mover)
	virtual void ProcessLanded(const FFloorCheckResult& FloorResult, FVector& Velocity, FRelativeBaseInfo& BaseInfo, FMoverTickEndData& TickEndData) const;


	//~ End UCommonMovementMode Interface

public:
	/**
	 * If true, actor will land and lose all speed in the vertical direction upon landing. If false, actor's vertical speed will be redirected based on the surface normal it hit.
	 * Note: Actor's horizontal speed will not be affected if true. If false, horizontal speed may be increased on landing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landing)
	bool bCancelVerticalSpeedOnLanding;

protected:
	/** Gameplay Event to send to the actor when we landed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Events)
	FGameplayTag LandingEventTag;

protected:
	/** Effective Velocity calculated this frame */
	FVector EffectiveVelocity;
};
