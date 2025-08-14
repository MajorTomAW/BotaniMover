// Author: Tom Werner (MajorT), 2025


#include "Components/BotaniVaultingComponent.h"

#include "BotaniMoverAbilityInputs.h"
#include "MotionWarpingComponent.h"
#include "MoverComponent.h"
#include "Components/BotaniMoverComponent.h"
#include "DefaultMovementSet/Settings/CommonLegacyMovementSettings.h"
#include "MoveLibrary/VaultingQueryUtils.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "BotaniVaultingComponent"
#endif


#include "BotaniCommonMovementSettings.h"
#include "BotaniMoverSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BotaniVaultingComponent)

UBotaniVaultingComponent::UBotaniVaultingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UBotaniVaultingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Grab the mover component on the owning pawn
	const APawn* Pawn = GetPawn<APawn>();
	check(IsValid(Pawn));

	UMoverComponent* MoverComp = Pawn->FindComponentByClass<UMoverComponent>();
	check(IsValid(MoverComp));

	WeakMoverComp = Cast<UBotaniMoverComponent>(MoverComp);

	MoverComp->OnPreSimulationTick.AddDynamic(this, &ThisClass::OnMoverPreSimulationTick);
}

void UBotaniVaultingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UBotaniMoverComponent* BotaniMover = GetMoverComponent())
	{
		BotaniMover->OnPreSimulationTick.RemoveAll(this);
	}
}

void UBotaniVaultingComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

#if WITH_EDITOR
EDataValidationResult UBotaniVaultingComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	auto IsImplementedInBlueprint = [](const UFunction* Func) -> bool
	{
		return Func && ensure(Func->GetOuter())
			&& Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	if (!IsImplementedInBlueprint(GetClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(ThisClass, OnPlayMoverVaultingMontage))))
	{
		Result = EDataValidationResult::Invalid;
		Context.AddError(
			FText::Format(
				LOCTEXT("OnPlayVaultingMontageNotImplementedError", "OnPlayMoverVaultingMontage is not implemented in Blueprint for {0}.\nYou must implement it or otherwise vaulting won't work."),
				FText::FromString(GetClass()->GetName())));
	}

	return Result;
}
#endif

UBotaniMoverComponent* UBotaniVaultingComponent::GetMoverComponent() const
{
	return WeakMoverComp.Get();
}

void UBotaniVaultingComponent::OnMoverPreSimulationTick(
	const FMoverTimeStep& TimeStep,
	const FMoverInputCmdContext& InputCmd)
{
	UBotaniMoverComponent* BotaniMover = GetMoverComponent();
	if (!IsValid(BotaniMover))
	{
		return;
	}

	// We should only handle vaulting if we are airborne
	if (!BotaniMover->IsAirborne())
	{
		return;
	}

	// Get the pawn
	const APawn* Pawn = BotaniMover->GetOwner<APawn>();
	if (!IsValid(Pawn))
	{
		return;
	}

	// Get the ability inputs
	const FBotaniMoverAbilityInputs* AbilityInputs =
		InputCmd.InputCollection.FindDataByType<FBotaniMoverAbilityInputs>();

	// Only handle vaulting if the vault button was pressed this frame
	//@TODO: Ability Inputs should always be valid now, but for some reason not when we play as client !!
	if (!AbilityInputs || !AbilityInputs->bVaultPressedThisFrame)
	{
		return;
	}

	// Find a vaulting path
	FFloatRange VaultSlopeRangeCosine;
	VaultSlopeRangeCosine.SetLowerBoundValue(FMath::Cos(MinVaultingSlopeAngle.GetValue()));
	VaultSlopeRangeCosine.SetUpperBoundValue(FMath::Cos(MaxVaultingSlopeAngle.GetValue()));

	FMovingComponentSet MovingComps;
	MovingComps.SetFrom(BotaniMover);

	FVaultingPathCheckResult VaultingPathCheck;
	UVaultingQueryUtils::FindVaultingPath(
		MovingComps,
		MaxVaultingHeight.GetValue(),
		MinVaultingHeight.GetValue(),
		VaultingTraceDistance.GetValue(),
		NumVaultingSamples,
		Pawn->GetActorLocation(),
		Pawn->GetActorRotation(),
		VaultSlopeRangeCosine,
		VaultingPathCheck);

	if (!VaultingPathCheck.IsValidVaultingPath())
	{
		return;
	}

	// Update the motion warping targets
	UMotionWarpingComponent* MotionWarpingComp = Pawn->FindComponentByClass<UMotionWarpingComponent>();
	if (IsValid(MotionWarpingComp))
	{
		MotionWarpingComp->AddOrUpdateWarpTargetFromLocation(VaultWarpTargetName, VaultingPathCheck.HitResult.Location);
	}

	// Queue falling movement mode
	const UBotaniMoverSettings* BotaniMoverSettings = BotaniMover->FindSharedSettings<UBotaniMoverSettings>();
	check(BotaniMoverSettings);

	BotaniMover->QueueNextMode(BotaniMoverSettings->AirMovementModeName, false);

	// Play the vaulting montage
	if (IsValid(VaultingMontage))
	{
		OnPlayMoverVaultingMontage(BotaniMover, VaultingMontage, MotionWarpingComp);
	}

	OnVaultingStarted.Broadcast(VaultingPathCheck);
}

#if WITH_EDITOR
#undef LOCTEXT_NAMESPACE
#endif
