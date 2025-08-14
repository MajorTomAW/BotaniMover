// Author: Tom Werner (MajorT), 2025

#pragma once

#include "EBotaniSlideRailType.h"
#include "UObject/Interface.h"

#include "IBotaniSlideRailInterface.generated.h"

class UObject;
class USplineComponent;
enum class EBotaniSlideRailType : uint8;

/** Interface for slid rails that can be used to communicate with Mover. */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UBotaniSlideRailInterface : public UInterface
{
	GENERATED_BODY()
};

class IBotaniSlideRailInterface
{
	GENERATED_BODY()

public:
	/** Returns the spline component used for the slide rail. */
	UFUNCTION(BlueprintCallable, Category=SlideRail)
	virtual USplineComponent* GetSlideRailSplineComponent() = 0;

	/** Returns the slide rail type. */
	UFUNCTION(BlueprintCallable, Category=SlideRail)
	virtual EBotaniSlideRailType GetSlideRailType() const = 0;
};
