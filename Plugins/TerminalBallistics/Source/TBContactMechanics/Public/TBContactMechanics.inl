// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include <ThirdParty/gcem/gcem.hpp>
#include "GenericPlatform/GenericPlatformMath.h"
#include "Math/UnrealMathUtility.h"
#include "Types/TBTensor.inl"

namespace TB::ContactMechanics
{
	template<typename T>
	static constexpr T CubeRoot(T x)
	{
		const T OneThird = (T)1.0 / (T)3.0;
		return pow(x, OneThird);
	}
	template<typename T>
	static constexpr T CubeRootConst(T x)
	{
		const T OneThird = (T)1.0 / (T)3.0;
		return gcem::pow(x, OneThird);
	}

	/* Based on "Exact Solutions of Axisymmetric Contact Problems" by Valentin L. Popov, Markus Heﬂ, and Emanuel Willert */

	class ElasticHalfSpace
	{
		const double PoissonsRatio;
		const double YoungsModulus;
	public:
		ElasticHalfSpace(const double PoissonsRatio, const double YoungsModulus)
			: PoissonsRatio(PoissonsRatio)
			, YoungsModulus(YoungsModulus)
		{}

		inline double CalculateEffectiveElasticModulus(const ElasticHalfSpace& Other) const;
		inline double CalculateEffectiveElasticModulus(const double OtherPoissonsRatio, const double OtherYoungsModulus) const;
	};

	class Cylinder : public ElasticHalfSpace
	{
		const double Radius;
	public:
		Cylinder(const double InPoissonsRatio, const double InYoungsModulus, const double InRadius)
			: ElasticHalfSpace(InPoissonsRatio, InYoungsModulus)
			, Radius(InRadius)
		{}

		inline double ForceFromDepth(const double E, const double Depth) const;

		inline double DepthFromForce(const double E, const double Force) const;

		inline double GetAverageStress(const double E, const double Depth) const;

		inline double GetStressDistribution(const double E, const double Depth, const double r) const;

		inline double GetDeformationDistribution(const double Depth, const double r) const;
	};

	class Cone : public ElasticHalfSpace
	{
		const double OutsideAngleRadians;
	public:
		Cone(const double InPoissonsRatio, const double InYoungsModulus, const double HalfAngleDegrees)
			: ElasticHalfSpace(InPoissonsRatio, InYoungsModulus)
			, OutsideAngleRadians(FMath::DegreesToRadians(90.0 - HalfAngleDegrees))
		{}

		inline double Profile(const double x) const;

		inline double GetContactAreaRadius(const double Depth) const;

		inline double DepthFromContactAreaRadius(const double ContactAreaRadius) const;

		inline double ForceFromDepth(const double E, const double Depth) const;

		inline double GetAverageStress(const double E) const;

		inline double GetStressDistribution(const double E, const double Depth, const double r) const;

		inline double GetDeformationDistribution(const double Depth, const double r) const;
	};

	class Parabaloid : public ElasticHalfSpace
	{
	public:
		double R;
		Parabaloid(const double InPoissonsRatio, const double InYoungsModulus, const double R)
			: ElasticHalfSpace(InPoissonsRatio, InYoungsModulus)
			, R(R)
		{}

		inline double Profile(const double x) const;

		inline double DepthFromContactAreaRadius(const double ConctactAreaRadius) const;
	
		inline double ContactAreaRadiusFromDepth(const double Depth) const;
	
		inline double ForceFromContactAreaRadius(const double E, const double ContactAreaRadius) const;
	
		inline double ForceFromDepth(const double E, const double Depth) const;

		inline double GetPotentialEnergyFromDeformation(const double E, const double Depth) const;
	
		inline double DepthFromForce(const double E, const double Force) const;
	
		inline double GetAverageStress(const double E, const double ContactAreaRadius) const;
	
		inline double GetStressDistribution(const double E, const double Depth, const double r) const;

		inline double GetDeformationDistribution(const double Depth, const double r) const;

