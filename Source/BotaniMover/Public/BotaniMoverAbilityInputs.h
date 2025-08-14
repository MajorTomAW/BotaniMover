// Author: Tom Werner (MajorT), 2025

#pragma once

#include "MoverTypes.h"

#include "BotaniMoverAbilityInputs.generated.h"

/** Input data for user activated abilities. */
USTRUCT(BlueprintType)
struct FBotaniMoverAbilityInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

public:
	FBotaniMoverAbilityInputs()
		: bSprintPressedThisFrame(false)
		, bIsSprintPressed(false)
		, bJumpPressedThisFrame(false)
		, bIsJumpPressed(false)
		, bDashPressedThisFrame(false)
		, bVaultPressedThisFrame(false)
		, bCrouchPressedThisFrame(false)
	{
	}

	virtual ~FBotaniMoverAbilityInputs() override {};

public:
	/** Was the Sprint input pressed this frame? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bSprintPressedThisFrame:1;

	/** Is the Sprint input currently being pressed aka held down? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bIsSprintPressed:1;

	/** Was jumping input pressed this frame? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bJumpPressedThisFrame:1;

	/** Is the Jump input currently being pressed aka held down? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bIsJumpPressed:1;

	/** Was the Dash input pressed this frame? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bDashPressedThisFrame:1;

	/** Was the Vault input pressed this frame? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bVaultPressedThisFrame:1;

	/** Was the Crouch input pressed this frame? */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	uint8 bCrouchPressedThisFrame:1;

public:
	//~ Begin FMoverDataStructBase Interface
	virtual FMoverDataStructBase* Clone() const override
	{
		FBotaniMoverAbilityInputs* CopyPtr = new FBotaniMoverAbilityInputs(*this);
		return CopyPtr;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);

		//@TODO: In case anything breaks, here may lay the issue, who knows 🤯🤯🤯

		// Serialize the digital input flags
		uint8 RepBits = 0;
		if (Ar.IsSaving())
		{
			if (bSprintPressedThisFrame) { RepBits |= 1 << 0; }
			if (bIsSprintPressed) { RepBits |= 1 << 1; }
			if (bDashPressedThisFrame) { RepBits |= 1 << 2; }
			if (bVaultPressedThisFrame) { RepBits |= 1 << 3; }
			if (bCrouchPressedThisFrame) { RepBits |= 1 << 4; }
		}

		// Serialize the bits (handles loading and saving)
		Ar.SerializeBits(&RepBits, 5);

		if (RepBits & (1 << 0)) { bSprintPressedThisFrame = true; } else { bSprintPressedThisFrame = false; }
		if (RepBits & (1 << 1)) { bIsSprintPressed = true; } else { bIsSprintPressed = false; }
		if (RepBits & (1 << 2)) { bDashPressedThisFrame = true; } else { bDashPressedThisFrame = false; }
		if (RepBits & (1 << 3)) { bVaultPressedThisFrame = true; } else { bVaultPressedThisFrame = false; }
		if (RepBits & (1 << 4)) { bCrouchPressedThisFrame = true; } else { bCrouchPressedThisFrame = false; }

		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Super::ToString(Out);
		Out.Appendf("SprintPressedThisFrame: %d, IsSprintPressed: %d", bSprintPressedThisFrame, bIsSprintPressed);
		Out.Appendf("DashPressedThisFrame: %d", bDashPressedThisFrame);
		Out.Appendf("VaultPressedThisFrame: %d", bVaultPressedThisFrame);
		Out.Appendf("CrouchPressedThisFrame: %d", bCrouchPressedThisFrame);
	}

	virtual bool ShouldReconcile(const FMoverDataStructBase& AuthorityState) const override
	{
		const FBotaniMoverAbilityInputs& TypedAuthority = static_cast<const FBotaniMoverAbilityInputs&>(AuthorityState);

		// Reconcile if any of the input flags differ
		return (bSprintPressedThisFrame != TypedAuthority.bSprintPressedThisFrame)
			|| (bIsSprintPressed != TypedAuthority.bIsSprintPressed)
			|| (bDashPressedThisFrame != TypedAuthority.bDashPressedThisFrame)
			|| (bVaultPressedThisFrame != TypedAuthority.bVaultPressedThisFrame)
			|| (bCrouchPressedThisFrame != TypedAuthority.bCrouchPressedThisFrame);
	}

	virtual void Interpolate(const FMoverDataStructBase& From, const FMoverDataStructBase& To, float Pct) override
	{
		// Since we're just copying bool properties,
		// we simply copy them from From if LerpFactor is less than 0.5, otherwise from To
		const FBotaniMoverAbilityInputs& SourceAbilityInputs =
			static_cast<const FBotaniMoverAbilityInputs&>((Pct < 0.5f) ? From : To);

		bIsSprintPressed = SourceAbilityInputs.bIsSprintPressed;
		bSprintPressedThisFrame = SourceAbilityInputs.bSprintPressedThisFrame;
		bDashPressedThisFrame = SourceAbilityInputs.bDashPressedThisFrame;
		bVaultPressedThisFrame = SourceAbilityInputs.bVaultPressedThisFrame;
		bCrouchPressedThisFrame = SourceAbilityInputs.bCrouchPressedThisFrame;
	}

	virtual void Merge(const FMoverDataStructBase& From) override
	{
		const FBotaniMoverAbilityInputs& FromInputs =
			static_cast<const FBotaniMoverAbilityInputs&>(From);

		bIsSprintPressed |= FromInputs.bIsSprintPressed;
		bSprintPressedThisFrame |= FromInputs.bSprintPressedThisFrame;
		bDashPressedThisFrame |= FromInputs.bDashPressedThisFrame;
		bVaultPressedThisFrame |= FromInputs.bVaultPressedThisFrame;
		bCrouchPressedThisFrame |= FromInputs.bCrouchPressedThisFrame;
	}
	//~ End FMoverDataStructBase Interface
};
