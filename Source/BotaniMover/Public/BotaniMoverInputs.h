// Author: Tom Werner (MajorT), 2025

#pragma once

#include "MoverTypes.h"

#include "BotaniMoverInputs.generated.h"

/** Input data for botani mover. */
USTRUCT(BlueprintType)
struct FBotaniMoverInputs : public FMoverDataStructBase
{
	GENERATED_BODY()

public:
	FBotaniMoverInputs()
		: InvisibleForce(FVector::ZeroVector)
	{
	}

	virtual ~FBotaniMoverInputs() override {}

public:
	/** Invisible force vector applied to the player when moving. */
	UPROPERTY(BlueprintReadWrite, Category=Input)
	FVector InvisibleForce;

public:
	//~ Begin FMoverDataStructBase Interface
	virtual FMoverDataStructBase* Clone() const override
	{
		FBotaniMoverInputs* CopyPtr = new FBotaniMoverInputs(*this);
		return CopyPtr;
	}

	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override
	{
		Super::NetSerialize(Ar, Map, bOutSuccess);

		// Serialize the invisible force vector
		Ar << InvisibleForce;

		bOutSuccess = true;
		return true;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return StaticStruct();
	}

	virtual void ToString(FAnsiStringBuilderBase& Out) const override
	{
		Super::ToString(Out);
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Super::AddReferencedObjects(Collector);
	}
	//~ End FMoverDataStructBase Interface
};

template<>
struct TStructOpsTypeTraits< FBotaniMoverInputs > : public TStructOpsTypeTraitsBase2< FBotaniMoverInputs >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