	protected:
		inline double CalcU(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const;
	public:
		/* Streeses rt and tz vanish due to rotational symmetry */

		inline TBTensor3x3_Cylindrical GetStressDistribution(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const;
	};

	class Sphere : public ElasticHalfSpace
	{
		const double Radius;
	public:
		Sphere(const double InPoissonsRatio, const double InYoungsModulus, const double Radius)
			: ElasticHalfSpace(InPoissonsRatio, InYoungsModulus)
			, Radius(Radius)
		{}
	
		inline double Profile(const double x) const;
	
		inline double DepthFromContactAreaRadius(const double ConctactAreaRadius) const;
	
		inline double ContactAreaRadiusFromDepth(const double Depth) const;
	
		inline double ForceFromContactAreaRadius(const double E, const double ContactAreaRadius) const;
	
		inline double ForceFromDepth(const double E, const double Depth) const;
	
		inline double DepthFromForce(const double E, const double Force) const;
	
		inline double GetAverageStress(const double E, const double Depth) const;
	
		inline double GetStressDistribution(const double E, const double Depth, const double r) const;
	
		inline double GetDeformationDistribution(const double Depth, const double r) const;
	};

	class Ellipsoid : public ElasticHalfSpace
	{
		const double MinorRadius; // Radius parallel to the impact plane
		const double MajorRadius; // Radius perpendicular to the impact plane
	public:
		Ellipsoid(const double InPoissonsRatio, const double InYoungsModulus, const double MinorRadius, const double MajorRadius)
			: ElasticHalfSpace(InPoissonsRatio, InYoungsModulus)
			, MinorRadius(MinorRadius)
			, MajorRadius(MajorRadius)
		{}

	
	};


	#pragma region Implementations	
	inline double ElasticHalfSpace::CalculateEffectiveElasticModulus(const ElasticHalfSpace& Other) const
	{
		const double E0 = YoungsModulus;
		const double V0 = PoissonsRatio;

		const double E1 = Other.YoungsModulus;
		const double V1 = Other.PoissonsRatio;

		const double Numerator = -E0 * E1;
		const double Denominator = E1 * (V0 * V0 - 1) + E0 * (V1 * V1 - 1);

		return Denominator == 0.0 ? 0.0 : Numerator / Denominator;
	}

	inline double ElasticHalfSpace::CalculateEffectiveElasticModulus(const double OtherPoissonsRatio, const double OtherYoungsModulus) const
	{
		const double E0 = YoungsModulus;
		const double V0 = PoissonsRatio;

		const double E1 = OtherYoungsModulus;
		const double V1 = OtherPoissonsRatio;

		const double Numerator = -E0 * E1;
		const double Denominator = E1 * (V0 * V0 - 1) + E0 * (V1 * V1 - 1);

		return Denominator == 0.0 ? 0.0 : Numerator / Denominator;
	}



	inline double Cylinder::ForceFromDepth(const double E, const double Depth) const
	{
		return 2.0 * E * Depth * Radius;
	}
	
	inline double Cylinder::DepthFromForce(const double E, const double Force) const
	{
		return Force / (2.0 * E * Radius);
	}
	
	inline double Cylinder::GetAverageStress(const double E, const double Depth) const
	{
		return (E * Depth) / (UE_DOUBLE_PI * Radius);
	}
	
	inline double Cylinder::GetStressDistribution(const double E, const double Depth, const double r) const
	{
		if (abs(r) < Radius)
		{
			const double Numerator = E * Depth;
			const double Denominator = UE_DOUBLE_PI * sqrt(Radius * Radius - r * r);
			const double RetVal = Numerator / Denominator;
			return isfinite(RetVal) ? RetVal : 0.0;
		}
		else
		{
			return Depth;
		}
	}

	inline double Cylinder::GetDeformationDistribution(const double Depth, const double r) const
	{
		if(abs(r) > Radius)
		{
			const double term1 = (2.0 * Depth) / UE_DOUBLE_PI;
			const double term2 = asin(Radius / abs(r));
			return term1 * term2;
		}
		else
		{
			return Depth;
		}
	}



	inline double Cone::Profile(const double x) const
	{
		return abs(x) * tan(OutsideAngleRadians);
	}

	inline double Cone::GetContactAreaRadius(const double Depth) const
	{
		const double cot_theta = 1.0 / tan(OutsideAngleRadians);
		return (2.0 * Depth * cot_theta) / UE_DOUBLE_PI;
	}

	inline double Cone::DepthFromContactAreaRadius(const double ContactAreaRadius) const
	{
		return UE_DOUBLE_HALF_PI * ContactAreaRadius * tan(OutsideAngleRadians);
	}
	
	inline double Cone::ForceFromDepth(const double E, const double Depth) const
	{
		const double a = GetContactAreaRadius(Depth);
		const double E_tan_theta = E * tan(OutsideAngleRadians);
		const double pi_a_sqr = UE_DOUBLE_PI * a * a;
		return (pi_a_sqr / 2.0) * E_tan_theta;
	}
	
	inline double Cone::GetAverageStress(const double E) const
	{
		return 0.5 * E * tan(OutsideAngleRadians);
	}
	
	inline double Cone::GetStressDistribution(const double E, const double Depth, const double r) const
	{
		const double a = GetContactAreaRadius(Depth);
		const double p0 = GetAverageStress(E);
		
		//const double acosh_a_over_r = log((a + sqrt(a*a - r*r)) / abs(r));

		return p0 * acosh(a / abs(r));
	}
	
	inline double Cone::GetDeformationDistribution(const double Depth, const double r) const
	{
		const double a = GetContactAreaRadius(Depth);
		if(abs(r) > a)
		{
			const double a_tan_theta = a * tan(OutsideAngleRadians);
			const double term1 = asin(a / abs(r));
			const double term2 = (sqrt(r * r - a * a) - abs(r)) / a;
			return a_tan_theta * (term1 + term2);
		}
		else // We're inside the contact area radius, so deformation is simply the cone
		{
			return Depth - Profile(r); // Cone formula, shifted to the requested depth
		}
	}



	inline double Parabaloid::Profile(const double x) const
	{
		return (x * x) / (2.0 * R);
	}

	inline double Parabaloid::DepthFromContactAreaRadius(const double ConctactAreaRadius) const
	{
		return (ConctactAreaRadius * ConctactAreaRadius) / R;
	}

	inline double Parabaloid::ContactAreaRadiusFromDepth(const double Depth) const
	{
		return sqrt(R * Depth);
	}

	inline double Parabaloid::ForceFromContactAreaRadius(const double E, const double ContactAreaRadius) const
	{
		static constexpr double FourThirds = 4.0 / 3.0;
		return FourThirds * E * sqrt(R * pow(ContactAreaRadius, 3.0));
	}

	inline double Parabaloid::ForceFromDepth(const double E, const double Depth) const
	{
		const double a = ContactAreaRadiusFromDepth(Depth);
		return ForceFromContactAreaRadius(E, a);
	}

	inline double Parabaloid::GetPotentialEnergyFromDeformation(const double E, const double Depth) const
	{
		static constexpr double EightFifteenths = 8.0 / 15.0;
		return EightFifteenths * E * sqrt(R * pow(Depth, 5.0));
	}

	inline double Parabaloid::DepthFromForce(const double E, const double Force) const
	{
		const double Numerator = CubeRoot(3.0 * Force * R);
		const double Denominator = pow(2.0, 2.0 / 3.0) * CubeRoot(E);
		return Numerator / Denominator;
	}

	inline double Parabaloid::GetAverageStress(const double E, const double ContactAreaRadius) const
	{
		const double Numerator = 4.0 * E * ContactAreaRadius;
		const double Denominator = 3.0 * UE_DOUBLE_PI * R;
		return Numerator / Denominator;
	}

	inline double Parabaloid::GetStressDistribution(const double E, const double Depth, const double r) const
	{
		const double a = ContactAreaRadiusFromDepth(Depth);
		if(abs(r) <= a)
		{
			const double term1 = (2.0 * E) / (UE_DOUBLE_PI * R);
			const double Term2 = sqrt(a * a - r * r);
			return term1 * Term2;
		}
		else
		{
			return 0.0;
		}
	}

	inline double Parabaloid::GetDeformationDistribution(const double Depth, const double r) const
	{
		const double a = ContactAreaRadiusFromDepth(Depth);
		if (abs(r) > a)
		{
			const double term1 = (a * a) / (UE_DOUBLE_PI * R);
			const double term2 = 2.0 - (r * r) / (a * a);
			const double term3 = asin(a / abs(r));
			const double term4 = sqrt(r*r - a*a) / a;
			return term1 * (term2 * term3 + term4);
		}
		return Depth - Profile(r);
	}

	inline double Parabaloid::CalcU(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	{
		const double rSqr = r * r;
		const double zSqr = z * z;
		const double aSqr = a * a;

		const double rSqr_plus_zSqr_minus_aSqr = rSqr + zSqr - aSqr;

		const double root = sqrt(rSqr_plus_zSqr_minus_aSqr * rSqr_plus_zSqr_minus_aSqr + 4.0 * aSqr * zSqr);

		return 0.5 * (rSqr_plus_zSqr_minus_aSqr + root);
	}

	//inline double Parabaloid::GetStressDistributionWithinHalfSpace_rr(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	//{
	//	const double v = PoissonsRatio_HalfSpace;
	//	const double rSqr = r * r;
	//	const double zSqr = z * z;
	//	const double aSqr = a * a;
	//
	//	const double u = CalcU(r, z, a, PoissonsRatio_HalfSpace);
	//	const double uSqr = u * u;
	//	const double uSqrt = sqrt(u);
	//
	//	const double z_over_root_u = z / uSqrt;
	//	const double z_over_root_u_cubed = pow(z_over_root_u, 3.0);
	//
	//	const double term1_0 = ((1.0 - 2.0 * v) / 3.0) * (aSqr / rSqr) * (1.0 - z_over_root_u_cubed);
	//	const double term1_1 = z_over_root_u_cubed * (aSqr * u) / (uSqr + aSqr * zSqr);
	//	const double term1 = term1_0 + term1_1;
	//
	//	const double term2_0 = ((1 - v) * u) / (aSqr + u);
	//	const double term2_1 = (1 + v) * (uSqrt / a) * atan(a / uSqrt) - 2.0;
	//	const double term2 = (z / uSqrt) * (term2_0 + term2_1);
	//
	//	return 1.5 * (term1 + term2);
	//}
	//
	//inline double Parabaloid::GetStressDistributionWithinHalfSpace_tt(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	//{
	//	const double v = PoissonsRatio_HalfSpace;
	//	const double rSqr = r * r;
	//	const double zSqr = z * z;
	//	const double aSqr = a * a;
	//
	//	const double u = CalcU(r, z, a, PoissonsRatio_HalfSpace);
	//	const double uSqr = u * u;
	//	const double uSqrt = sqrt(u);
	//
	//	const double z_over_root_u = z / uSqrt;
	//
	//	const double term1_0 = ((1.0 - 2.0 * v) / 3.0) * (aSqr / rSqr);
	//	const double term1_1 = 1.0 - pow(z_over_root_u, 3.0);
	//	const double term1 = term1_0 * term1_1;
	//
	//	const double term2_0 = 2.0 * v + ((1 - v) * u) / (aSqr + u);
	//	const double term2_1 = (1 + v) * (uSqrt / a) * atan(a / uSqrt);
	//	const double term2 = (z / uSqrt) * (term2_0 - term2_1);
	//
	//	return 1.5 * (term1 + term2);
	//}
	//
	//FORCENOINLINE double Parabaloid::GetStressDistributionWithinHalfSpace_zz(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	//{
	//	const double rSqr = r * r;
	//	const double zSqr = z * z;
	//	const double aSqr = a * a;
	//
	//	const double u = CalcU(r, z, a, PoissonsRatio_HalfSpace);
	//	const double uSqr = u * u;
	//
	//	const double term1 = pow(z / sqrt(u), 3.0);
	//	const double term2 = (aSqr * u) / (uSqr + aSqr * zSqr);
	//
	//	return 1.5 * term1 * term2;
	//}
	//
	//inline double Parabaloid::GetStressDistributionWithinHalfSpace_rz(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	//{
	//	const double rSqr = r * r;
	//	const double zSqr = z * z;
	//	const double aSqr = a * a;
	//
	//	const double u = CalcU(r, z, a, PoissonsRatio_HalfSpace);
	//	const double uSqr = u * u;
	//
	//	const double term1 = (r * zSqr) / (uSqr + aSqr * zSqr);
	//	const double term2 = (aSqr * sqrt(u)) / (u + aSqr);
	//
	//	return 1.5 * term1 * term2;
	//}

	FORCENOINLINE TBTensor3x3_Cylindrical Parabaloid::GetStressDistribution(const double r, const double z, const double a, const double PoissonsRatio_HalfSpace) const
	{
		/* See fig. 2.32 for Huber's solution for stresses within the half-space from the indentation of a parabolic indenter */
		using namespace TB::ContactMechanics;

		const double v = PoissonsRatio_HalfSpace;
		const double rSqr = r * r;
		const double zSqr = z * z;
		const double aSqr = a * a;

		const double u = CalcU(r, z, a, PoissonsRatio_HalfSpace);
		const double uSqr = u * u;
		const double uSqrt = sqrt(u);

		const double z_over_root_u = z / uSqrt;
		const double z_over_root_u_cubed = pow(z_over_root_u, 3.0);

		double stress_rr = 0;
		double stress_tt = 0;
		double stress_zz = 0;
		double stress_rz = 0;

		{
			const double term1_0 = ((1.0 - 2.0 * v) / 3.0) * (aSqr / rSqr) * (1.0 - z_over_root_u_cubed);
			const double term1_1 = z_over_root_u_cubed * (aSqr * u) / (uSqr + aSqr * zSqr);
			const double term1 = term1_0 + term1_1;

			const double term2_0 = ((1 - v) * u) / (aSqr + u);
			const double term2_1 = (1 + v) * (uSqrt / a) * atan(a / uSqrt) - 2.0;
			const double term2 = (z / uSqrt) * (term2_0 + term2_1);

			stress_rr = 1.5 * (term1 + term2);
		}

		{
			const double term1_0 = ((1.0 - 2.0 * v) / 3.0) * (aSqr / rSqr);
			const double term1_1 = 1.0 - z_over_root_u_cubed;
			const double term1 = term1_0 * term1_1;

			const double term2_0 = 2.0 * v + ((1 - v) * u) / (aSqr + u);
			const double term2_1 = (1 + v) * (uSqrt / a) * atan(a / uSqrt);
			const double term2 = (z / uSqrt) * (term2_0 - term2_1);

			stress_tt = 1.5 * (term1 + term2);
		}

		{
			const double term1 = z_over_root_u_cubed;
			const double term2 = (aSqr * u) / (uSqr + aSqr * zSqr);

			stress_zz = 1.5 * term1 * term2;
		}

		{
			const double term1 = (r * zSqr) / (uSqr + aSqr * zSqr);
			const double term2 = (aSqr * uSqrt) / (u + aSqr);

			stress_rz = 1.5 * term1 * term2;
		}

		auto Tensor = TBTensor3x3_Cylindrical(stress_rr, stress_tt, stress_zz);
		Tensor.rz = stress_rz;
		Tensor.zr = stress_rz;

		return Tensor;
	}



	inline double Sphere::Profile(const double x) const
	{
		return Radius - sqrt(Radius * Radius - x * x);
	}

	inline double Sphere::DepthFromContactAreaRadius(const double ConctactAreaRadius) const
	{
		return (ConctactAreaRadius * ConctactAreaRadius) / Radius;
	}

	inline double Sphere::ContactAreaRadiusFromDepth(const double Depth) const
	{
		return sqrt(Depth * Radius);
	}

	inline double Sphere::ForceFromContactAreaRadius(const double E, const double ContactAreaRadius) const
	{
		const double d = DepthFromContactAreaRadius(ContactAreaRadius);
		return ForceFromDepth(E, d);
	}

	inline double Sphere::ForceFromDepth(const double E, const double Depth) const
	{
		return (4.0 / 3.0) * E * sqrt(Radius) * pow(Depth, 1.5);
	}

	inline double Sphere::DepthFromForce(const double E, const double Force) const
	{
		static constexpr double Three_Pow_TwoThirds = gcem::pow(3.0, 2.0 / 3.0);
		static constexpr double Two_CubeRoot_Two = 2.0 * CubeRootConst(2.0);
		return (Three_Pow_TwoThirds * pow(Force / (E * sqrt(Radius)), 2.0 / 3.0)) / Two_CubeRoot_Two;
	}

	inline double Sphere::GetAverageStress(const double E, const double Depth) const
	{
		return 0.0;
	}

	inline double Sphere::GetStressDistribution(const double E, const double Depth, const double r) const
	{
		if(abs(r) < sqrt(Radius * Depth))
		{
			const double F = ForceFromDepth(E, Depth);
			const double P0 = UE_DOUBLE_INV_PI * CubeRoot((3.0 * F * E * E) / (Radius * Radius));
			return P0 * sqrt(1.0 - (r * r) / (Radius * Depth));
		}
		else
		{
			return 0.0;
		}
	}

	inline double Sphere::GetDeformationDistribution(const double Depth, const double r) const
	{
		/* STUB */
		return 0.0;
	}
	#pragma endregion
};