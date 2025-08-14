// Author: Tom Werner (MajorT), 2025

#pragma once

#include "MovementModifier.h"

#include "BotaniStanceModifier.generated.h"

#define MY_API BOTANIMOVER_API

class UCommonMoverComponent;

UENUM(BlueprintType)
enum class EBotaniStanceMode : uint8
{
	/** Invalid default stance. */
	Invalid = 0,

	/** Actor goes into crouching, decreasing their height and collision capsule size. */
	Crouch = 1,

	/** The Actor is fat, being slow and clumsy. */
	Fat = 2,
};

/**
 * Stance Movement Modifiers.
 * - Applies settings to the actor to make them go into different stances like crouching, etc.
 *
 * Assuming we are using a capsule component for the collision.
 */
USTRUCT(BlueprintType)
struct FBotaniStanceModifier : public FMovementModifierBase
{
	GENERATED_BODY()

public:
	MY_API FBotaniStanceModifier();
	virtual ~FBotaniStanceModifier() override {}

	//~ Begin FMovementModifierBase Interface
	MY_API virtual bool HasGameplayTag(FGameplayTag TagToFind, bool bExactMatch) const override;
	MY_API virtual void OnStart(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep, const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState) override;
	MY_API virtual void OnEnd(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep, const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState) override;
	MY_API virtual void OnPreMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep) override;
	MY_API virtual void OnPostMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep, const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState) override;
	MY_API FMovementModifierBase* Clone() const override;
	MY_API virtual void NetSerialize(FArchive& Ar) override;
	MY_API virtual UScriptStruct* GetScriptStruct() const override;
	MY_API virtual FString ToSimpleString() const override;
	MY_API virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	//~ End FMovementModifierBase Interface

	/** Checks if the mover component's owner can expand their capsule size in the current location. (might be blocked by the environment.) */
	MY_API virtual bool CanExpand(const UMoverComponent* MoverComp) const;

	/** Returns true if the expanding should be from the base of the capsule or its center. */
	MY_API virtual bool ShouldExpandingMaintainBase(const UCommonMoverComponent* MoverComp) const;

public:
	/** The current stance mode that this modifier is applying. */
	UPROPERTY(Transient)
	EBotaniStanceMode ActiveStance;

protected:
	/** Modifies the updated component's capsule size to match the stance. */
	MY_API virtual void AdjustCapsule(UMoverComponent* MoverComp, float OldHalfHeight, float NewHalfHeight, float NewEyeHeight);

	/** Applies any movement settings like acceleration or max speed changes based on the stance. */
	MY_API virtual void ApplyMovementSettings(UMoverComponent* MoverComp);

	/** Reverts any movement settings applied by this modifier. */
	MY_API virtual void RevertMovementSettings(UMoverComponent* MoverComp);
};

template<>
struct TStructOpsTypeTraits<FBotaniStanceModifier> : public TStructOpsTypeTraitsBase2<FBotaniStanceModifier>
{
	enum
	{
		WithCopy = true
	};
};

#undef MY_API
