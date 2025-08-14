// Author: Tom Werner (MajorT), 2025

#pragma once

#include "UObject/Object.h"
#include "MovementMode.h"
#include "ScalableFloat.h"

#include "BotaniCommonMovementSettings.generated.h"

#define MY_API BOTANIMOVER_API

#if WITH_EDITORONLY_DATA
UENUM()
enum class EBotaniEditorOnlyJumpPreset : uint8
{
	Normal,
	BunnyHop,
};
#endif

#define GetBotaniMoverFloatProp(FloatPropertyName) \
	BotaniMovementSettings->FloatPropertyName.GetValue()

/** Common movement settings backed by scalable floats. */
UCLASS(MinimalAPI, BlueprintType)
class UBotaniCommonMovementSettings
	: public UObject
	, public IMovementSettingsInterface
{
	GENERATED_BODY()

public:
	MY_API UBotaniCommonMovementSettings();

	//~ Begin IMovementSettingsInterface
	virtual FString GetDisplayName() const override { return GetName(); }
	//~ End IMovementSettingsInterface

	//~ Begin UObject Interface
#if WITH_EDITOR
	MY_API virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

public:
	/** If true, the actor will remain upright with gravity despite any rotation applied to the actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
	uint8 bShouldRemainUpright : 1;

	/**
	 * Whether the actor ignores changes in rotation of the base, it is standing on when using based movement.
	 * If true, the actor maintains its current world rotation regardless of the base's rotation.
	 * If false, the actor rotates with the moving base.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
	uint8 bIgnoreBaseRotation : 1;

	/** Maximum speed in the movement plane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(DisplayName="Max Speed (cm/s)"))
	FScalableFloat MaxSpeed;

	/** Mover actors will be able to step up onto or over obstacles shorter than this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(DisplayName="Max Step Height (cm)"))
	FScalableFloat MaxStepHeight;

	/** Default max linear rate of acceleration for controlled input. May be scaled based on the magnitude of input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(DisplayName="Acceleration (cm/s²)"))
	FScalableFloat Acceleration;

	/** Default max linear rate of deceleration when there is no controlled input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(DisplayName="Deceleration (cm/s²)"))
	FScalableFloat Deceleration;

	/**
	 * Should use acceleration for velocity-based movement intent?
	 * If true, acceleration is applied when using velocity input to reach the target velocity.
	 * If false, velocity is set directly, disregarding acceleration.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", AdvancedDisplay)
	uint8 bUseAccelerationForVelocityIntent : 1;

	/** Maximum rate of turning rotation (degrees per second). Negative numbers indicate instant rotation and should cause rotation to snap instantly to desired direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General", meta=(DisplayName="Turning Rate (deg/s)"))
	FScalableFloat TurningRate;

	/** Speeds the velocity direction changes while turning, to reduce sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General")
	FScalableFloat TurningBoost;

	/**
	 * Setting that affects movement control.
	 * Higher values allow faster changes in direction.
	 * This can be used to simulate slippery surfaces such as ice or oil by lowering the value
	 * (possibly based on the material the actor is standing on).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General|Friction")
	FScalableFloat GroundFriction;

	/**
	 * If true, BrakingFriction will be used to slow the character to a stop (when there is no Acceleration).
	 * If false, braking uses the same friction passed to CalcVelocity() (i.e., GroundFriction when walking), multiplied by BrakingFrictionFactor.
	 * This setting applies to all movement modes; if only desired in certain modes, consider toggling it when movement modes change.
	 * @see BrakingFriction
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General|Friction")
	uint8 bUseSeparateBrakingFriction : 1;

	/**
	 * Friction (drag) coefficient applied when braking (whenever Acceleration = 0, or if character is exceeding max speed); actual value used is this multiplied by BrakingFrictionFactor.
	 * When braking, this property allows you to control how much friction is applied when moving across the ground, applying an opposing force that scales with current velocity.
	 * Braking is composed of friction (velocity-dependent drag) and constant deceleration.
	 * This is the current value, used in all movement modes; if this is not desired, override it or bUseSeparateBrakingFriction when movement mode changes.
	 * @note Only used if bUseSeparateBrakingFriction setting is true, otherwise current friction such as GroundFriction is used.
	 * @see bUseSeparateBrakingFriction, BrakingFrictionFactor, GroundFriction, BrakingDecelerationWalking
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General|Friction", meta=(EditCondition=bUseSeparateBrakingFriction))
	FScalableFloat BrakingFriction;

	/**
	 * Factor used to multiply the actual value of friction used when braking.
	 * This applies to any friction value that is currently used, which may depend on bUseSeparateBrakingFriction.
	 * @note This is 2 by default for historical reasons, a value of 1 gives the true drag equation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="General|Friction")
	FScalableFloat BrakingFrictionFactor;







	/** Walkable slope angle, represented as cosine (max slope angle) for performance reasons. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement")
	FScalableFloat MaxWalkSlopeAngleCosine;

	/** Max distance to scan for floor surfaces under a Mover actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement", meta=(ClampMin=0, UIMin=0, ForceUnits=cm))
	float FloorSweepDistance;

	/** Slope boost multiplier for the floor velocity when walking on slopes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement")
	FScalableFloat SlopeBoostMultiplier;

	/** Maximum ground speed while sprinting, in the movement plane. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement|Sprinting", meta=(DisplayName="Max Sprint Speed (cm/s)"))
	FScalableFloat MaxSprintSpeed;

	/** Maximum ground acceleration while sprinting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement|Sprinting", meta=(DisplayName="Sprint Acceleration (cm/s²)"))
	FScalableFloat SprintAcceleration;

	/** Ground deceleration while sprinting when there is no controlled input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement|Sprinting", meta=(DisplayName="Sprint Deceleration (cm/s²)"))
	FScalableFloat SprintDeceleration;

	/** Turning rate while sprinting, in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement|Sprinting", meta=(DisplayName="Sprint Turning Rate (deg/s)"))
	FScalableFloat SprintTurningRate;

	/** Speeds the velocity direction changes while turning during sprinting, to reduce sliding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground Movement|Sprinting")
	FScalableFloat SprintTurningBoost;






	/**
	 * When falling, amount of movement control available to the player.
	 * 0 = no control
	 * 1 = full control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(DisplayName="Air Control (%)"))
	FScalableFloat AirControlPct;

	/**
	  * Deceleration to apply to air movement when falling slower than terminal velocity.
	  * Note: This is NOT applied to vertical velocity, only movement plane velocity
	  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(DisplayName="Falling Deceleration (cm/s²)"))
	FScalableFloat FallingDeceleration;

	/**
	 * Deceleration to apply to air movement when falling faster than terminal velocity
	 * Note: This is NOT applied to vertical velocity, only movement plane velocity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(DisplayName="Over Terminal Speed Falling Deceleration (cm/s²)"))
	FScalableFloat OverTerminalSpeedFallingDeceleration;

	/**
	 * If the actor's movement plane velocity is greater than this speed falling will start applying OverTerminalSpeedFallingDeceleration instead of FallingDeceleration
	 * The expected behavior is to set OverTerminalSpeedFallingDeceleration higher than FallingDeceleration so the actor will slow down faster
	 * when going over TerminalMovementPlaneSpeed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(DisplayName="Terminal Movement Plane Speed (cm/s)"))
	FScalableFloat TerminalMovementPlaneSpeed;

	/** When exceeding maximum vertical speed, should it be enforced via a hard clamp? If false, VerticalFallingDeceleration will be used for a smoother transition to the terminal speed limit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement")
	uint8 bShouldClampTerminalVerticalSpeed : 1;

	/** Deceleration to apply to vertical velocity when it's greater than TerminalVerticalSpeed. Only used if bShouldClampTerminalVerticalSpeed is false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(EditCondition="!bShouldClampTerminalVerticalSpeed", DisplayName="Vertical Falling Deceleration (cm/s²)"))
	FScalableFloat VerticalFallingDeceleration;

	/** If the actor's vertical speed is greater than TerminalVerticalSpeed VerticalFallingDeceleration will be applied to vertical velocity. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement", meta=(DisplayName="Terminal Vertical Speed (cm/s)"))
	FScalableFloat TerminalVerticalSpeed;

	/** Minimum amount of time to elapse between jumps. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Min Time Between Jumps (s)"))
	FScalableFloat MinTimeBetweenJumps;

	/** If greater than zero, the character can jump if less than this falling time has elapsed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Coyote Time (s)"))
	FScalableFloat CoyoteTime;

	/** If true, the character will only jump if it has a valid walkable floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bJumpRequiresGround : 1;

	/** Vertical impulse to provide with the jump as long as the button is pressed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Vertical Impulse (cm/s²)"))
	FScalableFloat JumpVerticalImpulse;

	/** Time to hold the jump impulse for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Hold Time (s)"))
	FScalableFloat JumpHoldTime;

	/** If this tag is present, an extra impulse will be provided. Allows for higher jumps while sprinting, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	FGameplayTag ExtraJumpVerticalImpulseTag;

	/** Extra vertical impulse to add if the extra impulse tag is present */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Extra Vertical Impulse (cm/s²)"))
	FScalableFloat ExtraJumpVerticalImpulse;

	/** Percentage of air control while jump is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Jumping Air Control (%)"))
	FScalableFloat JumpAirControlPct;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	EBotaniEditorOnlyJumpPreset JumpPreset = EBotaniEditorOnlyJumpPreset::Normal;
#endif

	/** If true, the character will stop receiving vertical impulse as soon as the jump button is released */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bTruncateOnJumpRelease : 1;

	/** If true, the character's movement plane velocity will be overridden by the provided computed momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bJumpOverridesMovementPlaneVelocity : 1;

	/** If true, the character's vertical velocity will be overridden by the provided computed momentum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bJumpOverridesVerticalVelocity : 1;

	/** If true, any floor velocity will be added to the overridden velocity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bJumpAddsFloorVelocity : 1;

	/** If true, the character will keep any existing movement plane velocity from before jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping")
	uint8 bJumpKeepsPreviousVelocity : 1;

	/** If true, the character will keep any existing vertical velocity from before jumping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(EditCondition=bJumpKeepsPreviousVelocity))
	uint8 bJumpKeepsPreviousVerticalVelocity : 1;

	/** If a positive value is provided, any carried over velocity will be clamped to this maximum value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Air Movement|Jumping", meta=(DisplayName="Max Previous Velocity (cm/s)"))
	FScalableFloat MaxJumpPreviousVelocity = -1.0f;
};

#undef MY_API
