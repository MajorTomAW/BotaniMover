// Author: Tom Werner (MajorT), 2025

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"

/**
 * Pretty much a copy of FGameplayDebuggerCategory_Mover, however now also with Blackboard data,
 * it was annoying to not see what was going on at the blackboard.
 *
 * NOTE: You should disable the engine "Mover" category in the Gameplay Debugger settings.
 */
class FGameplayDebuggerCategory_BotaniMover : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_BotaniMover();
	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	//~ Begin FGameplayDebuggerCategory Interface
	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual	void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;
	//~ End FGameplayDebuggerCategory Interface

protected:
	struct FRepData
	{
		FRepData()
			: Velocity(FVector::ZeroVector)
			, MoveIntent(FVector::ZeroVector)
			, MoveInputType(0)
			, MoveInput(FVector::ZeroVector)
			, OrientIntentDir(FVector::ZeroVector)
		{
		}

		// Player
		FString PawnName;
		FString LocalRole;

		// State
		FString MovementModeName;
		FString MovementBaseInfo;
		FVector Velocity;
		FVector MoveIntent;
		TArray<FString> ActiveLayeredMoves;
		TArray<FString> ModeMap;
		TArray<FString> ActiveTransitions;
		TArray<FString> ActiveModifiers;
		TArray<FString> SyncStateDataTypes;

		// Last input
		uint8 MoveInputType;
		FVector MoveInput;
		FVector OrientIntentDir;
		FString SuggestedModeName;

		// Blackboard data
		TMap<FString, FString> BlackboardData;

	public:
		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;

private:
	void DrawOverheadInfo(AActor& DebugActor, FGameplayDebuggerCanvasContext& CanvasContext);
	void DrawInWorldInfo(AActor& DebugActor, FGameplayDebuggerCanvasContext& CanvasContext);


	// This method is the almost sole reason for this entire class to exist.
	void CollectBlackboardDebugData(FRepData& InOutDataPack, const class UMoverBlackboard* SimBlackboard);
};

#endif
