// Author: Tom Werner (MajorT), 2025

#pragma once
#include "Components/CapsuleComponent.h"

namespace BotaniMover::VLog
{
	struct FVLogDrawCommand
	{
		FVLogDrawCommand()
			: LineStart(ForceInit)
			, LineEnd(ForceInit)
			, Color(FColor::Green)
			, ArrowSize(1)
			, Thickness(1.f)
			, Radius(0)
			, HalfHeight(0)
			, Center(ForceInit)
			, Extent(ForceInit)
			, Rotation(ForceInit)
			, TextLocation(ForceInit)
			, TestBaseActor(nullptr)
			, bDrawShadow(false)
			, FontScale(1.f)
			, Duration(1.f)
			, TransformMatrix()
			, bDrawAxis(false)
			, YAxis(ForceInit)
			, ZAxis(ForceInit)
			, Type(EDrawType::Sphere)
		{
		}

		FVector LineStart;
		FVector LineEnd;
		FColor Color;
		float ArrowSize;
		float Thickness;
		float Radius;
		float HalfHeight;
		FVector Center;
		FVector Extent;
		FQuat Rotation;
		FVector TextLocation;
		FString Text;
		class AActor* TestBaseActor;
		bool bDrawShadow;
		float FontScale;
		float Duration;
		FMatrix TransformMatrix;
		bool bDrawAxis;
		FVector YAxis;
		FVector ZAxis;

		enum class EDrawType
		{
			Point,
			Line,
			DirectionalArrow,
			Sphere,
			Box,
			String,
			Circle,
			Capsule,
		} Type;

		static FVLogDrawCommand DrawPoint(const FVector& InLocation, const FColor& InColor, float InThickness = 1.f, const FString& InText = FString())
		{
			FVLogDrawCommand Command;
			Command.LineStart = InLocation;
			Command.Color = InColor;
			Command.Thickness = InThickness;
			Command.Type = EDrawType::Point;
			Command.Text = InText;
			return Command;
		}

		static FVLogDrawCommand DrawDebugCapsule(const USceneComponent* UpdatedComponent, const FColor& InColor, const FQuat& InRotation = FQuat::Identity, float InThickness = 1.f, const FString& InText = FString())
		{
			float PawnRadius = 0.0f;
			float PawnHalfHeight = 0.0f;
			const UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(UpdatedComponent);
			CapsuleComponent->GetScaledCapsuleSize(PawnRadius, PawnHalfHeight);

			const FVector Base = UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, PawnHalfHeight);

			FVLogDrawCommand Command;
			Command.Center = Base;
			Command.HalfHeight = PawnHalfHeight;
			Command.Radius = PawnRadius;
			Command.Rotation = InRotation;
			Command.Color = InColor;
			Command.Thickness = InThickness;
			Command.Type = EDrawType::Capsule;
			Command.Text = InText;
			return Command;
		}

		static FVLogDrawCommand DrawArrow(const FVector& LineStart, const FVector& LineEnd, const FColor& InColor, const FString& InText = FString())
		{
			FVLogDrawCommand Command;
			Command.LineStart = LineStart;
			Command.LineEnd = LineEnd;
			Command.Color = InColor;
			Command.Type = EDrawType::DirectionalArrow;
			Command.Text = InText;
			return Command;
		}
	};

	void VisLogCommand(const UObject* LogOwner, const FVLogDrawCommand& Command);
}
