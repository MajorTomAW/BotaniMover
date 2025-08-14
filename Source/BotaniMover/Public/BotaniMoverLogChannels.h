// Author: Tom Werner (MajorT), 2025

#pragma once

#include "Logging/LogMacros.h"
#include <concepts>
#include <type_traits>
#include <utility>

class UObject;

BOTANIMOVER_API DECLARE_LOG_CATEGORY_EXTERN(LogBotaniMover, Log, All);
BOTANIMOVER_API DECLARE_LOG_CATEGORY_EXTERN(VLogBotaniMover, Log, All);

#define EXEC_INFO_FORMAT "%s: "
#define EXE_INFO *FString(__FUNCTION__)

#define _BOTANIMOVER_LOG_IMPL(LogCategory, Verbosity, Format, ...) \
	UE_LOG(LogCategory, Verbosity, TEXT(EXEC_INFO_FORMAT Format), EXE_INFO, ##__VA_ARGS__)

// LogBotaniMover
#define BOTANIMOVER_LOG(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Log, Format, ##__VA_ARGS__)
#define BOTANIMOVER_VERBOSE(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Verbose, Format, ##__VA_ARGS__)
#define BOTANIMOVER_WARN(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Warning, Format, ##__VA_ARGS__)
#define BOTANIMOVER_ERROR(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Error, Format, ##__VA_ARGS__)
#define BOTANIMOVER_FATAL(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Fatal, Format, ##__VA_ARGS__)
#define BOTANIMOVER_DISPLAY(Format, ...) _BOTANIMOVER_LOG_IMPL(LogBotaniMover, Display, Format, ##__VA_ARGS__)
