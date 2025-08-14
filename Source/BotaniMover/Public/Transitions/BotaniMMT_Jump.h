// Author: Tom Werner (MajorT), 2025

#pragma once


#include "MovementModeTransition.h"
#include "GameplayTagContainer.h"

#include "BotaniMMT_Jump.generated.h"

class UObject;
class UBaseMovementModeTransition;
struct FSimulationTickParams;
struct FTransitionEvalResult;
struct FGameplayTag;
struct FGameplayTagContainer;
struct FFrame;

#define MY_API BOTANIMOVER_API

/** Handles movement mode transitions due to jump inputs. */
UCLASS(DisplayName="Botani MMT: Jumping", MinimalAPI)
class UBotaniMMT_Jump : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	MY_API UBotaniMMT_Jump(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	MY_API virtual FTransitionEvalResult Evaluate_Implementation(const FSimulationTickParams& Params) const override;
	MY_API virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** Name of the movement mode to transition to for the jump */
	UPROPERTY(EditAnywhere, Category=Mode)
	FName JumpMovementMode;

	/** If true, the character's movement plane velocity will be overridden by the provided computed momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=LocalOverrides, DisplayName="Jump Overrides Movement Plane Velocity")
	TOptional<bool> bJumpOverridesMovementPlaneVelocity;

	/** If true, the character's vertical velocity will be overridden by the provided computed momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=LocalOverrides, DisplayName="Jump Overrides Vertical Velocity")
	TOptional<bool> bJumpOverridesVerticalVelocity;

	/** If true, any floor velocity will be added to the overridden velocity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=LocalOverrides, DisplayName="Jump Adds Floor Velocity")
	TOptional<bool> bJumpAddsFloorVelocity;

	/** If true, the character will keep any existing movement plane velocity from before jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=LocalOverrides, DisplayName="Jump Keeps Previous Velocity")
	TOptional<bool> bJumpKeepsPreviousVelocity;

	/** If true, the character will keep any existing vertical velocity from before jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=LocalOverrides, meta=(EditCondition=bJumpKeepsPreviousVelocity), DisplayName="Jump Keeps Previous Vertical Velocity")
	TOptional<bool> bJumpKeepsPreviousVerticalVelocity;


	/** Tags required on the sync state to allow the jump transition */
	UPROPERTY(EditAnywhere, Category=Evaluation)
	FGameplayTagContainer JumpRequiredTags;

	/** If true, the jump transition will happen when the jump button is pressed */
	UPROPERTY(EditAnywhere, Category=Evaluation)
	uint8 bJumpWhenButtonPressed : 1;

	/** If this transition is triggered, send this gameplay event to the owner */
	UPROPERTY(EditAnywhere, Category=Trigger)
	FGameplayTag TriggerEventTag;

	/** If a FName is provided, the simulation time of the jump will be saved to the Blackboard under that key */
	UPROPERTY(EditAnywhere, Category=Trigger)
	FName BlackboardTimeLoggingKey;
};

#undef MY_API
