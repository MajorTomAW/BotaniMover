// Author: Tom Werner (MajorT), 2025


#include "Transitions/BotaniMMT_Base.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BotaniMoverVLogHelpers.h"
#include "MoverComponent.h"
#include "MoverSimulationTypes.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "MoveLibrary/MoverBlackboard.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniMMT_Base)

UBotaniMMT_Base::UBotaniMMT_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBotaniMMT_Base::Trigger_Implementation(const FSimulationTickParams& Params)
{
	// Get the blackboard
	UMoverBlackboard* SimBlackboard = Params.MovingComps.MoverComponent->GetSimBlackboard_Mutable();
	if (IsValid(SimBlackboard) && BlackboardTimeLoggingKey != NAME_None)
	{
		// Save the last time into the blackboard
		SimBlackboard->Set<float>(BlackboardTimeLoggingKey, Params.TimeStep.BaseSimTimeMs);
	}

	// Send the trigger event
	if (TriggerEventTag.IsValid())
	{
		FGameplayEventData Payload;
		Payload.EventTag = TriggerEventTag;
		Payload.EventMagnitude = Params.TimeStep.BaseSimTimeMs;
		Payload.Instigator = Params.MovingComps.MoverComponent->GetOwner();
		Payload.Target = Params.MovingComps.MoverComponent->GetOwner();
		Payload.OptionalObject = Params.MovingComps.MoverComponent.Get();

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Params.MovingComps.MoverComponent->GetOwner(), Payload.EventTag, Payload);
	}

#if ENABLE_VISUAL_LOG
	if (bShouldCreateVisLogEntry)
	{
		using namespace BotaniMover::VLog;

		VisLogCommand(Params.MovingComps.MoverComponent->GetOwner(),
			FVLogDrawCommand::DrawDebugCapsule(Params.MovingComps.UpdatedComponent.Get(),
				FColor::Green,
				Params.MovingComps.UpdatedComponent->GetComponentQuat(),
				1.f,
				FString::Printf(TEXT("Triggered movement mode transition\n\t%s -> %s"),
					*GetNameSafe(Params.MovingComps.MoverComponent->GetMovementMode()), *GetNameSafe(this))));
	}
#endif
}
