// Author: Tom Werner (MajorT), 2025


#include "Components/BotaniMoverComponent.h"

#include "BotaniMoverSettings.h"
#include "Modes/BotaniMM_Falling.h"
#include "Modes/BotaniMM_Walking.h"
#include "Modes/BotaniMM_WallRunning.h"
#include "Modifiers/BotaniStanceModifier.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMoverComponent)


UBotaniMoverComponent::UBotaniMoverComponent(const FObjectInitializer& ObjectInitializer)
 	: Super(ObjectInitializer)
{
	// Default movement modes
	MovementModes.Add(
		DefaultModeNames::Walking,
		ObjectInitializer.CreateDefaultSubobject<UBotaniMM_Walking>(this, "ModeWalking"));

	MovementModes.Add(
		DefaultModeNames::Falling,
		ObjectInitializer.CreateDefaultSubobject<UBotaniMM_Falling>(this, "ModeFalling"));


	MovementModes.Add(
		BotaniMover::ModeNames::WallRunning,
		ObjectInitializer.CreateDefaultSubobject<UBotaniMM_WallRunning>(this, "ModeWallRunning"));

	PersistentSyncStateDataTypes.Add(FMoverDataPersistence(FGameplayTagsSyncState::StaticStruct(), false));

	StartingMovementMode = DefaultModeNames::Falling;
}

void UBotaniMoverComponent::BeginPlay()
{
	Super::BeginPlay();

	OnHandlerSettingChanged();
}

bool UBotaniMoverComponent::GetHandleStanceChanges() const
{
	return bHandleStanceChanges;
}

void UBotaniMoverComponent::SetHandleStanceChanges(bool bInHandleStanceChanges)
{
	if (bHandleStanceChanges != bInHandleStanceChanges)
	{
		bHandleStanceChanges = bInHandleStanceChanges;
		OnHandlerSettingChanged();
	}
}

bool UBotaniMoverComponent::IsWallRunning() const
{
	return HasGameplayTag(BotaniGameplayTags::Mover::Modes::TAG_MM_WallRunning, true);
}

void UBotaniMoverComponent::OnMoverPreSimulationTick(
	const FMoverTimeStep& TimeStep,
	const FMoverInputCmdContext& InputCmd)
{
	if (bHandleStanceChanges)
	{
		const FBotaniStanceModifier* StanceModifier =
			static_cast<const FBotaniStanceModifier*>(FindMovementModifier(StanceModifierHandle));

		// This is a fail-safe in case our handle was bad - try finding the modifier by type if we can
		if (!StanceModifier)
		{
			StanceModifier = FindMovementModifierByType<FBotaniStanceModifier>();
		}

		EBotaniStanceMode OldActiveStance = EBotaniStanceMode::Invalid;
		if (StanceModifier)
		{
			OldActiveStance = StanceModifier->ActiveStance;
		}

		const bool bIsCrouching = HasGameplayTag(Mover_IsCrouching, true);
		//@TODO: Crouch implementation

		EBotaniStanceMode NewActiveStance = EBotaniStanceMode::Invalid;
		if (StanceModifier)
		{
			NewActiveStance = StanceModifier->ActiveStance;
		}

		// Check if the stance has changed
		if (OldActiveStance != NewActiveStance)
		{
			OnStanceChanged.Broadcast(OldActiveStance, NewActiveStance);
		}
	}
}

void UBotaniMoverComponent::OnHandlerSettingChanged()
{
	if (bHandleStanceChanges)
	{
		OnPreSimulationTick.AddDynamic(this, &ThisClass::OnMoverPreSimulationTick);
	}
	else
	{
		OnPreSimulationTick.RemoveDynamic(this, &ThisClass::OnMoverPreSimulationTick);
	}
}
