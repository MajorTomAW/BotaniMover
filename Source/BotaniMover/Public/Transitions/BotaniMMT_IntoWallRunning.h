// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMMT_BaseWallRunning.h"
#include "MovementModeTransition.h"

#include "BotaniMMT_IntoWallRunning.generated.h"

class UBaseMovementMode;
class FObjectInitializer;
struct FSimulationTickParams;
struct FTransitionEvalResult;

#define MY_API BOTANIMOVER_API

/** Movement Mode Transition that handles transitioning into Wall Running if any valid walls nearby. */
UCLASS(MinimalAPI, DisplayName="Botani MMT: Into Wall Running")
class UBotaniMMT_IntoWallRunning : public UBotaniMMT_BaseWallRunning
{
	GENERATED_BODY()

public:
	MY_API UBotaniMMT_IntoWallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	MY_API virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
	MY_API virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** Name of the wall running movement mode to transition to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mode)
	FName WallRunningMovementMode = BotaniMover::ModeNames::WallRunning;
};

#undef MY_API
