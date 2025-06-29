// Copyright © 2025 Playton. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonMovementMode.h"
#include "DefaultMovementSet/Modes/FallingMode.h"
#include "MoveLibrary/BasedMovementUtils.h"
#include "BotaniMM_Falling.generated.h"

/** Specialized Falling Mode */
UCLASS(DisplayName="Botani: Falling Mode")
class BOTANIMOVER_API UBotaniMM_Falling : public UCommonMovementMode
{
	GENERATED_BODY()

public:
	UBotaniMM_Falling(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UCommonMovementMode Interface

	/** Clears blackboard fields on deactivation */
	virtual void Deactivate() override;

	/** Generates the movement data that will be consumed by the simulation tick */
	virtual void GenerateMove_Implementation(const FMoverTickStartData& StartState, const FMoverTimeStep& TimeStep, FProposedMove& OutProposedMove) const override;

protected:

	/** Gets additional falling data */
	virtual bool PrepareSimulationData(const FSimulationTickParams& Params) override;

	/** Handles most of the actual movement, including collision recovery  */
	virtual void ApplyMovement(FMoverTickEndData& OutputState) override;

	/** Handles any additional behaviors after the updated component's final position and velocity have been computed */
	virtual void PostMove(FMoverTickEndData& OutputState) override;

	/** Captures the final movement values and sends it to the Output Sync State */
	void CaptureFinalState(const FFloorCheckResult& FloorResult, float DeltaSecondsUsed, FMoverTickEndData& TickEndData, FMovementRecord& Record);

	/**
	 * Called at the end of the tick in falling mode. Handles checking any landings that should occur and switching to specific modes
	 * (i.e. landing on a walkable surface would switch to the walking movement mode) 
	 */
	UFUNCTION(BlueprintCallable, Category = Mover)
	virtual void ProcessLanded(const FFloorCheckResult& FloorResult, FVector& Velocity, FRelativeBaseInfo& BaseInfo, FMoverTickEndData& TickEndData) const;

	
	//~ End UCommonMovementMode Interface
	
public:
	/**
	 * If true, actor will land and lose all speed in the vertical direction upon landing. If false, actor's vertical speed will be redirected based on the surface normal it hit.
	 * Note: Actor's horizontal speed will not be affected if true. If false, horizontal speed may be increased on landing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Landing)
	bool bCancelVerticalSpeedOnLanding;

protected:
	/**
	 * When falling, amount of movement control available to the actor.
	 * 0 = no control, 1 = full control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta = (ClampMin = "0", ClampMax = "1.0"))
	float AirControlPercentage = 0.4f;
	
	/**
 	 * Deceleration to apply to air movement when falling slower than terminal velocity.
 	 * Note: This is NOT applied to vertical velocity, only movement plane velocity
 	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta=(ClampMin="0", ForceUnits = "cm/s^2"))
	float FallingDeceleration = 200.0f;

	/**
     * Deceleration to apply to air movement when falling faster than terminal velocity
	 * Note: This is NOT applied to vertical velocity, only movement plane velocity
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta=(ClampMin="0", ForceUnits = "cm/s^2"))
    float OverTerminalSpeedFallingDeceleration = 800.0f;
	
	/**
	 * If the actor's movement plane velocity is greater than this speed falling will start applying OverTerminalSpeedFallingDeceleration instead of FallingDeceleration
	 * The expected behavior is to set OverTerminalSpeedFallingDeceleration higher than FallingDeceleration so the actor will slow down faster
	 * when going over TerminalMovementPlaneSpeed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta=(ClampMin="0", ForceUnits = "cm/s"))
	float TerminalMovementPlaneSpeed = 1500.0f;

	/** When exceeding maximum vertical speed, should it be enforced via a hard clamp? If false, VerticalFallingDeceleration will be used for a smoother transition to the terminal speed limit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling)
	bool bShouldClampTerminalVerticalSpeed = true;

	/** Deceleration to apply to vertical velocity when it's greater than TerminalVerticalSpeed. Only used if bShouldClampTerminalVerticalSpeed is false. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta=(EditCondition="!bShouldClampTerminalVerticalSpeed", ClampMin="0", ForceUnits = "cm/s^2"))
	float VerticalFallingDeceleration = 4000.0f;
	
	/**
	 * If the actor's vertical s[eed is greater than speed VerticalFallingDeceleration will be applied to vertical velocity
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Falling, meta=(ClampMin="0", ForceUnits = "cm/s"))
	float TerminalVerticalSpeed = 2000.0f;


	/** Gameplay Event to send to the actor when we landed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Events)
	FGameplayTag LandingEventTag;

protected:
	/** Effective Velocity calculated this frame */
	FVector EffectiveVelocity;
};
