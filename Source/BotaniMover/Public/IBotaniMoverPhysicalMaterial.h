// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Interface.h"

#include "IBotaniMoverPhysicalMaterial.generated.h"

struct FGroundMoveParams;
struct FSimulationTickParams;
struct FHitResult;

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UBotaniMoverPhysicalMaterial : public UInterface
{
	GENERATED_BODY()
};

class IBotaniMoverPhysicalMaterial
{
	GENERATED_BODY()

public:
	virtual float CalculateFrictionCoefficient(const float& InFriction) const = 0;

	virtual TOptional<float> GetDecelerationOverride() const = 0;
	virtual TOptional<float> GetAccelerationOverride() const = 0;
};
