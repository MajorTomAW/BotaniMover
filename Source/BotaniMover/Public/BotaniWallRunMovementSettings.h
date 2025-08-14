// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "ScalableFloat.h"
#include "MoveLibrary/WallRunningMovementUtils.h"
#include "UObject/Object.h"

#include "BotaniWallRunMovementSettings.generated.h"

#define MY_API BOTANIMOVER_API

#define GetBotaniWallRunFloatProp(FloatPropertyName) \
	BotaniWallRunSettings->FloatPropertyName.GetValue()

/** WallRunMovementSettings: collection of settings that are used among any wall-running related movement modes and transitions. */
UCLASS(MinimalAPI, BlueprintType)
class UBotaniWallRunMovementSettings
	: public UObject
	, public IMovementSettingsInterface
{
	GENERATED_BODY()

public:
	UBotaniWallRunMovementSettings();

	//~ Begin IMovementSettingsInterface
	virtual FString GetDisplayName() const override { return GetName(); }
	//~ End IMovementSettingsInterface

public:
	/** The maximum speed the character can move while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunMaxSpeed", DisplayName="Wall Run Max Speed (cm/s)"))
	FScalableFloat WallRun_MaxSpeed;

	/** Linear rate of acceleration while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunAcceleration", DisplayName="Wall Run Acceleration (cm/s²)"))
	FScalableFloat WallRun_Acceleration;

	/** Liner rate of deceleration while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunDeceleration", DisplayName="Wall Run Deceleration (cm/s²)"))
	FScalableFloat WallRun_Deceleration;

	/** The maximum angle the character can have relative to the wall before falling off. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunPullAwayAngle", DisplayName="Wall Run Pull Away Angle (deg)"))
	FScalableFloat WallRun_PullAwayAngle;

	/** The amount of force that pulls the character towards the wall. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunAttractionForce", DisplayName="Wall Run Attraction Force Magnitude (cm/s²)"))
	FScalableFloat WallRun_AttractionForceMagnitude;

	/** The braking deceleration applied to the character when wall running. (will slow down the character) */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunBrakingDeceleration", DisplayName="Wall Run Braking Deceleration (cm/s²)"))
	FScalableFloat WallRun_BrakingDeceleration;

	/** Friction factor applied to the ground friction while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunSurfaceFrictionFactor", DisplayName="Wall Run Friction Factor"))
	FScalableFloat WallRun_SurfaceFrictionFactor;

	/** The minimum amount of time that has to pass until we can wall run again. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunMinTimeBetweenRuns", DisplayName="Wall Run Min Time Between Runs (s)"))
	FScalableFloat WallRun_MinTimeBetweenRuns;

	/** Optional maximum time the character can wall run. If not set, the character can wall run indefinitely. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunMaxTime", DisplayName="Wall Run Max Time (s)"))
	TOptional<FScalableFloat> WallRun_MaxTime;

	/** Runtime curve used to determine the gravity scale over velocity while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (DisplayName = "Wall Run [Gravity x Velocity] Scale Curve", ScriptName = "WallRunGravityVelScaleCurve"))
	FRuntimeFloatCurve WallRun_GravityVelScaleCurve;

	/** Runtime curve used to determine the gravity scale over time while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (DisplayName = "Wall Run [Gravity x Time] Scale Curve"))
	FRuntimeFloatCurve WallRun_GravityTimeScaleCurve;

	/** Overall gravity scale applied to the character while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunGravityScale", DisplayName="Wall Run Gravity Scale"))
	FScalableFloat WallRun_GravityScale;

	/** Overall gravity scale applied to the character while wall running if moving upwards on a wall. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ScriptName="WallRunUpwardsGravityScale", DisplayName="Wall Run Upwards Gravity Scale"))
	FScalableFloat WallRun_UpwardsGravityScale;

	UPROPERTY(EditAnywhere, Category="General")
	uint32 bResetTimerOnlyOnLand:1;

	/** Enum telling which side of the wall the character is running on. */
	UPROPERTY(BlueprintReadOnly, Category="Evaluation")
	TEnumAsByte<EBotaniWallRunSide> WallRunSide;

	/** Delta used for vector's tail in wall trace. */
	UPROPERTY(EditAnywhere, Category="Evaluation")
	float WallTraceVectorsTailDelta = 30.f;

	/** Delta used for vector's head in wall trace. */
	UPROPERTY(EditAnywhere, Category="Evaluation")
	float WallTraceVectorsHeadDelta = 90.f;

	/** Tags required on the sync state to allow the wall running transition */
	UPROPERTY(EditAnywhere, Category="Evaluation")
	FGameplayTagContainer WallRunningRequiredTags;

	/** Gameplay tag that blocks wall running when present on the character. */
	UPROPERTY(EditAnywhere, Category="Evaluation")
	FGameplayTagContainer WallRunningBlockedTags;

	/** Whether the character should always stay on the wall. */
	UPROPERTY(EditAnywhere, Category="Evaluation")
	uint32 bAlwaysStayOnWall:1;

	/** The minimum speed the character must have to start wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (EditCondition="!bAlwaysStayOnWall", ScriptName="WallRunMinRequiredSpeed", DisplayName="Wall Run Min Required Speed (cm/s)"))
	FScalableFloat WallRun_MinRequiredSpeed;

	/** The static minimum height of the character must be above the ground to start the wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ScriptName="WallRunMinRequiredHeight", DisplayName="Min Required Static Wall Height (cm)"))
	FScalableFloat WallRun_MinRequiredStaticHeight;

	/** The dynamic minimum height fo the character must be above the ground to start the wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ScriptName="WallRunMinRequiredDynamicHeight", DisplayName="Min Required Dynamic Wall Height (cm)"))
	FScalableFloat WallRun_MinRequiredDynamicHeight;

	/** The minimum angle of the wall required starting wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ScriptName="WallRunMinRequiredAngle", DisplayName="Min Required Wall Angle (deg)"))
	FScalableFloat WallRun_MinRequiredAngle;

	/** The maximum vertical speed the character can have while wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ScriptName="WallRunMaxVerticalSpeed", DisplayName="Wall Run Max Vertical Speed (cm/s)"))
	FScalableFloat WallRun_MaxVerticalSpeed;


	/** Whether the character is allowed to jump off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping")
	uint32 bAllowWallJump : 1;

	/** Force used to apply when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition="bAllowWallJump", ScriptName="WallJumpForce", DisplayName="Wall Jump Force Magnitude (cm/s²)"))
	FScalableFloat WallJump_ForceMagnitude;

	/** Force used to apply when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition="bAllowWallJump", ScriptName="WallJumpArcadeForce", DisplayName="Wall Jump Arcade Force (cm/s²)"))
	FVector WallJump_ArcadeForce;

	/** If true, any floor velocity will be added to the wall jump velocity. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition = "bAllowWallJump", DisplayName = "Wall Jump Adds Floor Velocity"))
	uint32 bWallJumpAddsFloorVelocity : 1;

	/** Whether to override the current z velocity when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition = "bAllowWallJump"))
	uint32 bWallJumpKeepsPreviousVelocity : 1;

	/** Whether to override the current xy velocity when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition = "bAllowWallJump && bWallJumpKeepsPreviousVelocity"))
	uint32 bWallJumpKeepsPreviousVerticalVelocity : 1;

	/** Whether to draw the debug lines in the editor */
	UPROPERTY(EditAnywhere, Category = "General", AdvancedDisplay)
	uint32 bDrawWallRunDebug : 1;

	/** Time WHEN the character started wall running. */
	UPROPERTY(BlueprintReadOnly, Category = "General", AdvancedDisplay)
	float WallRunTime;
};

#undef MY_API
