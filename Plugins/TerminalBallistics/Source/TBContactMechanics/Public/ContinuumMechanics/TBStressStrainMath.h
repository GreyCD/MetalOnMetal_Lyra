// Copyright Erik Hedberg. All rights reserved.

#pragma once

namespace TB::ContactMechanics::StressStrain
{
	extern TBCONTACTMECHANICS_API double GetCylinderLaterialSurfaceArea(const double Radius, const double Height);

	struct TBCONTACTMECHANICS_API FStresses
	{
		double AxialStress;
		double RadialStress;
		double HoopStress;

		FStresses() = default;
		FStresses(const double AxialStress, const double RadialStress, const double HoopStress)
			: AxialStress(AxialStress)
			, RadialStress(RadialStress)
			, HoopStress(HoopStress)
		{}
	};

	struct TBCONTACTMECHANICS_API FStressStrainPair
	{
		double Stress;
		double Strain;

		FStressStrainPair() = default;
		FStressStrainPair(const double Stress, const double Strain)
			: Stress(Stress)
			, Strain(Strain)
		{}
	};

	template<typename T = double>
	struct TBCONTACTMECHANICS_API TAxialRadialPair
	{
		T Axial;
		T Radial;

		TAxialRadialPair() = default;
		TAxialRadialPair(const T& Axial, const T& Radial)
			: Axial(Axial)
			, Radial(Radial)
		{}
	};

	typedef TAxialRadialPair<FStressStrainPair> FAxialRadialPair;

	struct TBCONTACTMECHANICS_API FStressCalculationParams
	{
		double OuterRadius;
		double OuterPressure;
		double OriginalCylinderRadius;
		double OriginalCylinderLength;
		double AxialForce;
		double Area;
		double YoungsModulus;
		double PoissonsRatio;
		double ElasticMaterial_YoungsModulus;
		double ElasticMaterial_PoissonsRatio;

		FStressCalculationParams() = default;
	};

	extern TBCONTACTMECHANICS_API double CalculateHoopStress(const double InnerPressure, const double OuterPressure, const double InnerRadius, const double OuterRadius, const double r);

	extern TBCONTACTMECHANICS_API double CalculateRadialStress(const double Pressure, const double InnerRadius, const double OuterRadius, const double r);

	extern TBCONTACTMECHANICS_API FStressStrainPair CalculateRadialFromAxial(const double AxialStrain, const double YoungsModulus, const double PoissonsRatio);

	extern TBCONTACTMECHANICS_API FAxialRadialPair CalculateAxialAndRadial(const double AxialForce, const double Area, const double YoungsModulus, const double PoissonsRatio);

	extern TBCONTACTMECHANICS_API double CalculateCavityRadius(const double OriginalCylinderRadius, const double OriginalCylinderLength, const double ElasticMaterial_YoungsModulus, const FStressStrainPair& RadialStressStrain);

	extern TBCONTACTMECHANICS_API FStresses CalculateStresses(const FStressCalculationParams& Params);
};