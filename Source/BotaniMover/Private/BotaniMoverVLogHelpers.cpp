// Author: Tom Werner (MajorT), 2025


#include "BotaniMoverVLogHelpers.h"

#include "BotaniMoverLogChannels.h"

void BotaniMover::VLog::VisLogCommand(const UObject* LogOwner, const FVLogDrawCommand& Command)
{
	switch (Command.Type) {
	case FVLogDrawCommand::EDrawType::Point:
		{
			UE_VLOG_SEGMENT_THICK(LogOwner, VLogBotaniMover, Log,
				Command.LineStart,
				Command.LineStart,
				Command.Color,
				Command.Thickness,
				TEXT("%s"), *Command.Text);
			break;
		}

	case FVLogDrawCommand::EDrawType::Line:
		{
			UE_VLOG_SEGMENT(LogOwner, VLogBotaniMover, Log,
				Command.LineStart,
				Command.LineEnd,
				Command.Color,
				TEXT("%s"), *Command.Text);
			break;
		}

	case FVLogDrawCommand::EDrawType::DirectionalArrow:
		{
			UE_VLOG_ARROW(LogOwner, VLogBotaniMover, Log,
				Command.LineStart,
				Command.LineEnd,
				Command.Color,
				TEXT("%s"), *Command.Text);
			break;
		}

	case FVLogDrawCommand::EDrawType::Sphere:
		break;
	case FVLogDrawCommand::EDrawType::Box:
		break;
	case FVLogDrawCommand::EDrawType::String:
		{
			UE_VLOG(LogOwner, VLogBotaniMover, Log, TEXT("%s"), *Command.Text);
			break;
		}
	case FVLogDrawCommand::EDrawType::Circle:
		break;
	case FVLogDrawCommand::EDrawType::Capsule:
		{
			UE_VLOG_CAPSULE(LogOwner, VLogBotaniMover, Log,
				Command.Center,
				Command.HalfHeight,
				Command.Radius,
				Command.Rotation,
				Command.Color,
				TEXT("%s"), *Command.Text);
			break;
		}
	}
}
