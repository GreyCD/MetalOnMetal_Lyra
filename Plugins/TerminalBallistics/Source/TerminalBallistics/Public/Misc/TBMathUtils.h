// Copyright Erik Hedberg. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/TBTraits.h"
#include <random>
#include <ThirdParty/gcem/gcem.hpp> // This must be included as soon as possible
#include <type_traits>

#define TB_SQR(X) TB::MathUtils::sqr(X)


namespace TB::MathUtils
{
	static FORCEINLINE float SinD(const float X)
	{
		return sin(UE_PI / (180) * X);
	}
	static FORCEINLINE double SinD(const double X)
	{
		return sin(UE_DOUBLE_PI / (180.0) * X);
	}

	static FORCEINLINE float CosD(const float X)
	{
		return cos(UE_PI / (180) * X);
	}
	static FORCEINLINE double CosD(const double X)
	{
		return cos(UE_DOUBLE_PI / (180) * X);
	}

	static FORCEINLINE float TanD(const float X)
	{
		return tan(UE_PI / (180) * X);
	}
	static FORCEINLINE double TanD(const double X)
	{
		return tan(UE_DOUBLE_PI / (180) * X);
	}

	static FORCEINLINE float AsinD(const float X)
	{
		return 180.f / UE_PI * asin(X);
	}
	static FORCEINLINE double AsinD(const double X)
	{
		return 180.0 / UE_DOUBLE_PI * asin(X);
	}

	static FORCEINLINE float AcosD(const float X)
	{
		return 180.f / UE_PI * acos(X);
	}
	static FORCEINLINE double AcosD(const double X)
	{
		return 180.0 / UE_DOUBLE_PI * acos(X);
	}

	static FORCEINLINE float AtanD(const float X)
	{
		return 180.f / UE_PI * atan(X);
	}
	static FORCEINLINE double AtanD(const double X)
	{
		return 180.0 / UE_DOUBLE_PI * atan(X);
	}

	using ::atan;
	template<TB::Concepts::Vector T>
	static FORCEINLINE auto atan(const T& Vector)
	{
		if constexpr (std::is_same_v<UE::Math::TVector2<typename T::FReal>, T>)
		{
			return T(
				atan(Vector.X),
				atan(Vector.Y)
			);
		}
		else
		{
			return T(
				atan(Vector.X),
				atan(Vector.Y),
				atan(Vector.Z)
			);
		}
	}

	using ::tan;
	template<TB::Concepts::Vector T>
	static FORCEINLINE auto tan(const T& Vector)
	{
		if constexpr (std::is_same_v<UE::Math::TVector2<typename T::FReal>, T>)
		{
			return T(
				tan(Vector.X),
				tan(Vector.Y)
			);
		}
		else
		{
			return T(
				tan(Vector.X),
				tan(Vector.Y),
				tan(Vector.Z)
			);
		}
	}

	template<typename T>
	static FORCEINLINE T sqr(const T X)
	{
		return X * X;
	}

	// FVector implementation of exp
	template<TB::Concepts::Vector3 VectorType = FVector>
	static FORCEINLINE auto expv(const VectorType& Vector)
	{
		return VectorType(
			exp(Vector.X),
			exp(Vector.Y),
			exp(Vector.Z)
		);
	}

	
	template<typename T1, typename T2, typename T3, typename T4, typename T5> requires TB::Traits::TAllAreArithmetic<T1, T2, T3, T4, T5>::Value
	static FORCEINLINE auto MapRangeClamped(const T1 InMin, const T2 InMax, const T3 OutMin, const T4 OutMax, const T5 Value)
	{
		using T = decltype(T1() * T2() * T3() * T4() * T5());
		return FMath::GetMappedRangeValueClamped(TRange<T>(InMin, InMax), TRange<T>(OutMin, OutMax), Value);
	}

	static FORCEINLINE double NormalizeAngleToPlusMinus90DegreeRange(const double AngleDegrees)
	{
		double Normalized = fmod(AngleDegrees, 180);
		if (Normalized > 90.0)
		{
			Normalized = 90.0 - Normalized;
		}
		return Normalized;
	}

	static FORCEINLINE double NormalizeAngleToPlusMinus90DegreeRangeRadians(const double AngleRadians)
	{
		double Normalized = fmod(AngleRadians, UE_DOUBLE_PI);
		if (Normalized > UE_DOUBLE_HALF_PI)
		{
			Normalized = UE_DOUBLE_HALF_PI - Normalized;
		}
		return Normalized;
	}

