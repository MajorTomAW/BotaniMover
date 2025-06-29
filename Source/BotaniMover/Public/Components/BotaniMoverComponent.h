// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonMoverComponent.h"

#include "BotaniMoverComponent.generated.h"

/** Mover component for Botani game. */
UCLASS()
class BOTANIMOVER_API UBotaniMoverComponent
	: public UCommonMoverComponent
{
	GENERATED_BODY()

public:
	UBotaniMoverComponent();

	//~ Begin UObject Interface
	virtual void BeginPlay() override;
	//~ End UObject Interface

public:
};
