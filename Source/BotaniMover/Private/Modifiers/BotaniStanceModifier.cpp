// Author: Tom Werner (MajorT), 2025


#include "Modifiers/BotaniStanceModifier.h"

#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverLogChannels.h"
#include "BotaniMoverSettings.h"
#include "BotaniStanceSettings.h"
#include "CommonMoverComponent.h"
#include "MoverComponent.h"
#include "MoverTypes.h"
#include "Components/CapsuleComponent.h"
#include "DefaultMovementSet/InstantMovementEffects/BasicInstantMovementEffects.h"
#include "MoveLibrary/MovementUtils.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniStanceModifier)

FBotaniStanceModifier::FBotaniStanceModifier()
{
	ActiveStance = EBotaniStanceMode::Crouch;
	DurationMs = -1.f; // Infinite duration by default
}

bool FBotaniStanceModifier::HasGameplayTag(FGameplayTag TagToFind, bool bExactMatch) const
{
	auto CheckTag = [TagToFind, bExactMatch]
	(const FGameplayTag& TagToCheck) -> bool
	{
		if (bExactMatch)
		{
			return TagToFind.MatchesTag(TagToCheck);
		}

		return TagToFind.MatchesTag(TagToCheck);
	};

	bool bResult = false;
	bResult |= CheckTag(Mover_IsCrouching);
	bResult |= CheckTag(Mover_IsFat);
	//@TODO: Check for additional stance tags

	return bResult;
}

void FBotaniStanceModifier::OnStart(
	UMoverComponent* MoverComp,
	const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState,
	const FMoverAuxStateContext& AuxState)
{
	const UBotaniStanceSettings* StanceSettings = MoverComp->FindSharedSettings<UBotaniStanceSettings>();
	if (!IsValid(StanceSettings))
	{
		BOTANIMOVER_ERROR("Botani Stance Modifier: No valid BotaniStanceSettings found on MoverComponent %s", *MoverComp->GetName());
		return;
	}

	// Check if we can expand the capsule (in this project we only use capsules so that fine
	if (const UCapsuleComponent* CapsuleComp =
		CastChecked<UCapsuleComponent>(MoverComp->GetUpdatedComponent(), ECastCheckedType::NullAllowed))
	{
		const float OldHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
		float NewHalfHeight = 0.f, NewEyeHeight = 0.f;

		switch (ActiveStance)
		{
		default:
		case EBotaniStanceMode::Crouch:
			{
				NewHalfHeight = StanceSettings->CrouchHalfHeight;
				NewEyeHeight = StanceSettings->CrouchedEyeHeight;
				break;
			}

		case EBotaniStanceMode::Fat:
			{
				BOTANIMOVER_WARN("Stance got into fat stance - this is not implemented yet, so using crouch instead");
				DurationMs = 0.f; // Early out
				break;
			}
		}

		AdjustCapsule(MoverComp, OldHalfHeight, NewHalfHeight, NewEyeHeight);
		ApplyMovementSettings(MoverComp);
	}
}

void FBotaniStanceModifier::OnEnd(
	UMoverComponent* MoverComp,
	const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState,
	const FMoverAuxStateContext& AuxState)
{
	const APawn* OwnerCDO = MoverComp->GetOwner()->GetClass()->GetDefaultObject<APawn>();
	if (!IsValid(OwnerCDO))
	{
		return;
	}

	if (const UCapsuleComponent* CapsuleComp =
		CastChecked<UCapsuleComponent>(MoverComp->GetUpdatedComponent(), ECastCheckedType::NullAllowed))
	{
		if (const UCapsuleComponent* OriginalCapsule = UMovementUtils::GetOriginalComponentType<UCapsuleComponent>(MoverComp->GetOwner()))
		{
			AdjustCapsule(MoverComp,
				CapsuleComp->GetScaledCapsuleHalfHeight(),
				OriginalCapsule->GetScaledCapsuleHalfHeight(),
				OwnerCDO->BaseEyeHeight);

			RevertMovementSettings(MoverComp);
		}
	}
}

void FBotaniStanceModifier::OnPreMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep)
{
	if (ActiveStance == EBotaniStanceMode::Fat)
	{
		BOTANIMOVER_WARN("Stance got into fat stance - this is not implemented yet, so using crouch instead");
		DurationMs = 0.f; // Early out
	}
}

