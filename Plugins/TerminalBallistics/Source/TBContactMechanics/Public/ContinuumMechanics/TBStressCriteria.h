// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/TBTensor.inl"

namespace TB::ContactMechanics
{
	namespace Private
	{
		class Singleton
		{
			Singleton() = delete;
			Singleton(Singleton&) = delete;
			Singleton(Singleton&&) = delete;
		};
	};

	struct FVonMisesYieldCriterion : public Private::Singleton
	{
	private:
		template<typename T>
		static constexpr T sqr(const T x) { return x * x; }
	public:
		static double CalculateEquivalentStress(const TBStressTensor_Cartesian& StressTensor)
		{
			using RowType = TBStressTensor_Cartesian::RowType;
			if (StressTensor.GetDiagonal() == RowType(0.0, 0.0, 0.0) && StressTensor.zx == 0.0 && StressTensor.yz == 0.0) // Pure shear
			{
				return UE_DOUBLE_SQRT_3 * FMath::Abs(StressTensor.xy);
			}
			else
			{
				const double term1 = sqr(StressTensor.xx - StressTensor.yy) + sqr(StressTensor.yy - StressTensor.zz) + sqr(StressTensor.zz - StressTensor.xx);
				const double term2 = 6.0 * (sqr(StressTensor.xy) + sqr(StressTensor.yz) + sqr(StressTensor.zx));
				return FMath::Sqrt(0.5 * (term1 + term2));
			}
		}
		static double CalculateEquivalentStress(const TBStressTensor_Cylindrical& StressTensor)
		{
			using RowType = TBStressTensor_Cylindrical::RowType;
			if (StressTensor.GetDiagonal() == RowType(0.0, 0.0, 0.0) && StressTensor.zx == 0.0 && StressTensor.yz == 0.0) // Pure shear
			{
				return UE_DOUBLE_SQRT_3 * FMath::Abs(StressTensor.xy);
			}
			else
			{
				const double term1 = sqr(StressTensor.rr - StressTensor.tt) + sqr(StressTensor.tt - StressTensor.zz) + sqr(StressTensor.zz - StressTensor.rr);
				const double term2 = 6.0 * (sqr(StressTensor.rt) + sqr(StressTensor.tz) + sqr(StressTensor.zr));
				return FMath::Sqrt(0.5 * (term1 + term2));
			}
		}

		template<ETensorCoordinateType CoordType>
		static inline bool TestYield(const TBStressTensor<CoordType>& StressTensor, const double YieldStrength)
		{
			return CalculateEquivalentStress(StressTensor) >= YieldStrength;
		}
	};

	struct FTrescaYieldCriterion : public Private::Singleton
	{
		template<ETensorCoordinateType CoordType>
		inline static bool TestYield(const TBStressTensor<CoordType>& StressTensor, const double YieldStrength)
		{
			const double T_crit = 0.5 * YieldStrength;
			const double T_max = 0.5 * (FMath::Max3(StressTensor.x00, StressTensor.x11, StressTensor.x22) - FMath::Min3(StressTensor.x00, StressTensor.x11, StressTensor.x22));
			return T_max >= T_crit;
		}
	};

	struct FGCriterion : public Private::Singleton
	{
		template<ETensorCoordinateType CoordType>
		inline static bool TestYield(const TBStressTensor<CoordType>& StressTensor, const double CrackArea, const double FractureToughness)
		{
			const double K1 = StressTensor.GetMaxDiagonal() * sqrt(CrackArea);
			return K1 >= FractureToughness;
		}
	};
};
