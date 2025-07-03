// Author: Tom Werner (MajorT), 2025


#include "MoveLibrary/WallRunningMovementUtils.h"

#include "BotaniMoverLogChannels.h"
#include "MoverComponent.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(WallRunningMovementUtils)

FProposedMove UWallRunningMovementUtils::ComputeControlledWallRunMove(const FWallRunMoveParams& InParams)
{
	FProposedMove OutMove;

	return OutMove;
}

bool UWallRunningMovementUtils::PerformWallTrace(
	const FMovingComponentSet& MovingComps,
	FHitResult& OutWallHit,
	float WallTraceVectorsHeadDelta,
	float WallTraceVectorsTailDelta,
	EBotaniWallRunSide WallSide)
{
	if (WallSide == Wall_Error)
	{
		UE_LOG(LogBotaniMover, Error, TEXT("[%hs] WallSide is set to Error on pawn '%s'!"),
			__func__, *GetNameSafe(MovingComps.MoverComponent->GetOwner()));
		return false;
	}

	UMoverComponent const* MoverComponent = MovingComps.MoverComponent.Get();
	const FVector DownDirection = MoverComponent->GetUpDirection() * -1.f;

	// Trace params
	UWorld const* World = MoverComponent->GetWorld();
	check(World);

	FCollisionQueryParams QueryParams = GetIgnoreOwnerQueryParams(MoverComponent);
	FHitResult WallHit;

	// Build up the trace start and end points
	const FVector FwdDir = MoverComponent->GetOwner()->GetActorForwardVector();
	const FVector Location = MoverComponent->GetOwner()->GetActorLocation();
	const FVector FwdDir2D = FVector::VectorPlaneProject(FwdDir, DownDirection);
	const FVector RightDir = (FwdDir2D ^ DownDirection).GetSafeNormal2D();

	auto DoTrace = [&] (const FVector& InTraceStart, const FVector& InTraceEnd)
	{
		auto Result = World->LineTraceSingleByChannel(WallHit, InTraceStart, InTraceEnd, ECC_Camera, QueryParams);

#if ENABLE_DRAW_DEBUG
		DrawDebugLine(World, InTraceStart, InTraceEnd, Result ? FColor::Blue : FColor::Red, false, 0.1f, 0, 1.f);
#endif

		return Result;
	};

	auto DoDoubleTrace = [&] (EBotaniWallRunSide InWallSide)
	{
		ensure(((!!(InWallSide & Wall_Left)) != (!!(InWallSide & Wall_Right))));

		// Expr
		const float Sign = (InWallSide == Wall_Left) ? -1.f : 1.f;
		const FVector SideDelta = (RightDir * WallTraceVectorsHeadDelta) * Sign;
		const FVector ForwardDelta = (FwdDir2D * WallTraceVectorsTailDelta);
		const FVector BackDelta = -ForwardDelta;

		// Compute point-vectors
		const FVector TraceStart = Location;
		const FVector TraceEndFront = (TraceStart + ForwardDelta + SideDelta);
		const FVector TraceEndBack = (TraceStart + BackDelta + SideDelta);

		return (DoTrace(TraceStart, TraceEndFront) ||
			DoTrace(TraceStart, TraceEndBack));
	};

	// Do left or/and right traces
	if (((WallSide & Wall_Left) && (DoDoubleTrace(Wall_Left)) ||
		((WallSide & Wall_Right) && (DoDoubleTrace(Wall_Right)))))
	{
		// Save trace result if a wall has been found
		OutWallHit = WallHit;
		return true;
	}
	
	return false;
}

FCollisionQueryParams UWallRunningMovementUtils::GetIgnoreOwnerQueryParams(
	const UMoverComponent* InMoverComponent)
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(InMoverComponent->GetOwner());

	TArray<AActor*> ChildActors;
	InMoverComponent->GetOwner()->GetAllChildActors(ChildActors);
	QueryParams.AddIgnoredActors(ChildActors);

	return QueryParams;
}

float UWallRunningMovementUtils::GetWallAngle(const FHitResult& WallHit, const FVector& UpDirection)
{
	return FMath::RadiansToDegrees(FMath::Acos(WallHit.Normal | UpDirection));
}

bool UWallRunningMovementUtils::ShouldFallOffWall(
	const FHitResult& WallHit,
	const float& PullAwayAngle,
	const FVector& MoveIntent)
{
	const float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(PullAwayAngle));
	return (WallHit.IsValidBlockingHit()
		&& !MoveIntent.IsNearlyZero()
		&& ((MoveIntent.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle));
}

bool UWallRunningMovementUtils::IsHighEnoughForWallRun(
	const FMovingComponentSet& MovingComps,
	float MinHeightAboveFloor,
	const FVector& UpDirection)
{
	UWorld const* World = MovingComps.MoverComponent->GetWorld();
	check(World);
	
	const FVector Start = MovingComps.UpdatedComponent->GetComponentLocation();
	FBoxSphereBounds Bounds = MovingComps.UpdatedComponent->GetLocalBounds();
	
	const FVector End = Start + (-UpDirection * ( MinHeightAboveFloor + Bounds.SphereRadius));

	FHitResult GroundHit;
	if (World->LineTraceSingleByChannel(GroundHit, Start, End, ECC_Visibility, GetIgnoreOwnerQueryParams(MovingComps.MoverComponent.Get())))
	{
		return false;
	}

	return true;
}
