// Copyright Erik Hedberg. All Rights Reserved.

#include "Tests/TBTestingUtils.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/TBBullets.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "Misc/TBCollisionPresets.h"
#include "PhysMatManager/PhysMat.h"
#include "Types/TBBulletInfo.h"
#include "Types/TBBulletPhysicalProperties.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBLocationAndDirection.h"
#include "Types/TBProjectile.h"
#include "Types/TBProjectileFlightData.h"
#include "Types/TBProjectileId.h"
#include "Types/TBShapes.h"
#include "Types/TBSpreadParams.h"

#define TB_CATEGORY Terminal Ballistics
#define TB_CATEGORY_TYPES TB_CATEGORY.Types

namespace Helpers
{
	static double Rand()
	{
		return FMath::RandRange(0.1, 100.0);
	}

	static FTBRicochetProperties RandRicochetProperties()
	{
		FTBRicochetProperties Properties{};
		Properties.bAddRicochetRandomness = true;
		Properties.bAllowRicochets = true;
		Properties.bLimitRicochetAngle = true;
		Properties.bProbabilityOverridesMaterialCalculations = true;
		Properties.RicochetAngleCutoff = Rand();
		Properties.RicochetProbability = FMath::RandRange(0.1, 1.0);
		return Properties;
	}
	static auto RandPhysMat()
	{
		FPhysMatProperties Properties;
		if (UGameplayTagsManager::GetIfAllocated())
		{
			if (UGameplayTagsManager::Get().RequestGameplayTag(TB::PhysMatConstants::Names::PhysMatName_Steel, false).IsValid())
			{
				return TB::PhysMatConstants::DefaultPhysMats::Steel();
			}
		}
		return FPhysMatProperties(FGameplayTag(), EPhysicalSurface::SurfaceType11, "Text", Rand(), Rand(), Rand(), Rand(), Rand(), Rand(), Rand(), Rand(), Rand(), Rand(), true, true, true, RandRicochetProperties());
	}

	static auto RandCylinder()
	{
		return FTBCylinder(Rand(), Rand());
	}
	static auto RandOgive()
	{
		return FTBOgive(Rand(), Rand(), Rand(), Rand(), RandCylinder());
	}

	static auto RandProjectileId()
	{
		return FTBProjectileId::CreateNew();
	}
	static auto RandProjectilePhysicalProperties()
	{
		return FTBProjectilePhysicalProperties(Rand(), Rand(), Rand(), Rand(), Rand(), FMath::VRand().Rotation(), 0.5);
	}
	static FTBProjectile RandProjectile()
	{
		FTBProjectile Projectile = FTBProjectile(Rand(), Rand(), Rand(), Rand(), ETBProjectileSize::Large);
		Projectile.Orientation = FMath::VRand().ToOrientationQuat();
		Projectile.SetDragCoefficient(0.5);
		return Projectile;
	}
	static auto RandFlightData()
	{
		return FTBProjectileFlightData(FMath::VRand(), FMath::VRand(), FMath::VRand(), RandProjectileId());
	}

	static auto RandImpact()
	{
		return FTBImpact(FHitResult(), FMath::VRand(), EPhysicalSurface::SurfaceType2, FMath::VRand(), nullptr, true, true, Rand(), Rand(), RandProjectileId(), FMath::VRand());
	}
	static auto RandImpactParamsBasic()
	{
		return FTBImpactParamsBasic(FHitResult(), RandProjectile(), FMath::VRand(), true, EPhysicalSurface::SurfaceType2, FMath::VRand(), nullptr, true, RandProjectileId(), Rand(), FMath::VRand());
	}
	static auto RandImpactParams()
	{
		return FTBImpactParams(FHitResult(), TB::Bullets::BulletTypes::Get().Bullet_556x45NATO, FMath::VRand(), true, EPhysicalSurface::SurfaceType2, FMath::VRand(), nullptr, true, RandProjectileId(), Rand(), FMath::VRand());
	}

	static auto RandSpread()
	{
		return FTBSpreadParams(Rand(), Rand(), Rand(), FVector2D(Rand(), Rand()));
	}

	static auto RandLocationAndDirection()
	{
		return FTBLocationAndDirection(FMath::VRand(), FMath::VRand());
	}
	static auto RandLaunchParams()
	{
		return FTBLaunchParams(
			nullptr,
			nullptr,
			Rand(),
			Rand(),
			RandLocationAndDirection(),
			{},
			CollisionPresets::DefaultCollisionQueryTypesWithoutPawn,
			ECC_GameTraceChannel1,
			true,
			true,
			true,
			0.5,
			5.0
		);
	}
};

