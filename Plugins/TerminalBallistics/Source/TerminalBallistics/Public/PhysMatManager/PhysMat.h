// Copyright Erik Hedberg. All Rights Reserved.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Misc/Crc.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Serialization/Archive.h"
#include "Types/TBEnums.h"
#include "Types/TBTraits.h"
#include "UObject/CoreNet.h"

#include "PhysMat.generated.h"

class UTBPhysicalMaterialWithTags;
class UTBConfiguration;

USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FTBRicochetProperties
{
	GENERATED_BODY()
	TB_WITH_OPTIMIZED_SERIALIZER();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides)
	uint8 bAllowRicochets : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides, meta = (InlineEditConditionToggle, EditCondition = "bAllowRicochets", EditConditionHides))
	uint8 bLimitRicochetAngle : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides, meta = (InlineEditConditionToggle, EditCondition = "bAllowRicochets", EditConditionHides))
	uint8 bAddRicochetRandomness : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides, meta = (EditCondition = "bAddRicochetRandomness & bAllowRicochets", ToolTip = "If true, RicochetProbability will be responsible for determining the chance of a ricochet.\nNo material calculations will be used."))
	uint8 bProbabilityOverridesMaterialCalculations : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides, meta = (EditCondition = "bLimitRicochetAngle", Units = "deg", Min = "0", UIMin = "0", Max = "90", UIMax = "90", ToolTip = "Impacts above this angle will never result in a ricochet.\n(An angle of 90 degrees indicates a head on impact)"))
	float RicochetAngleCutoff = 80.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RicochetOverrides, meta = (EditCondition = "bAddRicochetRandomness", Min = "0", UIMin = "0", Max = "1", UIMax = "1", ToolTip = "Controls the likelihood of a ricochet"))
	float RicochetProbability = 1.f;

	FTBRicochetProperties()
		: bAllowRicochets(true)
		, bLimitRicochetAngle(false)
		, bAddRicochetRandomness(false)
		, bProbabilityOverridesMaterialCalculations(false)
	{}

	FTBRicochetProperties(const bool bAllowRicochets, const bool bLimitRicochetAngle, const bool bAddRicochetRandomness, const bool bProbabilityOverridesMaterialCalculations, const float RicochetAngleCutoff, const float RicochetProbability)
		: bAllowRicochets(bAllowRicochets)
		, bLimitRicochetAngle(bLimitRicochetAngle)
		, bAddRicochetRandomness(bAddRicochetRandomness)
		, bProbabilityOverridesMaterialCalculations(bProbabilityOverridesMaterialCalculations)
		, RicochetAngleCutoff(RicochetAngleCutoff)
		, RicochetProbability(RicochetProbability)
	{}

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		Ar << *this;
		bOutSuccess = true;
		return true;
	}

	friend FArchive& operator<<(FArchive& Ar, FTBRicochetProperties& RicochetOverrides);

	bool operator==(const FTBRicochetProperties& other) const
	{
		return RicochetAngleCutoff == other.RicochetAngleCutoff
			&& RicochetProbability == other.RicochetProbability
			&& bAllowRicochets == other.bAllowRicochets
			&& bLimitRicochetAngle == other.bLimitRicochetAngle
			&& bAddRicochetRandomness == other.bAddRicochetRandomness
			&& bProbabilityOverridesMaterialCalculations == other.bProbabilityOverridesMaterialCalculations;
	}

	bool operator!=(const FTBRicochetProperties& other) const
	{
		return !(*this == other);
	}
};
template<> struct TStructOpsTypeTraits<FTBRicochetProperties> : TStructOpsTypeTraitsBase2<FTBRicochetProperties>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSerializer = true,
		WithNetSharedSerialization = true
	};
};


UENUM(BlueprintType)
enum class ETBPenetrationRequirement : uint8
{
	None = 0	UMETA(Hidden),
	PenetrationResistanceThreshold,
	EnergyThreshold,
	Both
};

USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FTBSimplePhysMatProperties
{
	GENERATED_BODY()
public:
	// Resistance to penetration. Stops penetration from pro
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimplyPhysMatProperties", meta = (EditCondition = "PenetrationRequirement != ETBPenetrationRequirement::EnergyThreshold", EditConditionHides))
	float PenetrationResistance = 1.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimplyPhysMatProperties")
	ETBPenetrationRequirement PenetrationRequirement = ETBPenetrationRequirement::Both;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimplyPhysMatProperties")
	// Energy loss per centimeter (Joules)
	double EnergyLossPerCm = 100.0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimplyPhysMatProperties", meta = (EditCondition = "PenetrationRequirement != ETBPenetrationRequirement::PenetrationResistanceThreshold", EditConditionHides))
	// Energy required for penetration to occur (Joules)
	double EnergyRequiredToPenetrate = 100.0;

public:
	FTBSimplePhysMatProperties() = default;

	FTBSimplePhysMatProperties(double PenetrationResistance, double EnergyLossPerCm, double EnergyRequiredToPenetrate, ETBPenetrationRequirement PenetrationRequirement)
		: PenetrationResistance(PenetrationResistance)
		, PenetrationRequirement(PenetrationRequirement)
		, EnergyLossPerCm(EnergyLossPerCm)
		, EnergyRequiredToPenetrate(EnergyRequiredToPenetrate)
	{}

	friend FArchive& operator<<(FArchive& Ar, FTBSimplePhysMatProperties& PhysMatProperties);

	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;
		Ar << *this;
		return bOutSuccess;
	}

	bool operator==(const FTBSimplePhysMatProperties& Other) const
	{
		return PenetrationResistance == Other.PenetrationResistance
			&& PenetrationRequirement == Other.PenetrationRequirement
			&& EnergyLossPerCm == Other.EnergyLossPerCm
			&& EnergyRequiredToPenetrate == Other.EnergyRequiredToPenetrate;
	}
};
template<> struct TStructOpsTypeTraits<FTBSimplePhysMatProperties> : TStructOpsTypeTraitsBase2<FTBSimplePhysMatProperties>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSerializer = true,
		WithNetSharedSerialization = true
	};
};


// TODO: For materials with undefined or not well defined properties, make alternate solutions (ex: water)

USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FPhysMatProperties : public FTableRowBase
{
	GENERATED_BODY()
	TB_WITH_OPTIMIZED_SERIALIZER();
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Material Name"))
	FName MaterialName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (Categories = "PhysMat"))
	FGameplayTag GameplayTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Density"))
	// g/cm^3
	float Density = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Fracture Toughness"))
	// MPa-m^0.5
	float FractureToughness = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Ultimate Tensile Strength"))
	// MPa
	float UltimateTensileStrength = 0.f;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Yield Strength"))
	// MPa
	mutable float YieldStrength = -1.f;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Compressive Strength"))
	// MPa
	float CompressiveStrength = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Shear Strength"))
	// MPa
	float ShearStrength = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Poisson's Ratio"))
	float PoissonsRatio = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Young's Modulus"))
	// GPa
	float YoungsModulus = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Coefficient of Friction (Dynamic)", ClampMin = "0", UIMin = "0"))
	float CoefficientOfFriction = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayName = "Viscosity"))
	// m^2*s^-1
	float Viscosity = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (AdvancedDisplay))
	float StrainRateFactor = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties")
	float PenetrationResistanceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (EditCondition = "bUseCustomRicochetProperties", EditConditionHides))
	FTBRicochetProperties RicochetProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (DisplayPriority = 1))
	TEnumAsByte<EPhysicalSurface> SurfaceType = SurfaceType_Default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties")
	uint8 bIsFluid : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties")
	uint8 bIsParticulate : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties")
	uint8 bUseCustomRicochetProperties : 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (InlineEditConditionToggle))
	uint8 bOverridePenetrationComplexity : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (EditCondition = "bOverridePenetrationComplexity"))
	ETBPenetrationComplexity PenetrationComplexity = ETBPenetrationComplexity::Complex;