void FBotaniStanceModifier::OnPostMovement(UMoverComponent* MoverComp, const FMoverTimeStep& TimeStep,
	const FMoverSyncState& SyncState, const FMoverAuxStateContext& AuxState)
{
	Super::OnPostMovement(MoverComp, TimeStep, SyncState, AuxState);
}

FMovementModifierBase* FBotaniStanceModifier::Clone() const
{
	FBotaniStanceModifier* CopyPtr = new FBotaniStanceModifier(*this);
	return CopyPtr;
}

void FBotaniStanceModifier::NetSerialize(FArchive& Ar)
{
	Super::NetSerialize(Ar);
}

UScriptStruct* FBotaniStanceModifier::GetScriptStruct() const
{
	return StaticStruct();
}

FString FBotaniStanceModifier::ToSimpleString() const
{
	return FString::Printf(TEXT("Botani Stance Modifier"));
}

void FBotaniStanceModifier::AddReferencedObjects(FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
}

bool FBotaniStanceModifier::CanExpand(const UMoverComponent* MoverComp) const
{
	float StandingHalfHeight = 90;
	float CurrentHalfHeight = 55;

	USceneComponent* UpdatedComponent = MoverComp->GetUpdatedComponent();
	UPrimitiveComponent* UpdatedCompAsPrimitive = Cast<UPrimitiveComponent>(UpdatedComponent);

	if (const UCapsuleComponent* OriginalCapsule = UMovementUtils::GetOriginalComponentType<UCapsuleComponent>(MoverComp->GetOwner()))
	{
		StandingHalfHeight = OriginalCapsule->GetScaledCapsuleHalfHeight();
	}

	if (UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(UpdatedComponent))
	{
		CurrentHalfHeight = CapsuleComponent->GetScaledCapsuleHalfHeight();
	}

	float HalfHeightDelta = StandingHalfHeight - CurrentHalfHeight;

	// Perform a capsule trace to check if we can expand the capsule without colliding with anything
	FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, MoverComp->GetOwner());
	FCollisionResponseParams ResponseParam;
	UMovementUtils::InitCollisionParams(UpdatedCompAsPrimitive, CapsuleParams, ResponseParam);

	const FMoverDefaultSyncState* SyncState = MoverComp->GetSyncState().SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

	FVector PawnLocation = SyncState->GetLocation_WorldSpace();
	FQuat PawnRot = SyncState->GetOrientation_WorldSpace().Quaternion();
	float PawnRadius = 0.0f;
	float PawnHalfHeight = 0.0f;
	UpdatedCompAsPrimitive->CalcBoundingCylinder(PawnRadius, PawnHalfHeight);

	// TODO: Compensate for the difference between current capsule size and standing size
	FCollisionShape StandingCapsuleShape = FCollisionShape::MakeCapsule(PawnRadius, StandingHalfHeight);
	const ECollisionChannel CollisionChannel = UpdatedCompAsPrimitive->GetCollisionObjectType();
	bool bEncroached = true;

	// TODO: We may need to expand this check to look at more than just the initial overlap - see CMC Uncrouch for details
	const UCommonMoverComponent* CommonMover = Cast<UCommonMoverComponent>(MoverComp);
	if (!ShouldExpandingMaintainBase(CommonMover))
	{
		// Expand in place
		bEncroached = UMovementUtils::OverlapTest(UpdatedComponent, UpdatedCompAsPrimitive, PawnLocation, PawnRot, CollisionChannel, StandingCapsuleShape, MoverComp->GetOwner());
	}
	else
	{
		// Expand while keeping base location the same.
		FVector StandingLocation = PawnLocation + (HalfHeightDelta + .01f) * MoverComp->GetUpDirection();
		bEncroached = UMovementUtils::OverlapTest(UpdatedComponent, UpdatedCompAsPrimitive, StandingLocation, PawnRot, CollisionChannel, StandingCapsuleShape, MoverComp->GetOwner());
	}

	return !bEncroached;
}

bool FBotaniStanceModifier::ShouldExpandingMaintainBase(const UCommonMoverComponent* MoverComp) const
{
	if (MoverComp->IsOnGround())
	{
		return true;
	}

	return false;
}

