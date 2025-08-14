// Copyright © 2025 Playton. All Rights Reserved.


#include "BotaniWallRunMovementSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniWallRunMovementSettings)

UBotaniWallRunMovementSettings::UBotaniWallRunMovementSettings()
	: WallRun_MaxSpeed(800.f)
	, WallRun_Acceleration(2048.f)
	, WallRun_Deceleration(1024.f)
	, WallRun_PullAwayAngle(72.f)
	, WallRun_AttractionForceMagnitude(120.f)
	, WallRun_BrakingDeceleration(800.f)
	, WallRun_SurfaceFrictionFactor(0.02f)
	, WallRun_MinTimeBetweenRuns(0.5f)
	, WallRun_GravityScale(1.f)
	, WallRun_UpwardsGravityScale(4.f)
	, bResetTimerOnlyOnLand(true)
	, WallRunSide(Wall_Error)
	, bAlwaysStayOnWall(true)
	, WallRun_MinRequiredSpeed(500.f)
	, WallRun_MinRequiredStaticHeight(5.f)
	, WallRun_MinRequiredDynamicHeight(50.f)
	, WallRun_MinRequiredAngle(64.f)
	, WallRun_MaxVerticalSpeed(400.f)
	, bAllowWallJump(true)
	, WallJump_ForceMagnitude(420.f)
	, WallJump_ArcadeForce(FVector::ZeroVector)
	, bWallJumpAddsFloorVelocity(true)
	, bWallJumpKeepsPreviousVelocity(true)
	, bWallJumpKeepsPreviousVerticalVelocity(false)
	, bDrawWallRunDebug(false)
	, WallRunTime(0.f)
{
}
