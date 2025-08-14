// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "CommonMoverComponent.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"
#include "Modifiers/BotaniStanceModifier.h"

#include "BotaniMoverComponent.generated.h"

#define MY_API BOTANIMOVER_API

/**
 * Fires when a stance is changed, if stance handling is enabled (see @SetHandleStanceChanges)
 * Note: If a stance was just Activated it will fire with an invalid OldStance
 *		 If a stance was just Deactivated it will fire with an invalid NewStance
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBotaniMover_OnStanceChanged, EBotaniStanceMode, OldStance, EBotaniStanceMode, NewStance);

/** Mover component for Botani game. */
UCLASS(MinimalAPI, BlueprintType)
class UBotaniMoverComponent
	: public UCommonMoverComponent
{
	GENERATED_BODY()

public:
	MY_API UBotaniMoverComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UObject Interface
	MY_API virtual void BeginPlay() override;
	//~ End UObject Interface

	/** Returns whether this component is tasked with handling character stance changes, including crouching. */
	UFUNCTION(BlueprintGetter)
	MY_API bool GetHandleStanceChanges() const;

	/** If true, this component will process stancing changes and crouching inputs. */
	UFUNCTION(BlueprintSetter)
	MY_API void SetHandleStanceChanges(bool bInHandleStanceChanges);

	/** Returns true if the owner is currently wall running. */
	UFUNCTION(BlueprintPure, Category="Mover")
	MY_API virtual bool IsWallRunning() const;

protected:
	UFUNCTION()
	MY_API virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd);

	/** Binds the simulation tick functions to the mover component. */
	MY_API virtual void OnHandlerSettingChanged();

protected:
	/** Delegate to be called whenever the actor's stance changes. */
	UPROPERTY(BlueprintAssignable, Category=BotaniMover)
	FBotaniMover_OnStanceChanged OnStanceChanged;

	/** Stance handle to keep track of the modifier responsible for the current stance. */
	UPROPERTY(Transient)
	FMovementModifierHandle StanceModifierHandle;

	/** Whether this component should directly handle stance changes, including crouching input. */
	UPROPERTY(EditAnywhere, BlueprintGetter=GetHandleStanceChanges, BlueprintSetter=SetHandleStanceChanges, Category=BotaniMover)
	uint8 bHandleStanceChanges : 1 = 1;
};

#undef MY_API