private:
	friend class UTBEngineSubsystem;
	friend class UTBConfiguration;
	static FPhysMatProperties DefaultPhysMat;
	uint8 bIsInBaseUnits : 1;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysMatProperties", meta = (ToolTip = "Simplified properties to use when penetration complexity is set to \"Simplified\"", EditCondition = "PenetrationComplexity == ETBPenetrationComplexity::Simplified"))
	FTBSimplePhysMatProperties SimpleProperties;

	bool ShouldUseSimpleProperties() const;

	FPhysMatProperties()
		: bIsFluid(false)
		, bIsParticulate(false)
		, bUseCustomRicochetProperties(false)
		, bOverridePenetrationComplexity(false)
		, bIsInBaseUnits(false)
	{}

	FPhysMatProperties(
		EPhysicalSurface InSurfaceType,
		const FName& MaterialName,
		const float Density,
		const float FractureToughness,
		const float UltimateTensileStrength,
		const float PoissonsRatio,
		const float YoungsModulus,
		const float Viscosity,
		const bool bIsFluid,
		const bool bIsParticulate,
		const FGameplayTag& GameplayTag,
		const bool bOverridePenetrationComplexity,
		const ETBPenetrationComplexity PenetrationComplexity,
		const FTBSimplePhysMatProperties& SimpleProperties
	)
		: MaterialName(MaterialName)
		, GameplayTag(GameplayTag)
		, Density(Density)
		, FractureToughness(FractureToughness)
		, UltimateTensileStrength(UltimateTensileStrength)
		, PoissonsRatio(PoissonsRatio)
		, YoungsModulus(YoungsModulus)
		, Viscosity(Viscosity)
		, SurfaceType(InSurfaceType)
		, bIsFluid(bIsFluid)
		, bIsParticulate(bIsParticulate)
		, bOverridePenetrationComplexity(bOverridePenetrationComplexity)
		, PenetrationComplexity(PenetrationComplexity)
		, bIsInBaseUnits(false)
		, SimpleProperties(SimpleProperties)
	{
		GetYieldStrength();
	}

	FPhysMatProperties(
		const FGameplayTag& Tag,
		const EPhysicalSurface InSurfaceType,
		const FName& MaterialName,
		const float Density,
		const float FractureToughness,
		const float UltimateTensileStrength,
		const float YieldStrength,
		const float CompressiveStrength,
		const float ShearStrength,
		const float PoissonsRatio,
		const float YoungsModulus,
		const float Viscosity,
		const float StrainRateFactor = 1.5,
		const bool bIsFluid = false,
		const bool bIsParticulate = false,
		const bool bUseCustomRicochetProperties = false,
		const FTBRicochetProperties RicochetProperties = FTBRicochetProperties(),
		const bool bIsInBaseUnits = false,
		const bool bOverridePenetrationComplexity = false,
		const ETBPenetrationComplexity PenetrationComplexity = ETBPenetrationComplexity::Complex,
		const FTBSimplePhysMatProperties& SimpleProperties = FTBSimplePhysMatProperties()
	)
		: MaterialName(MaterialName)
		, GameplayTag(Tag)
		, Density(Density)
		, FractureToughness(FractureToughness)
		, UltimateTensileStrength(UltimateTensileStrength)
		, YieldStrength(YieldStrength)
		, CompressiveStrength(CompressiveStrength)
		, ShearStrength(ShearStrength)
		, PoissonsRatio(PoissonsRatio)
		, YoungsModulus(YoungsModulus)
		, Viscosity(Viscosity)
		, StrainRateFactor(StrainRateFactor)
		, RicochetProperties(RicochetProperties)
		, SurfaceType(InSurfaceType)
		, bIsFluid(bIsFluid)
		, bIsParticulate(bIsParticulate)
		, bUseCustomRicochetProperties(bUseCustomRicochetProperties)
		, bOverridePenetrationComplexity(bOverridePenetrationComplexity)
		, PenetrationComplexity(PenetrationComplexity)
		, bIsInBaseUnits(bIsInBaseUnits)
		, SimpleProperties(SimpleProperties)
	{
		GetYieldStrength();
	}

	FString ToString() const
	{
		return MaterialName.ToString();
	}

	inline float GetYieldStrength(const bool bForceRecalculate = false) const
	{
		if (bForceRecalculate || YieldStrength <= 0.f)
		{
			CalculateYieldStrength();
		}
		return YieldStrength;
	}

	float EstimateCoeffientOfRestitution() const;

	/* Estimates the fraction of the total strain where plastic deformation begins to occur. */
	double EstimateK() const;

	inline bool CheckValid() const
	{
		return this->MaterialName != NAME_None;
	}

	bool CanEverHaveRicochet() const;

	/* Projectile impacts above this angle are more likely to penetrate than ricochet */
	double GetCriticalRicochetAngle() const;

	/* Rough estimate for the maximum amount of energy that we could transfer during this ricochet before material failure */
	double EstimateMaxImpartedRicochetEnergy(const double ImpactArea, const double ObjectThickness = 1.0) const;

	/*
	If bOverrideRicochet and bAddRicochetRandomness are set to true, decides whether or not a ricochet can occur based on the RicochetProbability.
	Returns true otherwise.
	*/
	inline bool CanRicochet() const;

	inline bool IsInBaseUnits() const
	{
		return bIsInBaseUnits;
	}
	// Note: This does not alter the values of this material, it only changes the variable "bIsInBaseUnits"
	inline void SetIsInBaseUnits(const bool NewIsInBaseUnits)
	{
		bIsInBaseUnits = NewIsInBaseUnits;
	}
	void ConvertToBaseUnits();
	void ConvertToDefaultUnits();
	FPhysMatProperties InBaseUnits() const;
	FPhysMatProperties InDefaultUnits() const;

	static const FPhysMatProperties GetDefault();

	static const FPhysMatProperties GetFromName(const FName& Name);
	static const FPhysMatProperties GetFromName(const FString& Name);
	static const FPhysMatProperties GetFromName(const FName& Name, bool& Success);
	static const FPhysMatProperties GetFromName(const FString& Name, bool& Success);

	static const FPhysMatProperties GetFromTag(const FGameplayTag& Tag);
	static const FPhysMatProperties GetFromTag(const FGameplayTag& Tag, bool& Success);

	static const FPhysMatProperties GetFromTagInterfaceMaterial(TWeakObjectPtr<UPhysicalMaterial> Material, bool& Success);

	static const FPhysMatProperties GetFromPhysMatWithTags(TObjectPtr<UTBPhysicalMaterialWithTags> Material, bool& Success);

	static const bool GetFromPhysicalMaterial(TWeakObjectPtr<UPhysicalMaterial> Material, FPhysMatProperties& OutProperties);

	static const FPhysMatProperties GetFromSurfaceType(const EPhysicalSurface SurfaceType);
	static const FPhysMatProperties GetFromSurfaceType(const EPhysicalSurface SurfaceType, bool& Success);
	static const FPhysMatProperties GetFromSurfaceType(const TEnumAsByte<EPhysicalSurface> SurfaceType);
	static const FPhysMatProperties GetFromSurfaceType(const TEnumAsByte<EPhysicalSurface> SurfaceType, bool& Success);


	static const double CalculateStrainRate(const double ImpactVelocity, const double CrackLength);
	double CalculateStrainEnergy(const double Stress, const double Volume) const;
	double CalculateStrainEnergyReleaseRate(const double ImpactVelocity, const double CrackLength) const;
	double CalculateEnergyRequiredToPropagateFracture(const double ImpactVelocity, const double CrackArea, const double Depth) const;
	double GetCriticalEnergyReleaseRate(const bool bIsPlaneStress = true) const;


	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	bool HasSameMaterialProperties(const FPhysMatProperties& other) const
	{
		return FractureToughness == other.FractureToughness
			&& UltimateTensileStrength == other.UltimateTensileStrength
			&& YieldStrength == other.YieldStrength
			&& CompressiveStrength == other.CompressiveStrength
			&& ShearStrength == other.ShearStrength
			&& PoissonsRatio == other.PoissonsRatio
			&& YoungsModulus == other.YoungsModulus
			&& Viscosity == other.Viscosity
			&& bIsFluid == other.bIsFluid
			&& bIsParticulate == other.bIsParticulate;
	}

	bool operator==(const FPhysMatProperties& other) const
	{
		return MaterialName == other.MaterialName
			&& GameplayTag == other.GameplayTag
			&& SurfaceType == other.SurfaceType
			&& bUseCustomRicochetProperties == other.bUseCustomRicochetProperties
			&& RicochetProperties == other.RicochetProperties
			&& bOverridePenetrationComplexity == other.bOverridePenetrationComplexity
			&& PenetrationComplexity == other.PenetrationComplexity
			&& SimpleProperties == other.SimpleProperties;
	}

	bool operator==(const FGameplayTag& Tag) const
	{
		return GameplayTag == Tag;
	}

	bool operator!=(const FPhysMatProperties& other) const
	{
		return !(*this == other);
	}

