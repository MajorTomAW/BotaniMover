// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefaultMovementSet/LayeredMoves/MultiJumpLayeredMove.h"

#include "BotaniLM_MultiJump.generated.h"

USTRUCT(BlueprintType)
struct BOTANIMOVER_API FBotaniLM_MultiJump : public FLayeredMove_MultiJump
{
	GENERATED_BODY()

public:
	FBotaniLM_MultiJump();
	virtual ~FBotaniLM_MultiJump() override;

	//~ Begin FLayeredMoveBase Interface
	virtual FLayeredMoveBase* Clone() const override;
	virtual void NetSerialize(FArchive& Ar) override;
	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual FString ToSimpleString() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
	//~ End FLayeredMoveBase Interface

	//~ Begin FLayeredMove_MultiJump Interface
	virtual bool GenerateMove(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, const UMoverComponent* MoverComp, UMoverBlackboard* SimBlackboard, FProposedMove& OutProposedMove) override;
	virtual bool WantsToJump(const FMoverInputCmdContext& InputCmd) override;
	virtual bool PerformJump(const FMoverDefaultSyncState* SyncState, const FMoverTimeStep& TimeStep, const UMoverComponent* MoverComp, FProposedMove& OutProposedMove) override;
	//~ End FLayeredMove_MultiJump Interface

public:
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