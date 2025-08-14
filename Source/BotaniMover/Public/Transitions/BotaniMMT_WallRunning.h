// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MovementModeTransition.h"

#include "BotaniMMT_WallRunning.generated.h"




/** Movement Mode Transition that handles transitioning into Wall Running if any walls nearby. */
class UE_DEPRECATED_FORGAME(5.6, "Consider using UBotaniMMT_IntoWallRunning or UBotaniMMT_OutOfWallRunning instead.") UDEPRECATED_BotaniMMT_WallRunning;
UCLASS(Deprecated="Consider using UBotaniMMT_IntoWallRunning or UBotaniMMT_OutOfWallRunning instead.")
class UDEPRECATED_BotaniMMT_WallRunning : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	UDEPRECATED_BotaniMMT_WallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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
