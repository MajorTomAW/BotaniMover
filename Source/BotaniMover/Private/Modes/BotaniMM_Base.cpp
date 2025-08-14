// Author: Tom Werner (MajorT), 2025


#include "Modes/BotaniMM_Base.h"

#include "MoverComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverSettings.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMM_Base)

UBotaniMM_Base::UBotaniMM_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SharedSettingsClasses.Add(UBotaniMoverSettings::StaticClass());
	SharedSettingsClasses.Add(UBotaniCommonMovementSettings::StaticClass());
}

void UBotaniMM_Base::OnRegistered(const FName ModeName)
{
	Super::OnRegistered(ModeName);

	// Get the botani mover settings
	BotaniMoverSettings = GetMoverComponent()->FindSharedSettings<UBotaniMoverSettings>();
	ensureMsgf(BotaniMoverSettings, TEXT("Failed to find instance of BotaniMoverSettings on %s. Movement may not function properly."),
		*GetPathNameSafe(this));

	// Get the common movement settings
	BotaniMovementSettings = GetMoverComponent()->FindSharedSettings<UBotaniCommonMovementSettings>();
	ensureMsgf(BotaniMovementSettings, TEXT("Failed to find instance of BotaniCommonMovementSettings on %s. Movement may not function properly."),
		*GetPathNameSafe(this));
}

void UBotaniMM_Base::OnUnregistered()
{
	// Release the botani mover settings pointer
	BotaniMoverSettings = nullptr;

	// Release the common movement settings pointer
	BotaniMovementSettings = nullptr;

	Super::OnUnregistered();
}
