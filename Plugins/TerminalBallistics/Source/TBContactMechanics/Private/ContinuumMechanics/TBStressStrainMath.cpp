// Copyright Erik Hedberg. All rights reserved.

#include "ContinuumMechanics/TBStressStrainMath.h"

#include "Math/UnrealMathUtility.h"

namespace TB::ContactMechanics::StressStrain
{
	static constexpr double sqr(const double x) { return x * x; }

	double GetCylinderLaterialSurfaceArea(const double Radius, const double Height)
	{
		return 2.0 * UE_DOUBLE_PI * Radius * Height;
	}

	double CalculateHoopStress(const double InnerPressure, const double OuterPressure, const double InnerRadius, const double OuterRadius, const double r)
	{
		const double OuterSqrMinusInnerSqr = sqr(OuterRadius) - sqr(InnerRadius);
		const double term1 = (InnerPressure * sqr(InnerRadius) - OuterPressure * sqr(OuterRadius)) / OuterSqrMinusInnerSqr;
		const double term2 = (sqr(InnerPressure) * sqr(OuterPressure) * (OuterRadius - InnerRadius)) / (sqr(r) * OuterSqrMinusInnerSqr);
		return term1 + term2;
	}

	double CalculateRadialStress(const double Pressure, const double InnerRadius, const double OuterRadius, const double r)
	{
		return (sqr(InnerRadius) * Pressure) / (sqr(OuterRadius) - sqr(InnerRadius)) - (sqr(OuterRadius) / sqr(r));
	}

	FStressStrainPair CalculateRadialFromAxial(const double AxialStrain, const double YoungsModulus, const double PoissonsRatio)
	{
		const double TransverseStrain = -PoissonsRatio * AxialStrain;
		const double TransverseStress = TransverseStrain * YoungsModulus;
		return FStressStrainPair(TransverseStress, TransverseStrain);
	}

	FAxialRadialPair CalculateAxialAndRadial(const double AxialForce, const double Area, const double YoungsModulus, const double PoissonsRatio)
	{
		const double AxialStress = AxialForce / Area;
		const double AxialStrain = AxialStress / YoungsModulus;
		const double TransverseStrain = -PoissonsRatio * AxialStrain;
		const double TransverseStress = TransverseStrain * YoungsModulus;

		const FStressStrainPair Axial = FStressStrainPair(AxialStress, AxialStrain);
		const FStressStrainPair Radial = FStressStrainPair(TransverseStress, TransverseStrain);
		return FAxialRadialPair(Axial, Radial);
	}

	double CalculateCavityRadius(const double OriginalCylinderRadius, const double OriginalCylinderLength, const double ElasticMaterial_YoungsModulus, const FStressStrainPair& RadialStressStrain)
	{
		const double ElasticMaterial_RadialStrain = RadialStressStrain.Stress / ElasticMaterial_YoungsModulus;
		const double DeltaR_ElasticMaterial = ElasticMaterial_RadialStrain * OriginalCylinderRadius;

		return OriginalCylinderRadius + abs(DeltaR_ElasticMaterial);
	}

	FStresses CalculateStresses(const FStressCalculationParams& Params)
	{
		const FAxialRadialPair AxialRadialPair = CalculateAxialAndRadial(Params.AxialForce, Params.Area, Params.YoungsModulus, Params.PoissonsRatio);
		const auto Axial = AxialRadialPair.Axial;
		const auto Radial = AxialRadialPair.Radial;

		const double CavityRadius = CalculateCavityRadius(Params.OriginalCylinderRadius, Params.OriginalCylinderLength, Params.ElasticMaterial_YoungsModulus, Radial);

		const double ContactArea = GetCylinderLaterialSurfaceArea(CavityRadius, Params.OriginalCylinderLength - Axial.Strain * Params.OriginalCylinderLength);

		const double InnerPressure = Radial.Stress * ContactArea;

		const double HoopStress = CalculateHoopStress(InnerPressure, Params.OuterPressure, CavityRadius, Params.OuterRadius, CavityRadius);
		const double AxialStress = Axial.Stress + (HoopStress / 2.0);

		return FStresses(AxialStress, Radial.Stress, HoopStress);
	}
};