private:
	inline void CalculateYieldStrength() const
	{
		YieldStrength = UltimateTensileStrength / (1.f + PoissonsRatio / 3.f); // Assumes linear-elastic stress-strain up to yield point. (Hooke's law)
	}
};
template<> struct TStructOpsTypeTraits<FPhysMatProperties> : public TStructOpsTypeTraitsBase2<FPhysMatProperties>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithNetSerializer = true,
		WithNetSharedSerialization = true
	};
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FPhysMatProperties& PhysMat);
#else
FORCEINLINE uint32 GetTypeHash(const FPhysMatProperties& PhysMat)
{
	const uint32 Hash = FCrc::MemCrc32(&PhysMat, sizeof(FPhysMatProperties));
	return Hash;
}
#endif

class UMaterialInterface;
class UTBEngineSubsystem;
namespace TB
{
	namespace PhysMatConstants
	{
		namespace Names
		{
			extern TERMINALBALLISTICS_API const FName PhysMatName_Acrylic;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Aluminum;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Asphalt;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Bone;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Brass;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Brick;
			extern TERMINALBALLISTICS_API const FName PhysMatName_CarbonFiber;
			extern TERMINALBALLISTICS_API const FName PhysMatName_CastIron;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Ceramic;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Concrete;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Copper;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Drywall;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Flesh;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Glass;
			extern TERMINALBALLISTICS_API const FName PhysMatName_HardenedSteel;
			extern TERMINALBALLISTICS_API const FName PhysMatName_HardPlastic;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Hardwood;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Ice;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Kevlar;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Lead;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Limestone;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Paper;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Plastic;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Polystyrene;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Rubber;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Softwood;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Steel;
			extern TERMINALBALLISTICS_API const FName PhysMatName_Titanium;
			extern TERMINALBALLISTICS_API const FName PhysMatName_TungstenCarbide;

