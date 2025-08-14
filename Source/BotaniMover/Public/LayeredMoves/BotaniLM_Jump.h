// Author: Tom Werner (MajorT), 2025

#pragma once

#include "LayeredMove.h"

#include "BotaniLM_Jump.generated.h"

/** Layered move for jumping, keeps track of initial jump simulation time through a blackboard key. */
USTRUCT(BlueprintType)
struct BOTANIMOVER_API FBotaniLM_Jump : public FLayeredMoveBase
{
	GENERATED_BODY()

public:
	FBotaniLM_Jump();
	virtual ~FBotaniLM_Jump() override;

	//~ Begin FLayeredMoveBase Interface
	virtual bool GenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, const UMoverComponent* MoverComp, UMoverBlackboard* SimBlackboard, FProposedMove& OutProposedMove) override;

	virtual FLayeredMoveBase* Clone() const override;
	virtual void NetSerialize(FArchive& Ar) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
	//~ End FLayeredMoveBase Interface

public:
	/** Upwards impulse in cm/s, to be applied in the direction the target actor considers up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	float UpwardsSpeed;

	/** Optional momentum carried on from before the jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	FVector Momentum;

	/** Air control percentage during the jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	float AirControl;

	/** If true, the layered move will end if the player releases the jump button */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	bool bTruncateOnJumpRelease;

	/** If true, the layered move will override movement plane velocity with the provided Momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	bool bOverrideHorizontalMomentum;

	/** If true, the layered move will override the vertical velocity with the provided Momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mover)
	bool bOverrideVerticalMomentum;
};

template<>
struct TStructOpsTypeTraits< FBotaniLM_Jump > : public TStructOpsTypeTraitsBase2<FBotaniLM_Jump>
{
	enum
	{
		WithCopy = true
	};
};
