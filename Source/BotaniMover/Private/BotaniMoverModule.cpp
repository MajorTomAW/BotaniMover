// Copyright Epic Games, Inc. All Rights Reserved.

#include "BotaniMoverModule.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "Debug/GameplayDebuggerCategory_BotaniMover.h"
#define BOTANIMOVER_CATEGORY_NAME "Botanimover"
#endif

#define LOCTEXT_NAMESPACE "FBotaniMoverModule"

void FBotaniMoverModule::StartupModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
	GameplayDebuggerModule.RegisterCategory(BOTANIMOVER_CATEGORY_NAME, IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategory_BotaniMover::MakeInstance));
	GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
}

void FBotaniMoverModule::ShutdownModule()
{
#if WITH_GAMEPLAY_DEBUGGER
	if (IGameplayDebugger::IsAvailable())
	{
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.UnregisterCategory(BOTANIMOVER_CATEGORY_NAME);
		GameplayDebuggerModule.NotifyCategoriesChanged();
	}
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBotaniMoverModule, BotaniMover)
