// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "MoverDataModelTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "WallRunningMovementUtils.generated.h"

struct FMoverSyncState;
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

/** Data about the wall for wall running movement, used by Mover simulations */
USTRUCT(BlueprintType)
struct FWallCheckResult
{
	GENERATED_BODY()

public:
	FWallCheckResult()
		: bBlockingHit(false)
		, bRunAbleWall(false)
		, WallDist(0.f)
		, HitResult(1.f)
	{
	}

	bool IsRunAbleWall() const
	{
		return bBlockingHit && bRunAbleWall;
	}

	void Clear()
	{
		bBlockingHit = false;
		bRunAbleWall = false;
		WallDist = 0.f;
		HitResult.Reset(1.f, false);
	}

	float GetDistanceToWall() const
	{
		return WallDist;
	}

	FHitResult GetHitResult() const
	{
		return HitResult;
	}

	MY_API void SetFromHitResult(const FHitResult& InHit, const float InWallDist, const bool bIsRunAbleWall);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category=Wall)
	uint8 bBlockingHit : 1;

	/** True if the hit found a valid wall to run on. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category=Wall)
	uint8 bRunAbleWall : 1;

	/** The distance to the wall, computed from the trace. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category=Wall)
	float WallDist;

	/** Hit result of the test that found a wall. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category=Wall)
	FHitResult HitResult;
};

/** Input parameters for controlled wall running movement function */
USTRUCT(BlueprintType)
struct FWallRunMoveParams
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	EMoveInputType MoveInputType = EMoveInputType::DirectionalIntent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FRotator OrientationIntent = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector PriorVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FRotator PriorOrientation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector MoveInput = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float MaxSpeed = 800.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float Acceleration = 4000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float Deceleration = 8000.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float Friction = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float TurningRate = 500.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float TurningBoost = 8.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector WallNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	float DeltaSeconds = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FVector UpDirection = FVector::UpVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	FQuat WorldToGravityQuat = FQuat::Identity;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Mover)
	bool bUseAccelerationForVelocityMove = true;
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

	/** Returns true if we are currently running on a wall */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool IsWallRunning(const FSimulationTickParams& TickParams);

	/** Performs the wall trace to find a valid wall to run on */
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool PerformWallTrace(const FMovingComponentSet& MovingComps, FHitResult& OutWallHit, float WallTraceVectorsHeadDelta, float WallTraceVectorsTailDelta, EBotaniWallRunSide WallSide = Wall_Both);
	UFUNCTION(BlueprintCallable, Category = Mover)
	static MY_API bool PerformWallTrace_Mover(const UMoverComponent* MoverComponent, FHitResult& OutWallHit, float WallTraceVectorsHeadDelta, float WallTraceVectorsTailDelta, EBotaniWallRunSide WallSide = Wall_Both);

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

	/** Returns a current blackboard value as a FWallCheckResult, by its blackboard key. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Mover|Blackboard")
	static MY_API FWallCheckResult GetBlackboardValueAsWallCheckResult(const UMoverBlackboard* Blackboard, FName KeyName);
};

#undef MY_API