			static TArray<FName> GetNames()
			{
				return {
					PhysMatName_Acrylic,
					PhysMatName_Aluminum,
					PhysMatName_Asphalt,
					PhysMatName_Bone,
					PhysMatName_Brass,
					PhysMatName_Brick,
					PhysMatName_CarbonFiber,
					PhysMatName_CastIron,
					PhysMatName_Ceramic,
					PhysMatName_Concrete,
					PhysMatName_Copper,
					PhysMatName_Drywall,
					PhysMatName_Flesh,
					PhysMatName_Glass,
					PhysMatName_HardenedSteel,
					PhysMatName_HardPlastic,
					PhysMatName_Hardwood,
					PhysMatName_Ice,
					PhysMatName_Kevlar,
					PhysMatName_Lead,
					PhysMatName_Limestone,
					PhysMatName_Paper,
					PhysMatName_Plastic,
					PhysMatName_Polystyrene,
					PhysMatName_Rubber,
					PhysMatName_Softwood,
					PhysMatName_Steel,
					PhysMatName_Titanium,
					PhysMatName_TungstenCarbide
				};
			}
		};

		extern TERMINALBALLISTICS_API const FGameplayTag PhysMatTag();

		namespace DefaultPhysMats
		{
			/* Make sure to update "NumPhysMats" when adding new defaults. */

			const FPhysMatProperties Acrylic();
			const FPhysMatProperties Aluminum();
			const FPhysMatProperties Asphalt();
			const FPhysMatProperties Bone();
			const FPhysMatProperties Brass();
			const FPhysMatProperties Brick();
			const FPhysMatProperties CarbonFiber();
			const FPhysMatProperties CastIron();
			const FPhysMatProperties Ceramic();
			const FPhysMatProperties Concrete();
			const FPhysMatProperties Copper();
			const FPhysMatProperties Drywall();
			const FPhysMatProperties Flesh();
			const FPhysMatProperties Glass();
			const FPhysMatProperties HardenedSteel();
			const FPhysMatProperties HardPlastic();
			const FPhysMatProperties Hardwood();
			const FPhysMatProperties Ice();
			const FPhysMatProperties Kevlar();
			const FPhysMatProperties Lead();
			const FPhysMatProperties Limestone();
			const FPhysMatProperties Paper();
			const FPhysMatProperties Plastic();
			const FPhysMatProperties Polystyrene();
			const FPhysMatProperties Rubber();
			const FPhysMatProperties Softwood();
			const FPhysMatProperties Steel();
			const FPhysMatProperties Titanium();
			const FPhysMatProperties TungstenCarbide();

			TArray<FPhysMatProperties> GetDefaults();

			FPhysMatProperties* GetDefaultMaterialByName(const FName& Name);

			TSoftObjectPtr<UPhysicalMaterial> GetDefaultPhysicalMaterialAssetForPhysMat(const FPhysMatProperties& PhysMat);
		};

		static bool bUsePresetMaterialProperties = true;
		constexpr int NumPhysMats = 29;
	};

	namespace PhysMatHelpers
	{
		UMaterialInterface* GetMaterialFromStaticMesh(const UStaticMeshComponent* StaticMeshComponent, const int32 MaterialIndex);

		bool IsFlesh(const FPhysMatProperties& PhysMat);
		bool IsBone(const FPhysMatProperties& PhysMat);

		bool IsFleshOrBone(const FPhysMatProperties& PhysMat);
		bool IsFleshOrBone(const EPhysicalSurface SurfaceType);

		FVector GetAdjustedVelocity(const FVector& Velocity, const FVector& SurfaceNormal, const FPhysMatProperties& ImpactedObjectProperties);
	};
};
