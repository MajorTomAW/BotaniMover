// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MovementModeTransition.h"

#include "BotaniMMT_WallRunning.generated.h"




/** Movement Mode Transition that handles transitioning into Wall Running if any walls nearby. */
UCLASS(DisplayName="Botani MMT: Wall Running")
class UBotaniMMT_WallRunning : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	UBotaniMMT_WallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
	virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** Checks if there is a valid wall to run on */
	virtual bool CanStartWallRunning(const FSimulationTickParams& Params, FHitResult& OutWallHit) const;

protected:
	/** Name of the movement mode to transition to for the wall running */
	UPROPERTY(EditAnywhere, Category="Mode")
	FName WallRunningMovementMode;

	/** Name of the movement mode to transition to when wall running ends */
	UPROPERTY(EditAnywhere, Category="Mode")
	FName WallRunningEndMovementMode;

	/** If true, any floor velocity will be added to the overridden velocity */
	UPROPERTY(EditAnywhere, Category=Trigger)
	uint8 bAddFloorVelocity : 1;

	/** If true, the character will keep any existing movement plane velocity from before jumping */
	UPROPERTY(EditAnywhere, Category="Trigger")
	uint8 bKeepPreviousVelocity : 1;

	/** If true, the character will keep any existing vertical velocity from before jumping */
	UPROPERTY(EditAnywhere, Category="Trigger")
	uint8 bKeepPreviousVerticalVelocity : 1;

	/** If this transition is triggered, send this gameplay event to the owner */
	UPROPERTY(EditAnywhere, Category="Trigger")
	FGameplayTag TriggerEvent;

	/** If a FName is provided, the simulation time of the jump will be saved to the Blackboard under that key */
	UPROPERTY(EditAnywhere, Category="Trigger")
	FName BlackboardTimeLoggingKey;
};
