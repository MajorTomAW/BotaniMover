// Author: Tom Werner (MajorT), 2025


#include "GameplayDebuggerCategory_BotaniMover.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "BotaniMoverSettings.h"
#include "MoverComponent.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Debug/MoverDebugComponent.h"

FGameplayDebuggerCategory_BotaniMover::FGameplayDebuggerCategory_BotaniMover()
{
	SetDataPackReplication<FRepData>(&DataPack);
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_BotaniMover::MakeInstance()
{
	return MakeShared<FGameplayDebuggerCategory_BotaniMover>();
}

void FGameplayDebuggerCategory_BotaniMover::CollectData(
	APlayerController* OwnerPC, AActor* DebugActor)
{
	APawn* MyPawn = Cast<APawn>(DebugActor);
	UMoverComponent* MyMoverComponent = MyPawn
		? Cast<UMoverComponent>(MyPawn->GetComponentByClass(UMoverComponent::StaticClass()))
		: nullptr;

	if (IsValid(MyPawn))
	{
		// Get the debug component
		UMoverDebugComponent* MoverDebugComponent =
			Cast<UMoverDebugComponent>(MyPawn->GetComponentByClass(UMoverDebugComponent::StaticClass()));

		// Make it if we can't find it
		if (!IsValid(MoverDebugComponent))
		{
			MoverDebugComponent = Cast<UMoverDebugComponent>(
				MyPawn->AddComponentByClass(
					UMoverDebugComponent::StaticClass(),
					false,
					FTransform::Identity,
					false));

			MoverDebugComponent->SetHistoryTracking(1.0f, 20.0f);
		}

		MoverDebugComponent->bShowTrajectory = false;
		MoverDebugComponent->bShowTrail = false;
		MoverDebugComponent->bShowCorrections = false;

		// I am not sure how good this way actually is, but as Mover didnt expose them to us, i dont see any other way.
		IConsoleManager& ConsoleMgr = IConsoleManager::Get();

		{
			bool bShowTrajectory = false;
			ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.ShowTrajectory"))->GetValue(bShowTrajectory);
			if (bShowTrajectory)
			{
				MoverDebugComponent->DrawTrajectory();
			}
		}

		{
			bool bShowTrail = false;
			ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.ShowTrail"))->GetValue(bShowTrail);
			if (bShowTrail)
			{
				MoverDebugComponent->DrawTrail();
			}
		}

		{
			bool bShowCorrections = false;
			ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.ShowCorrections"))->GetValue(bShowCorrections);
			if (bShowCorrections)
			{
				MoverDebugComponent->DrawCorrections();
			}
		}
	}

	DataPack.PawnName = MyPawn
		? MyPawn->GetHumanReadableName()
		: FString(TEXT("{red}No selected pawn."));

	DataPack.LocalRole = MyPawn
		? StaticEnum<ENetRole>()->GetDisplayValueAsText(MyPawn->GetLocalRole()).ToString()
		: FString();


	// Set defaults for info that may not be available
	DataPack.MovementModeName = FString("invalid");
	DataPack.MovementBaseInfo = FString("invalid");
	DataPack.Velocity = FVector::ZeroVector;
	DataPack.MoveIntent = FVector::ZeroVector;
	DataPack.ActiveLayeredMoves.Empty();
	DataPack.ActiveModifiers.Empty();
	DataPack.SyncStateDataTypes.Empty();
	DataPack.ModeMap.Empty();
	DataPack.MoveInputType = 0;
	DataPack.MoveInput = FVector::ZeroVector;
	DataPack.OrientIntentDir = FVector::ZeroVector;
	DataPack.SuggestedModeName = FString("invalid");

	if (MyMoverComponent)
	{
		UPrimitiveComponent* MovementBaseComp = MyMoverComponent->GetMovementBase();

		DataPack.MovementModeName = MyMoverComponent->GetMovementModeName().ToString();
		DataPack.MovementBaseInfo = MovementBaseComp ? FString::Printf(TEXT("%s.%s"), *GetNameSafe(MovementBaseComp->GetOwner()), *MovementBaseComp->GetName()) : FString();
		DataPack.MoveIntent = MyMoverComponent->GetMovementIntent();
		DataPack.Velocity = MyMoverComponent->GetVelocity();

		for (auto ModeIter = MyMoverComponent->MovementModes.begin(); ModeIter; ++ModeIter)
		{
			UBaseMovementMode* MappedMode = ModeIter->Value;
			DataPack.ModeMap.Add(FString::Printf(TEXT("%s => %s"), *ModeIter->Key.ToString(), (MappedMode ? *MappedMode->GetClass()->GetName() : TEXT("null"))));

			if (ModeIter->Key == MyMoverComponent->GetMovementModeName())
			{
				for (UBaseMovementModeTransition* Transition : MyMoverComponent->MovementModes[ModeIter->Key]->Transitions)
				{
					DataPack.ActiveTransitions.Add(FString::Printf(TEXT("%s (%s)"), (Transition ? *Transition->GetClass()->GetName() : TEXT("null")), *ModeIter->Key.ToString()));
				}
			}
		}

		for (UBaseMovementModeTransition* Transition : MyMoverComponent->Transitions)
		{
			DataPack.ActiveTransitions.Add(FString::Printf(TEXT("%s (global)"), (Transition ? *Transition->GetClass()->GetName() : TEXT("null"))));
		}

		const FMoverSyncState& SyncState = MyMoverComponent->GetSyncState();

		for (const TSharedPtr<FLayeredMoveBase>& ActiveMove : SyncState.LayeredMoves.GetActiveMoves())
		{
			DataPack.ActiveLayeredMoves.Add(ActiveMove->ToSimpleString());
		}

		for (auto It = SyncState.MovementModifiers.GetActiveModifiersIterator(); It; ++It)
		{
			DataPack.ActiveModifiers.Add(*It->Get()->ToSimpleString());
		}

		for (auto It = SyncState.SyncStateCollection.GetDataArray().CreateConstIterator(); It; ++It)
		{
			DataPack.SyncStateDataTypes.Add(It->Get()->GetScriptStruct()->GetName());
		}

		const FMoverInputCmdContext& LastInputCmd = MyMoverComponent->GetLastInputCmd();

		if (const FCharacterDefaultInputs* DefaultInputs = LastInputCmd.InputCollection.FindDataByType<FCharacterDefaultInputs>())
		{
			DataPack.MoveInputType = static_cast<uint8>(DefaultInputs->GetMoveInputType());
			DataPack.MoveInput = DefaultInputs->GetMoveInput_WorldSpace();
			DataPack.OrientIntentDir = DefaultInputs->GetOrientationIntentDir_WorldSpace();
			DataPack.SuggestedModeName = DefaultInputs->SuggestedMovementMode.ToString();
		}

		if (const UMoverBlackboard* SimBlackboard = MyMoverComponent->GetSimBlackboard())
		{
			CollectBlackboardDebugData(DataPack, SimBlackboard);
		}
	}
}

void FGameplayDebuggerCategory_BotaniMover::CollectBlackboardDebugData(
	FRepData& InOutDataPack,
	const UMoverBlackboard* SimBlackboard)
{
	auto AddFloatValue = [&MyDataPack=InOutDataPack, &MySimBlackboard=SimBlackboard]
		(FName BlackboardKey)
		{
			float Value = 0.f;
			if (MySimBlackboard->TryGet<float>(BlackboardKey, Value))
			{
				// Add the key and value to the data pack
				// Format: "KeyName: Value (Value in seconds)"
				MyDataPack.BlackboardData.Add(
					BlackboardKey.ToString(),
					FString::Printf(TEXT("%f\t\t{grey}(%.2fs)"), Value, Value * BotaniMover::Lazy::MsToS));
			}
		};

	AddFloatValue(BotaniMover::Blackboard::LastFallTime);
	AddFloatValue(BotaniMover::Blackboard::LastJumpTime);
	AddFloatValue(BotaniMover::Blackboard::LastWallRunTime);
	AddFloatValue(BotaniMover::Blackboard::LastWallRunStartTime);
	AddFloatValue(BotaniMover::Blackboard::LastWallJumpTime);
}

void FGameplayDebuggerCategory_BotaniMover::DrawData(
	APlayerController* OwnerPC,
	FGameplayDebuggerCanvasContext& CanvasContext)
{
	AActor* FocusedActor = FindLocalDebugActor();

	if (FocusedActor != nullptr)
	{
		// Display any info attached to the focused actor

		DrawOverheadInfo(*FocusedActor, CanvasContext);
		DrawInWorldInfo(*FocusedActor, CanvasContext);
	}

	// Compact player info
	CanvasContext.Printf(TEXT("{yellow}%s\n{grey}Local Role: {white}%s\n{grey}Mode: {white}%s\n{grey}Velocity: {white}%s\n{grey}Speed: {white}%.2f"),
		*DataPack.PawnName,
		*DataPack.LocalRole,
		*DataPack.MovementModeName,
		*DataPack.Velocity.ToString(),
		 DataPack.Velocity.Length()
		);

	// Move info
	if (DataPack.MoveInputType > 0)
	{
		CanvasContext.Printf(TEXT("{grey}Move Input Type: {white}%s  {grey}Vec: {white}%s\n{grey}Input Suggested Mode: {white}%s\n{grey}Input Orient Intent: {white}%s"),
			*StaticEnum<EMoveInputType>()->GetDisplayValueAsText((EMoveInputType)DataPack.MoveInputType).ToString(),
			*DataPack.MoveInput.ToString(),
			*DataPack.SuggestedModeName,
			*DataPack.OrientIntentDir.ToString()
		);
	}

	// Advanced move info
	CanvasContext.Printf(TEXT("{yellow}Active Moves: {white}\n%s\n{yellow}Active Modifiers: {white}\n%s\n{yellow}Mode Map: \n{white}%s\n{yellow}Active Transitions: {white}\n%s\n{yellow}SyncStateTypes: {white}%s"),
		*FString::JoinBy(DataPack.ActiveLayeredMoves, TEXT("\n"), [](FString MoveAsString) { return MoveAsString; }),
		*FString::JoinBy(DataPack.ActiveModifiers, TEXT("\n"), [](FString ModifierAsString) { return ModifierAsString; }),
		*FString::JoinBy(DataPack.ModeMap, TEXT("\n"), [](FString ModeMappingAsString) { return ModeMappingAsString; }),
		*FString::JoinBy(DataPack.ActiveTransitions, TEXT("\n"), [](FString TransitionAsString) { return TransitionAsString; }),
		*FString::JoinBy(DataPack.SyncStateDataTypes, TEXT(","), [](FString SyncStateTypeAsString) { return SyncStateTypeAsString; })
		);

	// Blackboard data
	if (DataPack.BlackboardData.Num() > 0)
	{
		CanvasContext.Printf(TEXT("\n\n{yellow}Blackboard Data: {white}\n%s"),
			*FString::JoinBy(DataPack.BlackboardData, TEXT("\n"), [](const TPair<FString, FString>& KeyValuePair)
			{
				return FString::Printf(TEXT("{grey}%s: {white}%s"), *KeyValuePair.Key, *KeyValuePair.Value);
			}));
	}
}

void FGameplayDebuggerCategory_BotaniMover::DrawOverheadInfo(
	AActor& DebugActor,
	FGameplayDebuggerCanvasContext& CanvasContext)
{
	const FVector OverheadLocation = DebugActor.GetActorLocation() + FVector(0, 0, DebugActor.GetSimpleCollisionHalfHeight());

	if (CanvasContext.IsLocationVisible(OverheadLocation))
	{
		FGameplayDebuggerCanvasContext OverheadContext(CanvasContext);
		TWeakObjectPtr<UFont> testFont = GEngine->GetSmallFont();
		OverheadContext.Font = GEngine->GetSmallFont();
		OverheadContext.FontRenderInfo.bEnableShadow = true;

		const FVector2D ScreenLoc = OverheadContext.ProjectLocation(OverheadLocation);

		FString ActorDesc;

		if (DataPack.MovementBaseInfo.Len() > 0)
		{
			ActorDesc = FString::Printf(TEXT("{yellow}%s\n{white}%s\nBase: %s"), *DataPack.PawnName, *DataPack.MovementModeName, *DataPack.MovementBaseInfo);
		}
		else
		{
			ActorDesc = FString::Printf(TEXT("{yellow}%s\n{white}%s"), *DataPack.PawnName, *DataPack.MovementModeName);
		}

		float SizeX(0.f), SizeY(0.f);
		OverheadContext.MeasureString(ActorDesc, SizeX, SizeY);
		OverheadContext.PrintAt(ScreenLoc.X - (SizeX * 0.5f), ScreenLoc.Y - (SizeY * 1.2f), ActorDesc);
	}
}

void FGameplayDebuggerCategory_BotaniMover::DrawInWorldInfo(
	AActor& DebugActor,
	FGameplayDebuggerCanvasContext& CanvasContext)
{
	UWorld* MyWorld = CanvasContext.GetWorld();
	IConsoleManager& ConsoleMgr = IConsoleManager::Get();

	UMoverComponent* MoverComp = Cast<UMoverComponent>(DebugActor.GetComponentByClass(UMoverComponent::StaticClass()));

	const FVector ActorMidLocation = DebugActor.GetActorLocation();
	const FVector ActorLowLocation = ActorMidLocation - FVector(0,0,DebugActor.GetSimpleCollisionHalfHeight()*0.95f);	// slightly above lowest point

	const FVector NudgeUp(0.0, 0.0, 2.0);


	// Draw approximate bounds
	if (CanvasContext.IsLocationVisible(ActorMidLocation))
	{
		DrawDebugCapsule(MyWorld,
			ActorMidLocation,
			DebugActor.GetSimpleCollisionHalfHeight(),
			DebugActor.GetSimpleCollisionRadius(),
			FQuat(DebugActor.GetActorRotation()),
			FColor::Green);
	}

	float MaxMoveIntentDrawLength = 0.f;
	ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.MaxMoveIntentDrawLength"))->GetValue(MaxMoveIntentDrawLength);
	float OrientationDrawLength = 0.f;
	ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.OrientationDrawLength"))->GetValue(OrientationDrawLength);

	bool bShowStateArrowViz = false;
	ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.ShowStateArrows"))->GetValue(bShowStateArrowViz);
	if (bShowStateArrowViz)
	{
		// Draw arrow showing movement intent (direction + magnitude)
		if (CanvasContext.IsLocationVisible(ActorLowLocation))
		{
			DrawDebugDirectionalArrow(MyWorld,
				ActorMidLocation,
				ActorMidLocation + (DataPack.MoveIntent * MaxMoveIntentDrawLength),
				40.f, FColor::Blue, false, -1.f, 0, 3.f);
		}

		// Draw overlaid arrows showing target orientation and actual
		if (MoverComp)
		{
			const FMoverSyncState& LastState = MoverComp->GetSyncState();
			const FMoverDefaultSyncState* MoverState = LastState.SyncStateCollection.FindDataByType<FMoverDefaultSyncState>();

			if (MoverState)
			{
				const FVector ActualFacingDir = MoverState->GetOrientation_WorldSpace().Vector();
				const FVector TargetFacingDir = MoverComp->GetTargetOrientation().Vector();

				DrawDebugDirectionalArrow(MyWorld,
					ActorLowLocation,
					ActorLowLocation + (TargetFacingDir * OrientationDrawLength),
					30.f, FColor::Yellow, false, -1.f, 0, 2.5f);

				DrawDebugDirectionalArrow(MyWorld,
					ActorLowLocation + NudgeUp,
					ActorLowLocation + NudgeUp + (ActualFacingDir * OrientationDrawLength * 0.9),
					10.f, FColor::Green, false, -1.f, 0, 1.25f);

			}
		}
	}

	bool bShowInputArrowViz = false;
	ConsoleMgr.FindConsoleVariable(TEXT("mover.debug.ShowInputArrows"))->GetValue(bShowInputArrowViz);
	if (bShowInputArrowViz)
	{
		// Draw arrows showing what the input cmds want to do
		if (CanvasContext.IsLocationVisible(ActorMidLocation))
		{
			if (!DataPack.MoveInput.IsNearlyZero())
			{
				DrawDebugDirectionalArrow(MyWorld,
					ActorMidLocation,
					ActorMidLocation + (DataPack.MoveInput.GetSafeNormal() * MaxMoveIntentDrawLength),
					40.f, FColor::Cyan, false, -1.f, 0, 3.f);
			}

			if (!DataPack.OrientIntentDir.IsNearlyZero())
			{
				DrawDebugDirectionalArrow(MyWorld,
					ActorMidLocation + NudgeUp,
					ActorMidLocation + NudgeUp + (DataPack.OrientIntentDir.GetSafeNormal() * MaxMoveIntentDrawLength),
					30.f, FColor::Orange, false, -1.f, 0, 3.f);
			}
		}
	}
}

void FGameplayDebuggerCategory_BotaniMover::FRepData::Serialize(FArchive& Ar)
{
	Ar << PawnName;
	Ar << LocalRole;
	Ar << MovementModeName;
	Ar << MovementBaseInfo;
	Ar << Velocity;
	Ar << MoveIntent;
	Ar << ActiveLayeredMoves;
	Ar << ActiveModifiers;
	Ar << SyncStateDataTypes;
	Ar << ModeMap;
	Ar << ActiveTransitions;
	Ar << MoveInputType;
	Ar << MoveInput;
	Ar << OrientIntentDir;
	Ar << SuggestedModeName;
	Ar << BlackboardData;
}


#endif
