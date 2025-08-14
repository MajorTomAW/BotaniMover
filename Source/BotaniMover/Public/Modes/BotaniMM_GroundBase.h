// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "CommonGroundModeBase.h"
#include "MoveLibrary/GroundMovementUtils.h"

#include "BotaniMM_GroundBase.generated.h"

class UBotaniCommonMovementSettings;
class UBotaniMoverSettings;
class UAbilitySystemComponent;

/** Ground movement mode base class for the Botani game. */
UCLASS(Abstract)
class BOTANIMOVER_API UBotaniMM_GroundBase : public UCommonGroundModeBase
{
	GENERATED_BODY()

public:
	UBotaniMM_GroundBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	//~ Begin UCommonGroundModeBase Interface
	virtual void OnRegistered(const FName ModeName) override;
	virtual void OnUnregistered() override;

	virtual void ApplyMovement(FMoverTickEndData& OutputState) override;
	virtual void ValidateFloor(float FloorSweepDistance, float MaxWalkableSlopeCosine) override;
	//~ End UCommonGroundModeBase Interface

	/** Applies the physical ground friction to the move parameters based on the physical material of the floor. */
	virtual void ApplyPhysicalGroundFriction(FGroundMoveParams& MoveParams, const FFloorCheckResult& FloorToUse, const bool bOverrideFriction = true) const;

	/** Returns the effective max speed for the movement mode, taking into account move restrictions. */
	virtual float GetEffectiveMaxSpeed(const float& InMaxSpeed, const FMoverTickStartData& StartState) const;

	/** Returns the ability system component of the owning actor, if available. */
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	/** Pointer to the botani mover settings. */
	UPROPERTY()
	TObjectPtr<const UBotaniMoverSettings> BotaniMoverSettings;

	/** Pointer to the botani movement settings. */
	UPROPERTY()
	TObjectPtr<const UBotaniCommonMovementSettings> BotaniMovementSettings;

private:
	/** Transient pointer to the owning actor's ability system component. */
	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
