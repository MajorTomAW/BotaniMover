// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMMT_BaseWallRunning.h"

#include "BotaniMMT_OutOfWallRunning.generated.h"

class UBaseMovementMode;
class FObjectInitializer;
struct FSimulationTickParams;
struct FTransitionEvalResult;

#define MY_API BOTANIMOVER_API

/**
 * Not sure if it is the best approach to separate "into"- and "out-of"- wall running transitions into different classes.
 * There is a lot of duplicated code, but keeping them separated gives me more customization options for
 * conditions to stay inside wall running, etc., without having to perform a bunch of if-else checks in the same class.
 */

/** Movement Mode Transition that handles transitioning out of wall running when wall running is not possible anymore. */
UCLASS(MinimalAPI, DisplayName="Botani MMT: Out Of Wall Running")
class UBotaniMMT_OutOfWallRunning : public UBotaniMMT_BaseWallRunning
{
	GENERATED_BODY()

public:
	MY_API UBotaniMMT_OutOfWallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	MY_API virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
	MY_API virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** Name of the falling movement mode to transition to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Mode)
	FName FallingMovementMode = DefaultModeNames::Falling;
};

#undef MY_API
