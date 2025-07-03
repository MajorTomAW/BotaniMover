// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MovementMode.h"
#include "MoveLibrary/WallRunningMovementUtils.h"
#include "UObject/Object.h"
#include "BotaniWallRunMovementSettings.generated.h"

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
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", ForceUnits = "cm/s", DisplayName = "Wall Run Max Speed"))
	float WallRun_MaxSpeed;

	/** The maximum angle the character can have relative to the wall before falling off. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", Units = "deg", ScriptName = "WallRunPullAwayAngle"))
	float WallRun_PullAwayAngle;

	/** The amount of force that pulls the character towards the wall. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", Units = "cm/s^2", ScriptName = "WallRunAttractionForce"))
	float WallRun_AttractionForce;

	/** The braking deceleration applied to the character when wall running. (will slow down the character) */
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", Units = "cm/s^2"))
	float WallRun_BrakingDeceleration;

	/** Friction factor applied to the ground friction while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", Units = "x", DisplayName = "Wall Run Friction Factor"))
	float WallRun_SurfaceFrictionFactor;

	/** The minimum amount of time that has to pass until we can wall run again. */
	UPROPERTY(EditAnywhere, Category="General", meta = (ClampMin = "0.0", Units = "s"))
	float WallRun_MinTimeBetweenRuns;

	/** Runtime curve used to determine the gravity scale over velocity while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (DisplayName = "Wall Run [Gravity x Velocity] Scale Curve", ScriptName = "WallRunGravityVelScaleCurve"))
	FRuntimeFloatCurve WallRun_GravityVelScaleCurve;

	/** Runtime curve used to determine the gravity scale over time while wall running. */
	UPROPERTY(EditAnywhere, Category="General", meta = (DisplayName = "Wall Run [Gravity x Time] Scale Curve"))
	FRuntimeFloatCurve WallRun_GravityTimeScaleCurve;

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
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ClampMin = "0.0", ForceUnits = "cm/s", EditCondition = "!bAlwaysStayOnWall"))
	float WallRun_MinRequiredSpeed;

	/** The minimum height of the character must be above the ground to start the wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ClampMin = "0.0", ForceUnits = "cm", DisplayName = "Min Required Wall Height"))
	float WallRun_MinRequiredHeight;

	/** The minimum angle of the wall required starting wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ClampMin = "0.0", Units = "deg"))
	float WallRun_MinRequiredAngle;

	/** The maximum vertical speed the character can have while wall running. */
	UPROPERTY(EditAnywhere, Category="Evaluation", meta = (ClampMin = "0.0", ForceUnits = "cm/s"))
	float WallRun_MaxVerticalSpeed;


	/** Gameplay tag that blocks wall jumping when present on the character. */
	UPROPERTY(EditAnywhere, Category="Jumping")
	FGameplayTagContainer WallJumpBlockedTags;

	/** Whether the character is allowed to jump off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping")
	uint32 bAllowWallJump:1;

	/** Force used to apply when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (ClampMin = "0.0", ForceUnits = "cm/s^2", EditCondition = "bAllowWallJump", ScriptName = "WallJumpForce"))
	float WallJump_Force;

	/** Force used to apply when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (ClampMin = "0.0", EditCondition = "bAllowWallJump", ScriptName = "WallJumpArcadeForce"))
	FVector WallJump_ArcadeForce;

	/** Whether to override the current z velocity when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition = "bAllowWallJump", DisplayName = "Wall Jump Overrides Z Velocity"))
	uint32 bOverrideWallJumpZVelocity:1;

	/** Whether to override the current xy velocity when jumping off a wall. */
	UPROPERTY(EditAnywhere, Category="Jumping", meta = (EditCondition = "bAllowWallJump", DisplayName = "Wall Jump Overrides XY Velocity"))
	uint32 bOverrideWallJumpXYVelocity:1;

	/** Whether to draw the debug lines in the editor */ 
	UPROPERTY(EditAnywhere, Category = "General", AdvancedDisplay)
	uint32 bDrawWallRunDebug:1;

	/** Time WHEN the character started wall running. */
	UPROPERTY(BlueprintReadOnly, Category = "General", AdvancedDisplay)
	float WallRunTime;
};