void FBotaniStanceModifier::AdjustCapsule(
	UMoverComponent* MoverComp,
	float OldHalfHeight,
	float NewHalfHeight,
	float NewEyeHeight)
{
	const float HalfHeightDelta = FMath::Abs(NewHalfHeight - OldHalfHeight);
	const bool bExpanding = OldHalfHeight < NewHalfHeight;

	// Set capsule size to crouching size
	if (UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(MoverComp->GetOwner()->FindComponentByClass(UCapsuleComponent::StaticClass())))
	{
		if (CapsuleComponent->GetUnscaledCapsuleHalfHeight() == NewHalfHeight)
		{
			return;
		}

		CapsuleComponent->SetCapsuleSize(CapsuleComponent->GetUnscaledCapsuleRadius(), NewHalfHeight);
	}

	// update eye height on pawn
	if (APawn* MoverCompOwnerAsPawn = Cast<APawn>(MoverComp->GetOwner()))
	{
		MoverCompOwnerAsPawn->BaseEyeHeight = NewEyeHeight;
	}

	const FVector CapsuleOffset = MoverComp->GetUpDirection() * (bExpanding ? HalfHeightDelta : -HalfHeightDelta);
	// This is only getting used to add relative offset - so assuming z is up is fine here
	const FVector VisualOffset = FVector::UpVector * (bExpanding ? -HalfHeightDelta : HalfHeightDelta);

	// Adjust location of capsule as setting it's size left it floating
	if (!bExpanding || MoverComp->GetVelocity().Length() <= 0)
	{
		TSharedPtr<FTeleportEffect> TeleportEffect = MakeShared<FTeleportEffect>();
		TeleportEffect->TargetLocation = MoverComp->GetUpdatedComponentTransform().GetLocation() + (CapsuleOffset);
		MoverComp->QueueInstantMovementEffect(TeleportEffect);
	}

	// Add offset to visual component as the base location has changed
	FTransform MoverVisualComponentOffset = MoverComp->GetBaseVisualComponentTransform();
	MoverVisualComponentOffset.SetLocation(MoverVisualComponentOffset.GetLocation() + VisualOffset);
	MoverComp->SetBaseVisualComponentTransform(MoverVisualComponentOffset);
}

void FBotaniStanceModifier::ApplyMovementSettings(UMoverComponent* MoverComp)
{
	switch (ActiveStance)
	{
	default:
	case EBotaniStanceMode::Crouch:
		{
			if (const UBotaniStanceSettings* StanceSettings = MoverComp->FindSharedSettings<UBotaniStanceSettings>())
			{
				// Update relevant movement settings
				if (UBotaniCommonMovementSettings* BotaniMovementSettings = MoverComp->FindSharedSettings_Mutable<UBotaniCommonMovementSettings>())
				{
					BotaniMovementSettings->Acceleration = StanceSettings->CrouchingMaxAcceleration;
					BotaniMovementSettings->MaxSpeed = StanceSettings->CrouchingMaxSpeed;
				}
			}

			break;
		}

	case EBotaniStanceMode::Fat:
		{
			BOTANIMOVER_WARN("currently, being fat isn't implemented, only skinny characters!!!!")
			break;
		}
	}
}

void FBotaniStanceModifier::RevertMovementSettings(UMoverComponent* MoverComp)
{
	if (const UMoverComponent* CDOMoverComp = UMovementUtils::GetOriginalComponentType<UMoverComponent>(MoverComp->GetOwner()))
	{
		const UBotaniCommonMovementSettings* OriginalBotaniMovementSettings = CDOMoverComp->FindSharedSettings<UBotaniCommonMovementSettings>();
		UBotaniCommonMovementSettings* BotaniMovementSettings = MoverComp->FindSharedSettings_Mutable<UBotaniCommonMovementSettings>();

		// Revert movement settings back to original settings
		if (BotaniMovementSettings && OriginalBotaniMovementSettings)
		{
			BotaniMovementSettings->Acceleration = OriginalBotaniMovementSettings->Acceleration;
			BotaniMovementSettings->MaxSpeed = OriginalBotaniMovementSettings->MaxSpeed;
		}
	}
}
