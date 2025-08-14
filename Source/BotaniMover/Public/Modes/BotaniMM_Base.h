// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "CommonMovementMode.h"

#include "BotaniMM_Base.generated.h"

class UObject;
class UAbilitySystemComponent;
class UBotaniCommonMovementSettings;
class UBotaniMoverSettings;

/** Movement mode base class for any movement modes that are not Ground movement modes. */
UCLASS(Abstract)
class BOTANIMOVER_API UBotaniMM_Base : public UCommonMovementMode
{
	GENERATED_BODY()

public:
	UBotaniMM_Base(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UCommonMovementMode Interface
	virtual void OnRegistered(const FName ModeName) override;
	virtual void OnUnregistered() override;
	//~ End UCommonMovementMode Interface

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
