// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "MoverDataModelTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "WallRunningMovementUtils.generated.h"

struct FMovingComponentSet;
struct FSimulationTickParams;
struct FHitResult;
class UMoverComponent;

/** Enum for specifying which wall side to trace for wall running */
UENUM(BlueprintType)
enum EBotaniWallRunSide
{
	Wall_Error = 0,
	Wall_Left = 1 << 0,
	Wall_Right = 1 << 1,
	Wall_Both = Wall_Left | Wall_Right
};

#define MY_API BOTANIMOVER_API

/** Input parameters for controlled wall running movement function */
USTRUCT(BlueprintType)
struct FWallRunMoveParams
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	EMoveInputType MoveInputType = EMoveInputType::DirectionalIntent;
};


/** WallRunningMovementUtils: a collection of stateless static functions for computing wall running behavior. */
UCLASS(MinimalAPI)
class UWallRunningMovementUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Generates a new movement based on move/orientation intents and the prior state for the wall running move */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API FProposedMove ComputeControlledWallRunMove(const FWallRunMoveParams& InParams);
	
	/** Performs the wall trace to find a valid wall to run on */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool PerformWallTrace(const FMovingComponentSet& MovingComps, FHitResult& OutWallHit, float WallTraceVectorsHeadDelta, float WallTraceVectorsTailDelta, EBotaniWallRunSide WallSide = Wall_Both);

	/** Constructs the trace parameters to ignore the owner of the mover component */
	static MY_API FCollisionQueryParams GetIgnoreOwnerQueryParams(const UMoverComponent* InMoverComponent);

	/** Returns the angle of a wall hit result relative to the up direction */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API float GetWallAngle(const FHitResult& WallHit, const FVector& UpDirection = FVector::UpVector);

	/** Returns whether we want to fall off the wall based on the current wall hit */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool ShouldFallOffWall(const FHitResult& WallHit, const float& PullAwayAngle, const FVector& MoveIntent);

	/** Checks if we are high enough above any floor to consider starting a wall run */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool IsHighEnoughForWallRun(const FMovingComponentSet& MovingComps, float MinHeightAboveFloor = 100.f, const FVector& UpDirection = FVector::UpVector);
};

#undef MY_API