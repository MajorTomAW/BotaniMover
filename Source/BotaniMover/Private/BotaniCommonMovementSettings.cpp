// Author: Tom Werner (MajorT), 2025


#include "BotaniCommonMovementSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniCommonMovementSettings)

UBotaniCommonMovementSettings::UBotaniCommonMovementSettings()
	: bShouldRemainUpright(true)
	, bIgnoreBaseRotation(false)
	, MaxSpeed(800.f)
	, MaxStepHeight(44.f)
	, Acceleration(4000.f)
	, Deceleration(4000.f)
	, bUseAccelerationForVelocityIntent(true)
	, TurningRate(720.f)
	, TurningBoost(8.f)
	, GroundFriction(8.f)
	, bUseSeparateBrakingFriction(false)
	, BrakingFriction(8.f)
	, BrakingFrictionFactor(2.f)
	, MaxWalkSlopeAngleCosine(0.71f) // 45 degrees
	, FloorSweepDistance(40.f)
	, SlopeBoostMultiplier(1.f)
	, MaxSprintSpeed(1000.f)
	, SprintAcceleration(2000.f)
	, SprintDeceleration(200.f)
	, SprintTurningRate(360.f)
	, SprintTurningBoost(3.f)
	, AirControlPct(.4f)
	, FallingDeceleration(200.f)
	, OverTerminalSpeedFallingDeceleration(800.f)
	, TerminalMovementPlaneSpeed(1500.f)
	, bShouldClampTerminalVerticalSpeed(true)
	, VerticalFallingDeceleration(4000.f)
	, TerminalVerticalSpeed(2000.f)
	, MinTimeBetweenJumps(0.1f)
	, CoyoteTime(0.3f)
	, bJumpRequiresGround(true)
	, JumpVerticalImpulse(800.f)
	, JumpHoldTime(0.3f)
	, ExtraJumpVerticalImpulse(150.f)
	, JumpAirControlPct(0.2f)
	, bTruncateOnJumpRelease(true)
	, bJumpOverridesMovementPlaneVelocity(false)
	, bJumpOverridesVerticalVelocity(true)
	, bJumpAddsFloorVelocity(true)
	, bJumpKeepsPreviousVelocity(true)
	, bJumpKeepsPreviousVerticalVelocity(true)
{
}

#if WITH_EDITOR
void UBotaniCommonMovementSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, JumpPreset))
	{
		switch (JumpPreset)
		{
		case EBotaniEditorOnlyJumpPreset::Normal:
			{
				bTruncateOnJumpRelease = true;
				bJumpOverridesMovementPlaneVelocity = true;
				bJumpOverridesVerticalVelocity = false;
				bJumpAddsFloorVelocity = true;
				bJumpKeepsPreviousVelocity = false;
				bJumpKeepsPreviousVerticalVelocity = true;
				break;
			}

		case EBotaniEditorOnlyJumpPreset::BunnyHop:
			{
				bTruncateOnJumpRelease = true;
				bJumpOverridesMovementPlaneVelocity = false;
				bJumpOverridesVerticalVelocity = false;
				bJumpAddsFloorVelocity = true;
				bJumpKeepsPreviousVelocity = true;
				bJumpKeepsPreviousVerticalVelocity = true;
				break;
			}
		}
	}
}
#endif
