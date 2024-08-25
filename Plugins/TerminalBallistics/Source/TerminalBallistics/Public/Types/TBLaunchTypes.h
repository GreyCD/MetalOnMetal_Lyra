// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Misc/TBCollisionPresets.h"
#include "Misc/TBMacrosAndFunctions.h"
#include "Types/TBLocationAndDirection.h"
#include "Types/TBEnums.h"
#include "Types/TBProjectileId.h"
#include "Types/TBTraits.h"

#include "TBLaunchTypes.generated.h"

class UPackageMap;
class FArchive;


USTRUCT(BlueprintType, DisplayName = "LaunchParams", meta = (DisableSplitPin, HasNativeMake = "/Script/TerminalBallistics.TerminalBallisticsStatics.MakeLaunchParams", HasNativeBreak = "/Script/TerminalBallistics.TerminalBallisticsStatics.BreakLaunchParams"))
struct TERMINALBALLISTICS_API FTBLaunchParams
{
	GENERATED_BODY()
	TB_WITH_OPTIMIZED_SERIALIZER();
public:
	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (Units = "m/s"))
	/* Speed of the Projectile(m / s) */
	double ProjectileSpeed = 360.0;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (Units = "meters"))
	/**
	* Effective range (m)
	* This controls roughly how far the Projectile can travel.
	*/
	double EffectiveRange = 150.0;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (Min = "0", UIMin = "0", ToolTip = "Controls how quickly the simulation will run. Can be used for a \"Slow Motion\" effect. A value of zero or less is ignored."))
	double Timescale = 1.0;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (Units = "meters"))
	double GravityMultiplier = 1.0;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", AdvancedDisplay, meta = (Units = "meters"))
	/* OwnerIgnoreDistance	Owning actor will be ignored until the projectile has travelled this distance (m). Helps to prevent erroneous hits on the owner due to latency. */
	double OwnerIgnoreDistance = 10.0;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", AdvancedDisplay, meta = (Units = "meters"))
	double TracerActivationDistance = 25.0;
	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	/* Where the Projectile is being fired from, and which direction it is pointing. */
	FTBLocationAndDirection FireTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	TArray<TObjectPtr<AActor>> ToIgnore = {};

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = CollisionPresets::DefaultCollisionQueryTypes;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel10;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	uint8 IgnoreOwner : 1;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (ToolTip = "When true, the velocity of the projectile will be added to the velocity of the owning actor."))
	uint8 AddToOwnerVelocity : 1;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", meta = (ToolTip = "If true, no tracer will be used, even if one exists."))
	uint8 bForceNoTracer : 1;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	TObjectPtr<AActor> Owner = nullptr;
	UPROPERTY(BlueprintReadWrite, Category = "Launch Params")
	TObjectPtr<AController> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Launch Params", AdvancedDisplay)
	TObjectPtr<UObject> Payload = nullptr;

public:
	FTBLaunchParams()
		: IgnoreOwner(true)
		, AddToOwnerVelocity(true)
		, bForceNoTracer(false)
	{}

	FTBLaunchParams(
		TObjectPtr<AActor> InOwner,
		TObjectPtr<AController> InInstigator,
		const double InProjectileSpeed,
		const double InEffectiveRange,
		const FTBLocationAndDirection& InFireTransform,
		const TArray<AActor*> InToIgnore,
		const TArray<TEnumAsByte<EObjectTypeQuery>> InObjectTypes = CollisionPresets::DefaultCollisionQueryTypes,
		const TEnumAsByte<ECollisionChannel> InTraceChannel = ECC_GameTraceChannel10,
		const bool InIgnoreOwner = true,
		const bool InAddToOwnerVelocity = true,
		const bool InForceNoTracer = false,
		const double Timescale = 1.0,
		const double GravityMultiplier = 1.0,
		const double OwnerIgnoreDistance = 10.0,
		const double TracerActivationDistance = 50.0,
		TObjectPtr<UObject> Payload = nullptr
	)
		: ProjectileSpeed(InProjectileSpeed)
		, EffectiveRange(InEffectiveRange)
		, Timescale(Timescale)
		, GravityMultiplier(GravityMultiplier)
		, OwnerIgnoreDistance(OwnerIgnoreDistance)
		, TracerActivationDistance(TracerActivationDistance)
		, FireTransform(InFireTransform)
		, ToIgnore(InToIgnore)
		, ObjectTypes(InObjectTypes)
		, TraceChannel(InTraceChannel)
		, IgnoreOwner(InIgnoreOwner)
		, AddToOwnerVelocity(InAddToOwnerVelocity)
		, bForceNoTracer(InForceNoTracer)
		, Owner(InOwner)
		, Instigator(InInstigator)
		, Payload(Payload)
	{
		if (IgnoreOwner && !ToIgnore.Contains(Owner))
		{
			ToIgnore.Add(Owner);
		}
	}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	bool operator==(const FTBLaunchParams& other) const
	{
		return FMath::IsNearlyEqual(ProjectileSpeed, other.ProjectileSpeed, 1e-4)
			&& FMath::IsNearlyEqual(EffectiveRange, other.EffectiveRange, 1e-4)
			&& FMath::IsNearlyEqual(Timescale, other.Timescale, 1e-4)
			&& FMath::IsNearlyEqual(OwnerIgnoreDistance, other.OwnerIgnoreDistance, 1e-4)
			&& FMath::IsNearlyEqual(TracerActivationDistance, other.TracerActivationDistance, 1e-4)
			&& FireTransform == other.FireTransform
			&& ToIgnore == other.ToIgnore
			&& ObjectTypes == other.ObjectTypes
			&& TraceChannel == other.TraceChannel
			&& IgnoreOwner == other.IgnoreOwner
			&& AddToOwnerVelocity == other.AddToOwnerVelocity
			&& bForceNoTracer == other.bForceNoTracer
			&& Owner == other.Owner
			&& Instigator == other.Instigator
			&& Payload == other.Payload;
	}

	bool operator!=(const FTBLaunchParams& other) const
	{
		return !(*this == other);
	}
};
template<> struct TStructOpsTypeTraits<FTBLaunchParams> : public TStructOpsTypeTraitsBase2<FTBLaunchParams>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSerializer = true
	};
};


USTRUCT(BlueprintType, DisplayName = "LaunchData")
struct TERMINALBALLISTICS_API FTBLaunchData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Launch Data")
	FTBLaunchParams LaunchParams;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Launch Data")
	FTBProjectileId Id = FTBProjectileId::None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Launch Data", meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType"))
	int32 DebugType = 0;

	FTBLaunchData() = default;

	FTBLaunchData(const FTBLaunchParams& InLaunchParams, FTBProjectileId InId, int32 InDebugType)
		: LaunchParams(InLaunchParams)
		, Id(InId)
		, DebugType(InDebugType)
	{}
};