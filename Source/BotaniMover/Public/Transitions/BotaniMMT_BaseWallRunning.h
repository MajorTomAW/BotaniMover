// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "BotaniMMT_Base.h"
#include "BotaniMoverSettings.h"

#include "BotaniMMT_BaseWallRunning.generated.h"

#define MY_API BOTANIMOVER_API

class UBotaniWallRunMovementSettings;
/** Base class for all Wall Running transitions, provides a bunch of helper functions shared among other transitions. */
UCLASS(MinimalAPI, Abstract)
class UBotaniMMT_BaseWallRunning : public UBotaniMMT_Base
{
	GENERATED_BODY()

public:
	UBotaniMMT_BaseWallRunning(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UBaseMovementModeTransition Interface
	virtual void OnRegistered() override;
	virtual void OnUnregistered() override;
	//~ End UBaseMovementModeTransition Interface

protected:
	/** Internal helper function that checks if there is a valid wall to run on. */
	MY_API virtual bool CanStartWallRunning(const FSimulationTickParams& Params, FHitResult& OutHitResult) const;

protected:
	/** Wall run settings that this transition depends on. */
	UPROPERTY(Transient)
	TObjectPtr<const UBotaniWallRunMovementSettings> BotaniWallRunSettings;
};

#undef MY_API
