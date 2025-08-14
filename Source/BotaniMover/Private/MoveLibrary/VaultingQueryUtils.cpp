// Author: Tom Werner (MajorT), 2025


#include "MoveLibrary/VaultingQueryUtils.h"

#include "Components/BotaniMoverComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(VaultingQueryUtils)

void FVaultingPathCheckResult::SetFromLineTrace(
	const FHitResult& InHit,
	const float InLineDist,
	const bool bIsValidFaultingPath)
{
	bValidVaultingPath = bIsValidFaultingPath;
	HitResult = InHit;
	bBlockingHit = HitResult.bBlockingHit;
	VaultingSpotDistance = InLineDist;
}

void UVaultingQueryUtils::FindVaultingPath(
	const FMovingComponentSet& MovingComps,
	float MaxVaultHeight,
	float MinVaultHeight,
	float VaultSweepDistance,
	uint8 VaultingSamples,
	const FVector& Location,
	const FRotator& Rotation,
	const FFloatRange& VaultingSlopeCosineRange,
	FVaultingPathCheckResult& OutVaultingResult)
{
	// Reset our vaulting data
	OutVaultingResult.Clear();

	if (!MovingComps.UpdatedComponent->IsQueryCollisionEnabled())
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(FindVaultingPath), false, MovingComps.UpdatedPrimitive->GetOwner());

	FCollisionResponseParams ResponseParams;
	UMovementUtils::InitCollisionParams(MovingComps.UpdatedPrimitive.Get(), QueryParams, ResponseParams);
	const ECollisionChannel CollisionChannel = MovingComps.UpdatedPrimitive->GetCollisionObjectType();


	float PawnRadius = 0.0f;
	float PawnHalfHeight = 0.0f;
	FVector UpDirection = MovingComps.MoverComponent->GetUpDirection();

	// This is kinda bada, but for vaulting pawns i guess capsules or spheres are the most common components.
	if (UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(MovingComps.UpdatedComponent))
	{
		CapsuleComponent->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);
	}
	else if (USphereComponent* SphereComponent = Cast<USphereComponent>(MovingComps.UpdatedComponent))
	{
		PawnRadius = SphereComponent->GetScaledSphereRadius();
		PawnHalfHeight = SphereComponent->GetScaledSphereRadius();
	}
	else
	{
		// Default to a reasonable size if no capsule or sphere component is found
		PawnRadius = 34.f; // Default radius for a humanoid character
		PawnHalfHeight = 88.f; // Default half-height for a humanoid character
	}

	// Perform the line trace(s)
	if (VaultSweepDistance > 0.f && VaultingSamples > 0)
	{
		const FVector ForwardVector = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		const float SampleHeightOffset = ( Location.Z - PawnHalfHeight ) + MinVaultHeight;

		// Perform vaulting samples
		for (int32 SampleIndex = 0; SampleIndex < VaultingSamples; SampleIndex++)
		{
			const float SampleHeight = ( (MaxVaultHeight - MinVaultHeight) / (float)VaultingSamples ) * SampleIndex;
			const FVector SampleStart = FVector(Location.X, Location.Y, SampleHeightOffset + SampleHeight);
			const FVector SampleEnd = SampleStart + (ForwardVector * VaultSweepDistance);

			FHitResult Hit(1.f);
			const bool bBlockingHit = MovingComps.UpdatedComponent->GetWorld()
				->LineTraceSingleByChannel(Hit, SampleStart, SampleEnd, CollisionChannel, QueryParams, ResponseParams);

			if (bBlockingHit && Hit.Time > 0.f)
			{
				OutVaultingResult.bBlockingHit = true;
				if (IsVaultingPathValid(Hit, UpDirection, VaultingSlopeCosineRange))
				{
					OutVaultingResult.SetFromLineTrace(Hit, Hit.Distance, true);
					break;
				}
			}
		}
	}

	// No hits were acceptable
	OutVaultingResult.bValidVaultingPath = false;
}

bool UVaultingQueryUtils::IsVaultingPathValid(
	const FHitResult& Hit,
	const FVector& UpDirection,
	const FFloatRange& VaultingSlopeCosineRange)
{
	if (!Hit.IsValidBlockingHit())
	{
		// No hit, or starting in penetration
		return false;
	}

	return VaultingSlopeCosineRange.Contains(Hit.ImpactNormal.Dot(UpDirection));
}
