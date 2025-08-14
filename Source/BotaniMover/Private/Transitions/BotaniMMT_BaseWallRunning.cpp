// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_BaseWallRunning.h"

#include "BotaniWallRunMovementSettings.h"
#include "MoverComponent.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_BaseWallRunning)

UBotaniMMT_BaseWallRunning::UBotaniMMT_BaseWallRunning(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniWallRunMovementSettings::StaticClass());
}

void UBotaniMMT_BaseWallRunning::OnRegistered()
{
	Super::OnRegistered();

	// Get the settings
	BotaniWallRunSettings = GetMoverComponent()->FindSharedSettings<UBotaniWallRunMovementSettings>();
	ensureMsgf(BotaniWallRunSettings, TEXT("Failed to find instance of UBotaniWallRunMovementSettings on %s. Movement won't function properly!"),
		*GetPathNameSafe(this));
}

void UBotaniMMT_BaseWallRunning::OnUnregistered()
{
	// Release the settings reference
	BotaniWallRunSettings = nullptr;

	Super::OnUnregistered();
}

bool UBotaniMMT_BaseWallRunning::CanStartWallRunning(
	const FSimulationTickParams& Params,
	FHitResult& OutHitResult) const
{
	// Start by resetting whatever is in the hit result
	OutHitResult.Reset(1.f, false);

	// Trace for walls to run on
	FHitResult WallHit;
	const bool bHitWall = UWallRunningMovementUtils::PerformWallTrace(
		Params.MovingComps,
		WallHit,
		BotaniWallRunSettings->WallTraceVectorsHeadDelta,
		BotaniWallRunSettings->WallTraceVectorsTailDelta);

	if (bHitWall)
	{
		OutHitResult = WallHit;
	}

	return bHitWall;
}
