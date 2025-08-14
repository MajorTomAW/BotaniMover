// Author: Tom Werner (MajorT), 2025

#pragma once

#include "CoreMinimal.h"
#include "ScalableFloat.h"
#include "Components/PawnComponent.h"

#include "BotaniVaultingComponent.generated.h"

struct FVaultingPathCheckResult;
class UMotionWarpingComponent;
class AActor;
class APawn;
class UBaseMovementMode;
struct FMoverInputCmdContext;
struct FMoverTimeStep;
struct FFrame;

#define MY_API BOTANIMOVER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBotaniVaultingEvent, const FVaultingPathCheckResult&, VaultingPath);

/** Pawn component that will scan for vaulting opportunities while the character is in the air. */
UCLASS(BlueprintType, DisplayName="Vaulting Component", MinimalAPI, Abstract)
class UBotaniVaultingComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	MY_API UBotaniVaultingComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~ Begin UActorComponent Interface
	MY_API virtual void BeginPlay() override;
	MY_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	MY_API virtual void InitializeComponent() override;
	//~ End UActorComponent Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
	//~ End UObject Interface
#endif

	/** Returns the mover component that owns this vaulting component. */
	MY_API class UBotaniMoverComponent* GetMoverComponent() const;

	/** Called when vaulting is started. */
	UPROPERTY(BlueprintAssignable)
	FBotaniVaultingEvent OnVaultingStarted;

protected:
	/** Bound to the mover component's pre-simulation tick event. This is where vaulting checks are performed. */
	UFUNCTION()
	MY_API virtual void OnMoverPreSimulationTick(const FMoverTimeStep& TimeStep, const FMoverInputCmdContext& InputCmd);

	/** Must be implemented in blueprints to handle the vaulting montage playback. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayMoverVaultingMontage(UBotaniMoverComponent* MoverComponent, UAnimMontage* Montage, UMotionWarpingComponent* MotionWarpingComponent);

public:
	/** Number of vaulting samples to perform to find vaulting opportunities. Note that higher values may decrease performance. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting")
	uint8 NumVaultingSamples = 3;

	/** The minimum angle of the edge that can be vaulted over. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting", meta=(DisplayName="Min Vaulting Slope Angle (deg)"))
	FScalableFloat MinVaultingSlopeAngle = 60.f;

	/** The maximum angle of the edge that can be vaulted over. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting", meta=(DisplayName="Max Vaulting Slope Angle (deg)"))
	FScalableFloat MaxVaultingSlopeAngle = 90.f;

	/** The trace distance to use when checking for vaulting opportunities. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting", meta=(DisplayName="Vaulting Trace Distance (cm)"))
	FScalableFloat VaultingTraceDistance = 200.f;

	/** The minimum height of the edge that can be vaulted over. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting", meta=(DisplayName="Min Vaulting Height (cm)"))
	FScalableFloat MinVaultingHeight = 50.f;

	/** The maximum height of the edge that can be vaulted over. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting", meta=(DisplayName="Max Vaulting Height (cm)"))
	FScalableFloat MaxVaultingHeight = 160.f;

	/** The motion warping target name to use for vaulting. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting|Motion Warpin")
	FName VaultWarpTargetName = TEXT("VaultWarpTarget");

	/** The montage to play when vaulting. */
	UPROPERTY(EditDefaultsOnly, Category = "Vaulting|Motion Warpin")
	TObjectPtr<UAnimMontage> VaultingMontage;

private:
	/** Transient pointer to the mover component that owns this vaulting component. */
	UPROPERTY(Transient)
	TWeakObjectPtr<class UBotaniMoverComponent> WeakMoverComp;
};

#undef MY_API
