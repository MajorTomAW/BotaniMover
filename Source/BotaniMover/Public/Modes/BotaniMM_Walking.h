// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BotaniMM_GroundBase.h"
#include "BotaniMM_Walking.generated.h"

/** Walking mode for Botani game. */
UCLASS(DisplayName="Botani: Walking Mode")
class BOTANIMOVER_API UBotaniMM_Walking : public UBotaniMM_GroundBase
{
	GENERATED_BODY()

public:
	UBotaniMM_Walking(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBotaniMM_GroundBase Interface
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

	virtual void PostMove(FMoverTickEndData& OutputState) override;

	/** Handles any movement mode transitions as a result of falling */
	virtual bool HandleFalling(FMoverTickEndData& OutputState, FMovementRecord& MoveRecord, FHitResult& Hit, float TimeAppliedSoFar) override;
	//~ End UBotaniMM_GroundBase Interface

protected:
	//~ Begin UCommonMovementMode Interface

	/** Walking needs to account for based movement so it overrides the default disabled check */
	virtual bool CheckIfMovementDisabled();
	//~ End UCommonMovementMode Interface

protected:
	/** Slope boost multiplier for the walking mode. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Walking, meta=(ClampMin=0.0, UIMin=0.0))
	float SlopeBoostMultiplier;
	
	/** Gameplay Tag to use when sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Tags)
	FGameplayTag SprintingTag;
	
	/** Maximum ground speed while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sprinting, meta=(ClampMin=0.0, UIMin=0.0, ForceUnits="cm/s"))
	float Sprint_MaxSpeed;

	/** Maximum ground acceleration while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sprinting, meta=(ClampMin=0.0, UIMin=0.0, ForceUnits="cm/s^2"))
	float Sprint_Acceleration;

	/** Maximum ground deceleration while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sprinting, meta=(ClampMin=0.0, UIMin=0.0, ForceUnits="cm/s^2"))
	float Sprint_Deceleration;

	/** Maximum ground turning rate while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sprinting, meta=(ClampMin=0.0, UIMin=0.0, ForceUnits="deg/s"))
	float Sprint_TurningRate;

	/** Maximum ground turn rate boost while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Sprinting, meta=(ClampMin=0.0, UIMin=0.0, ForceUnits="deg/s"))
	float Sprint_TurnRateBoost;

	/** Gameplay Event to send to the owner when the player starts sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Events)
	FGameplayTag SprintStartEventTag;

	/** Gameplay Event to send to the owner when the player stops sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Events)
	FGameplayTag SprintStopEventTag;
};
