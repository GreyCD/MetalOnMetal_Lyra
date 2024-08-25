// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "Containers/EnumAsByte.h"
#include "Math/MathFwd.h"

struct FTBBullet;
typedef TSharedPtr<FTBBullet> BulletPointer;

struct FPhysMatProperties;
struct FHitResult;
struct FTBProjectilePhysicalProperties;

namespace TB
{
	typedef FVector BulletPenFunc(
		const FHitResult& HitResult,
		BulletPointer& ActualBullet,
		const FVector& ImpactVelocity,
		const double PenetrationThickness,
		const FPhysMatProperties& ObjectProperties,
		bool& bIsZero,
		double& dE,
		double& DepthOfPenetration,
		const double PenetrationMultiplier /* = 1.0 */,
		bool bDebugPrint /* = false */
	);

	typedef FVector LegacyPenFunc(
		const FHitResult& HitResult,
		const FTBProjectilePhysicalProperties& Projectile,
		const FVector& ImpactVelocity,
		const double& PenetrationThickness,
		const TEnumAsByte<EPhysicalSurface>& SurfaceType,
		const FPhysMatProperties& ProjectilePhysicalMaterial,
		bool& bIsZero,
		double& dE,
		double& DepthOfPenetration,
		const double PenetrationMultiplier /* = 1.0 */,
		bool bDebugPrint /* = false */
	);
}
