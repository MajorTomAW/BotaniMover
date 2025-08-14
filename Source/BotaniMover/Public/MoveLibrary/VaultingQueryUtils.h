// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "VaultingQueryUtils.generated.h"

#define MY_API BOTANIMOVER_API

struct FMovingComponentSet;

/** Data about a vaulting path, used by mover simulations. */
USTRUCT(BlueprintType)
struct FVaultingPathCheckResult
{
	GENERATED_BODY()

	FVaultingPathCheckResult()
		: bBlockingHit(false)
		, bValidVaultingPath(false)
		, VaultingSpotDistance(0.f)
		, HitResult(1.f)
	{
	}

	/** Returns true if the vaulting path check is a valid vaulting path. */
	bool IsValidVaultingPath() const
	{
		return bBlockingHit && bValidVaultingPath;
	}

	/** Returns the distance from the start location to the vaulting spot. */
	float GetVaultingSpotDistance() const
	{
		return VaultingSpotDistance;
	}

	/** Sets the passed in hit-result with data from a line trace. */
	MY_API void SetFromLineTrace(const FHitResult& InHit, const float InLineDist, const bool bIsValidFaultingPath);

	void Clear()
	{
		bBlockingHit = false;
		bValidVaultingPath = false;
		VaultingSpotDistance = 0.f;
		HitResult.Reset(1.f, false);
	}

	/** True if there was a blocking hit in the vaulting path test that was NOT in initial penetration. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Vaulting)
	uint32 bBlockingHit : 1;

	/** True if the hit found a valid vaulting path, false otherwise. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Vaulting)
	uint32 bValidVaultingPath : 1;

	/** The distance to the vaulting spot, computed from the start location. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Vaulting)
	float VaultingSpotDistance;

	/** The hit result of the test that found a vaulting path. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = Vaulting)
	FHitResult HitResult;
};

/** VaultingQueryUtils: a collection of stateless static functions for a variety of operation involving vaulting checks. */
UCLASS(MinimalAPI)
class UVaultingQueryUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Performs a vaulting path query for the given moving component set, checking if a vaulting path may exist at the given location. */
	UFUNCTION(BlueprintCallable, Category = "Mover|Vaulting")
	static MY_API void FindVaultingPath(const FMovingComponentSet& MovingComps, float MaxVaultHeight, float MinVaultHeight, float VaultSweepDistance, uint8 VaultingSamples, const FVector& Location, const FRotator& Rotation, const FFloatRange& VaultingSlopeCosineRange, FVaultingPathCheckResult& OutVaultingResult);

	/** Verifies whether the vaulting path is vaultable, checking physical materials, slope angles, etc. */
	UFUNCTION(BlueprintCallable, Category = "Mover|Vaulting")
	static MY_API bool IsVaultingPathValid(const FHitResult& Hit, const FVector& UpDirection, const FFloatRange& VaultingSlopeCosineRange);
};

#undef MY_API
