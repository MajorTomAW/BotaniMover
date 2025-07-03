// Copyright © 2025 Playton. All Rights Reserved.


#include "BotaniWallRunMovementSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniWallRunMovementSettings)

UBotaniWallRunMovementSettings::UBotaniWallRunMovementSettings()
{
	bAlwaysStayOnWall = 1;
	WallRunSide = Wall_Error;
	WallRun_MinRequiredSpeed = 500.f;
	WallRun_MinRequiredHeight = 100.f;
	WallRun_MinRequiredAngle = 64.f;
	WallRun_MaxVerticalSpeed = 400.f;
	WallRun_MaxSpeed = 800.f;
	WallRun_PullAwayAngle = 72.f;
	WallRun_AttractionForce = 120.f;
	WallRun_BrakingDeceleration = 800.f;
	WallRun_SurfaceFrictionFactor = 0.02f;
	bDrawWallRunDebug = 1;
	bResetTimerOnlyOnLand = 1;
	WallRunTime = 0.f;
	WallRun_MinTimeBetweenRuns = 0.5f;

	bAllowWallJump = true;
	bOverrideWallJumpZVelocity = 1;
	bOverrideWallJumpXYVelocity = 0;
	WallJump_Force = 420.f;
	WallJump_ArcadeForce = FVector::ZeroVector;
}