	namespace RandStatics
	{
		static std::random_device rd{};
		static std::default_random_engine gen{ rd() };
		static void Seed(const unsigned int seed)
		{
			gen.seed(seed);
		}
	};

	template<typename T = double> requires std::is_arithmetic_v<T>
	static T GaussianRand(const T Mean = (T)0.5, const T StdDev = (T)(1.0 / 6.0))
	{
		using namespace RandStatics;
		std::normal_distribution<T> dist{ Mean, StdDev };
		return dist(gen);
	}

	namespace VectorUtils
	{
		/**
		* Returns a random unit vector within the specified cone using a normal distribution.
		* Adapted from the uniform-distributed FMath::VRandCone function.
		*/
		static FVector VRandConeGuassian(const FVector& Direction, double ConeHalfAngleRad)
		{
			if (ConeHalfAngleRad <= 0.0)
			{
				return Direction.GetSafeNormal();
			}

			const double RandU = FMath::Clamp(GaussianRand(), 0.0, 1.0);
			const double RandV = FMath::Clamp(GaussianRand(), 0.0, 1.0);

			// Get spherical coords that have an even distribution over the unit sphere
			// Method described at http://mathworld.wolfram.com/SpherePointPicking.html	
			double Theta = 2.0 * UE_DOUBLE_PI * RandU;
			double Phi = FMath::Acos((2.0 * RandV) - 1.0);

			// restrict phi to [0, ConeHalfAngleRad]
			// this gives an even distribution of points on the surface of the cone
			// centered at the origin, pointing upward (z), with the desired angle
			Phi = FMath::Fmod(Phi, ConeHalfAngleRad);

			// get axes we need to rotate around
			FMatrix const DirMat = FRotationMatrix(Direction.Rotation());
			// note the axis translation, since we want the variation to be around X
			FVector const DirZ = DirMat.GetScaledAxis(EAxis::X);
			FVector const DirY = DirMat.GetScaledAxis(EAxis::Y);

			FVector Result = Direction.RotateAngleAxisRad(Phi, DirY);
			Result = Result.RotateAngleAxisRad(Theta, DirZ);

			// ensure it's a unit vector (might not have been passed in that way)
			Result = Result.GetSafeNormal();

			return Result;
		}

		/**
		 * This is a version of VRandConeGuassian that handles "squished" cones, i.e. with different angle limits in the Y and Z axes.
		 * Assumes world Y and Z, although this could be extended to handle arbitrary rotations.
		 * Adapted from the uniform-distributed FMath::VRandCone function.
		 */
		static FVector VRandConeGuassian(const FVector& Direction, const double HorizontalConeHalfAngleRad, const double VerticalConeHalfAngleRad)
		{
			if (HorizontalConeHalfAngleRad <= 0.0 || VerticalConeHalfAngleRad <= 0.0)
			{
				return Direction.GetSafeNormal();
			}

			const double RandU = FMath::Clamp(GaussianRand(), 0.0, 1.0);
			const double RandV = FMath::Clamp(GaussianRand(), 0.0, 1.0);

			// Get spherical coords that have an even distribution over the unit sphere
			// Method described at http://mathworld.wolfram.com/SpherePointPicking.html	
			double Theta = 2.0 * UE_DOUBLE_PI * RandU;
			double Phi = FMath::Acos((2.0 * RandV) - 1.0);

			// restrict phi to [0, ConeHalfAngleRad]
			// where ConeHalfAngleRad is now a function of Theta
			// (specifically, radius of an ellipse as a function of angle)
			// function is ellipse function (x/a)^2 + (y/b)^2 = 1, converted to polar coords
			double ConeHalfAngleRad = FMath::Square(FMath::Cos(Theta) / VerticalConeHalfAngleRad) + FMath::Square(FMath::Sin(Theta) / HorizontalConeHalfAngleRad);
			ConeHalfAngleRad = FMath::Sqrt(1.0 / ConeHalfAngleRad);

			// clamp to make a cone instead of a sphere
			Phi = FMath::Fmod(Phi, ConeHalfAngleRad);

			// get axes we need to rotate around
			const FMatrix DirMat = FRotationMatrix(Direction.Rotation());
			// note the axis translation, since we want the variation to be around X
			const FVector DirZ = DirMat.GetScaledAxis(EAxis::X);
			const FVector DirY = DirMat.GetScaledAxis(EAxis::Y);

			FVector Result = Direction.RotateAngleAxisRad(Phi, DirY);
			Result = Result.RotateAngleAxisRad(Theta, DirZ);

			// ensure it's a unit vector (might not have been passed in that way)
			Result = Result.GetSafeNormal();

			return Result;
		}

