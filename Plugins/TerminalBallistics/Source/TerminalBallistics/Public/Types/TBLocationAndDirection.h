// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/TBMacrosAndFunctions.h"
#include "Templates/UnrealTypeTraits.h"
#include "Types/TBTraits.h"

#include "TBLocationAndDirection.generated.h"

USTRUCT(BlueprintType, DisplayName = "LocationAndDirection", meta = (DisableSplitPin, HasNativeMake = "/Script/TerminalBallistics.TerminalBallisticsStatics.MakeLocationAndDirection", HasNativeBreak = "/Script/TerminalBallistics.TerminalBallisticsStatics.BreakLocationAndDirection"))
struct TERMINALBALLISTICS_API FTBLocationAndDirection
{
	GENERATED_BODY()
	TB_WITH_OPTIMIZED_SERIALIZER();
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location and Direction")
	FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location and Direction")
	FVector_NetQuantizeNormal Direction = FVector_NetQuantizeNormal::ZeroVector;
	
	FTBLocationAndDirection() = default;

	template<typename Vec1, typename Vec2, TEMPLATE_REQUIRES(TAnd<TB::Traits::TIsFVector<Vec1>, TB::Traits::TIsFVector<Vec2>>::Value)>
	FTBLocationAndDirection(const Vec1& InLocation, const Vec2& InDirection)
		: Location(InLocation)
		, Direction(InDirection)
	{}

	template<typename Vec, TEMPLATE_REQUIRES(TB::Traits::TIsFVector<Vec>::Value)>
	FTBLocationAndDirection(const Vec& InLocation, const FRotator& Rotation)
		: Location(InLocation)
		, Direction(Rotation.Quaternion().GetForwardVector())
	{}

	template<typename Vec, TEMPLATE_REQUIRES(TB::Traits::TIsFVector<Vec>::Value)>
	FTBLocationAndDirection(const Vec& InLocation, const FQuat& Rotation)
		: Location(InLocation)
		, Direction(Rotation.GetForwardVector())
	{}

	FTBLocationAndDirection(const FTransform& Transform)
		: Location(Transform.GetLocation())
		, Direction(Transform.GetRotation().GetForwardVector())
	{}

	inline FRotator GetDirectionAsRotator() const
	{
		return Direction.ToOrientationRotator();
	}

	inline FQuat GetDirectionAsQuat() const
	{
		return Direction.ToOrientationQuat();
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	friend FArchive& operator<<(FArchive& Ar, FTBLocationAndDirection& LocationAndDirection);

	bool operator==(const FTBLocationAndDirection& other) const
	{
		return Location.Equals(other.Location, 1e-2)
			&& Direction.Equals(other.Direction, 1e-2);
	}

	bool operator!=(const FTBLocationAndDirection& other) const
	{
		return !(*this == other);
	}

	template<typename FReal>
	operator UE::Math::TTransform<FReal>()
	{
		return UE::Math::TTransform<FReal>(GetDirectionAsQuat(), Location);
	}
};
template<> struct TIsPODType<FTBLocationAndDirection> { enum { Value = true }; };
template<> struct TStructOpsTypeTraits<FTBLocationAndDirection> : public TStructOpsTypeTraitsBase2<FTBLocationAndDirection>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSerializer = true,
		WithNetSharedSerialization = true
	};
};