#pragma region PhysMat
IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Physical Materials,
	FTBRicochetProperties,
	Helpers::RandRicochetProperties()
);

IMPLEMENT_STRUCT_SERIALIZATION_TEST(
	TB_CATEGORY_TYPES.Physical Materials,
	FPhysMatProperties,
	Helpers::RandPhysMat(),
	EAutomationTestFlags::ProductFilter
);
#pragma endregion

#pragma region Shapes
IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Shapes,
	FTBCylinder,
	Helpers::RandCylinder()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Shapes,
	FTBOgive,
	Helpers::RandOgive()
);
#pragma endregion

#pragma region Projectiles
IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBProjectileId,
	Helpers::RandProjectileId()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBProjectilePhysicalProperties,
	Helpers::RandProjectilePhysicalProperties()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBProjectile,
	Helpers::RandProjectile()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBBulletInfo,
	FTBBulletInfo::GetDefaultBulletInfo()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBBulletPhysicalProperties,
	TB::Bullets::BulletPhysicalProperties::BULLET_556x45NATO
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBProjectileFlightData,
	Helpers::RandFlightData()
);

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Projectiles,
	FTBSpreadParams,
	Helpers::RandSpread()
);

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTBSpreadParamsTest, PREPROCESSOR_TO_STRING(TB_CATEGORY_TYPES.Projectiles.FTBSpreadParams.GenerateSpreadVector), EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FTBSpreadParamsTest::RunTest(const FString& Parameters)
{
	FTBSpreadParams Params = Helpers::RandSpread();
	Params.Variance = FVector2D(0.2);
	const auto SpreadAngle = FMath::RadiansToDegrees(Params.GetSpreadAngle() + Params.GetVarianceAngle());
	const auto MaxHalfAngle = SpreadAngle / 2.0;
	
	int i = 0;
	const int num_tests = 500;
	FVector dir;

	bool bPassed = true;
	while (i < num_tests)
	{
		i++;
		dir = Params.GenerateSpreadVector(FVector::ForwardVector);//TB::MathUtils::VectorUtils::VRandConeGuassian(FVector::ForwardVector, SpreadAngle.X / 2.0, SpreadAngle.Y / 2.0);
		FRotator rot = dir.ToOrientationRotator();
		const bool localPassed = TestTrue("Pitch is in range", abs(rot.Pitch) <= MaxHalfAngle.Y) && TestTrue("Yaw is in range", abs(rot.Yaw) <= MaxHalfAngle.X);
		bPassed &= localPassed;
		if (!localPassed)
		{
			AddInfo(FString::Printf(TEXT("DeltaYaw: %f\nDeltaPitch: %f\nYaw: %f / %f\nPitch: %f / %f"), MaxHalfAngle.X - abs(rot.Yaw), MaxHalfAngle.Y - abs(rot.Pitch), abs(rot.Yaw), MaxHalfAngle.X, abs(rot.Pitch), MaxHalfAngle.Y));
		}
	}
	return bPassed;
}
#pragma endregion

#pragma region Impact Params
IMPLEMENT_STRUCT_SERIALIZATION_TEST(
	TB_CATEGORY_TYPES.ImpactParams,
	FTBImpact,
	Helpers::RandImpact(),
	EAutomationTestFlags::ProductFilter
);

IMPLEMENT_STRUCT_SERIALIZATION_TEST(
	TB_CATEGORY_TYPES.ImpactParams,
	FTBImpactParamsBasic,
	Helpers::RandImpactParamsBasic(),
	EAutomationTestFlags::ProductFilter
);

IMPLEMENT_STRUCT_SERIALIZATION_TEST(
	TB_CATEGORY_TYPES.ImpactParams,
	FTBImpactParams,
	Helpers::RandImpactParams(),
	EAutomationTestFlags::ProductFilter
);
#pragma endregion

IMPLEMENT_STRUCT_SERIALIZATION_SMOKE_TEST(
	TB_CATEGORY_TYPES.Misc,
	FTBLocationAndDirection,
	Helpers::RandLocationAndDirection()
);

IMPLEMENT_STRUCT_SERIALIZATION_TEST(
	TB_CATEGORY_TYPES.Misc,
	FTBLaunchParams,
	Helpers::RandLaunchParams(),
	EAutomationTestFlags::ProductFilter
);
#pragma endregion

#endif