		template<TB::Concepts::Vector3 VectorType = FVector>
		static FORCEINLINE auto GetClosestPointOnLine(const VectorType& Point, const VectorType& LineStart, const VectorType& LineEnd)
		{
			const VectorType Direction = (LineEnd - LineStart).GetSafeNormal();
			return LineStart + (Direction * ((Point - LineStart) | Direction));
		}

		template<TB::Concepts::Vector3 VectorType = FVector>
		static FORCEINLINE auto GetClosestPointOnLine(const VectorType& Point, const VectorType& LineStart, const VectorType& LineEnd, typename VectorType::FReal& Distance)
		{
			const VectorType Direction = (LineEnd - LineStart).GetSafeNormal();
			const VectorType ClosestPoint = LineStart + (Direction * ((Point - LineStart) | Direction));
			Distance = VectorType::Dist(ClosestPoint, Point);
			return ClosestPoint;
		}

		template<TB::Concepts::Vector3 VectorType = FVector>
		static FORCEINLINE auto GetDistanceFromLine(const VectorType& Point, const VectorType& LineStart, const VectorType& LineEnd)
		{
			return VectorType::Dist(Point, GetClosestPointOnLine(Point, LineStart, LineEnd));
		}

		template<TB::Concepts::Vector3 Vec1 = FVector, TB::Concepts::Vector3 Vec2 = FVector>
		static FORCEINLINE auto GetImpactAngle(const Vec1& SurfaceNormal, const Vec2& DirectionVector)
		{
			return UE_DOUBLE_HALF_PI - acos(-SurfaceNormal.GetSafeNormal() | DirectionVector.GetSafeNormal());
		}

		template<TB::Concepts::Vector3 Vec1 = FVector, TB::Concepts::Vector3 Vec2 = FVector>
		static FORCEINLINE auto GetImpactAngleAlt(const Vec1& SurfaceNormal, const Vec2& DirectionVector)
		{
			return acos(-SurfaceNormal | DirectionVector.GetSafeNormal());
		}

		template<TB::Concepts::Vector3 VectorType = FVector>
		static FORCEINLINE VectorType ProjectObjectSpaceVectorOntoWorldSpaceDirectionVector(const VectorType& VectorToProject, const VectorType& DirectionVector, const FQuat& ObjectOrientation)
		{
			using FReal = typename VectorType::FReal;

			const VectorType Norm = DirectionVector.GetSafeNormal();

			const VectorType ObjectSpaceAxes[3] = {
				ObjectOrientation.GetAxisX(),
				ObjectOrientation.GetAxisY(),
				ObjectOrientation.GetAxisZ()
			};

			const double DotX = VectorToProject | (ObjectSpaceAxes[0] ^ Norm);
			const double DotY = VectorToProject | (ObjectSpaceAxes[1] ^ Norm);
			const double DotZ = VectorToProject | (ObjectSpaceAxes[2] ^ Norm);


			const VectorType ProjectionLocal = VectorType(DotX, DotY, DotZ);
			const VectorType ProjectionWorld = ObjectOrientation.Inverse().RotateVector(ProjectionLocal);
			return ProjectionWorld;
		}

		template<TB::Concepts::Vector2 VectorType = FVector2D>
		static FORCEINLINE auto GetDistance2D(const VectorType& PointA, const VectorType& PointB) -> decltype(VectorType::FReal)
		{
			return sqrt(sqr(PointA.X - PointB.X) + sqr(PointA.Y - PointB.Y));
		}

		template<TB::Concepts::Vector VectorType = FVector2D>
		static FORCEINLINE auto GetVectorMagnitudeInDirection(const VectorType& Vector, const VectorType& DirectionVector) -> decltype(VectorType::FReal)
		{
			using FReal = typename VectorType::FReal;

			FReal Magnitude = Vector.Size();

			VectorType DirectionNorm = DirectionVector.GetSafeNormal();

			FReal Dot = Vector | DirectionNorm;

			return Magnitude * Dot;
		}
	};
};
