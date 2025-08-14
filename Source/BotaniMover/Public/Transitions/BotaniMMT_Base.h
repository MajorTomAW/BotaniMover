// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MovementModeTransition.h"

#include "BotaniMMT_Base.generated.h"

#define MY_API BOTANIMOVER_API

/** Base MMT class that handles sending gameplay events when the transition is triggered. */
UCLASS(Abstract, MinimalAPI)
class UBotaniMMT_Base : public UBaseMovementModeTransition
{
	GENERATED_BODY()

public:
	MY_API UBotaniMMT_Base(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	virtual void Trigger_Implementation(const FSimulationTickParams& Params) override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** If this transition is triggered, send this gameplay event to the owner */
	UPROPERTY(EditAnywhere, Category=Trigger)
	FGameplayTag TriggerEventTag;

	/** If a name is provided, it will be used to log the transition trigger time into the Blackboard under that key. */
	UPROPERTY(EditAnywhere, Category=Trigger)
	FName BlackboardTimeLoggingKey;

	/** If true, will create a visual log entry when the transition is triggered */
	UPROPERTY(EditAnywhere, Category=Trigger)
	bool bShouldCreateVisLogEntry = true;
};

#undef MY_API
