// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include <ThirdParty/gcem/gcem.hpp>
#include <tuple>
#include <vector>
#include "CoreTypes.h"
#include "HAL/UnrealMemory.h"
#include "Math/Matrix.h"
#include "Math/UnrealMathSSE.h"
#include "Misc/CoreMiscDefines.h"


namespace TB::ContactMechanics
{
	enum class ETensorCoordinateType : uint8
	{
		Cartesian,
		Cylindrical
	};

	enum ETensorInitIdentity { InitIdentity };

	constexpr auto Coord_Cartesian = ETensorCoordinateType::Cartesian;
	constexpr auto Coord_Cylindrical = ETensorCoordinateType::Cylindrical;


	template<typename T = double, ETensorCoordinateType CoordType = Coord_Cartesian>
	class TTBTensor3x3
	{
	public:
		using RowType = std::tuple<T, T, T>;
		using ColumnType = RowType;

		union
		{
			T M[3][3];

			union
			{
				struct
				{
					T x00;
					T x01;
					T x02;

					T x10;
					T x11;
					T x12;

					T x20;
					T x21;
					T x22;
				};

				struct
				{
					T xx;
					T xy;
					T xz;

					T yx;
					T yy;
					T yz;

					T zx;
					T zy;
					T zz;
				};

				struct
				{
					T rr;
					T rt;
					T rz;

					T tr;
					T tt;
					T tz;

					T zr;
					T zt;
				};
			};
		};

#if ENABLE_NAN_DIAGNOSTIC
		FORCEINLINE void DiagnosticCheckNaN() const
		{
			if (ContainsNaN())
			{
				logOrEnsureNanError(TEXT("TBTensor3x3 contains NaN"));
				*const_cast<TBTensor3x3*>(static_cast<const TBTensor3x3*>(this)) = TBTensor3x3<T, CoordType>(ForceInitToZero);
			}
		}
#else
		FORCEINLINE void DiagnosticCheckNaN() const {}
#endif


		FORCEINLINE TTBTensor3x3();

		explicit FORCEINLINE TTBTensor3x3(EForceInit);

		explicit FORCEINLINE TTBTensor3x3(ETensorInitIdentity);

		explicit FORCEINLINE TTBTensor3x3(T arr[3][3]);

		FORCEINLINE TTBTensor3x3(
			const T m00, const T m01, const T m02,
			const T m10, const T m11, const T m12,
			const T m20, const T m21, const T m22
		);

		/* Construct from diagonal */
		FORCEINLINE TTBTensor3x3(
			const T m00,
			const T m11,
			const T m22
		);

		/* Construct from diagonal */
		FORCEINLINE TTBTensor3x3(const RowType& Diagonal);
		
		/* Construct from diagonal */
		FORCEINLINE TTBTensor3x3(const std::vector<T>& Diagonal);

		/* Fill will given value */
		FORCEINLINE TTBTensor3x3(const T& Value);

		template<ETensorCoordinateType OtherCoordType, typename std::enable_if_t<OtherCoordType != CoordType>>
		FORCEINLINE TTBTensor3x3(const TTBTensor3x3<T, OtherCoordType>& Other);


		FORCEINLINE static TTBTensor3x3<T, Coord_Cylindrical> CartesianToCylindrical(const TTBTensor3x3<T, Coord_Cartesian>& Tensor, const double x, const double y);
		FORCEINLINE static TTBTensor3x3<T, Coord_Cartesian> CylindricalToCartesian(const TTBTensor3x3<T, Coord_Cylindrical>& Tensor, const double r, const double theta);

		inline void SetIdentity();
		inline TTBTensor3x3<T, CoordType> GetIdentity() const;

		inline void SetToValue(const T& Value);

		inline RowType GetDiagonal() const;

		inline T GetMaxDiagonal() const;

		inline void SetDiagonal(const T& Value);
		inline void SetDiagonal(const T m00, const T m11, const T m22);


		/* Tensor-Tensor operators */

		FORCEINLINE TTBTensor3x3<T, CoordType>& operator*=(const TTBTensor3x3<T, CoordType>& Other);
		template<typename T2, ETensorCoordinateType CoordType2>
		friend TTBTensor3x3<T2, CoordType2> operator*(const TTBTensor3x3<T2, CoordType2>& lhs, const TTBTensor3x3<T2, CoordType2>& rhs);

		FORCEINLINE TTBTensor3x3<T, CoordType>& operator+=(const TTBTensor3x3<T, CoordType>& Other);
		template<typename T2, ETensorCoordinateType CoordType2>
		friend TTBTensor3x3<T2, CoordType2> operator-(const TTBTensor3x3<T2, CoordType2>& lhs, const TTBTensor3x3<T2, CoordType2>& rhs);

		FORCEINLINE TTBTensor3x3<T, CoordType>& operator-=(const TTBTensor3x3<T, CoordType>& Other);
		template<typename T2, ETensorCoordinateType CoordType2>
		friend TTBTensor3x3<T2, CoordType2> operator-(const TTBTensor3x3<T2, CoordType2>& lhs, const TTBTensor3x3<T2, CoordType2>& rhs);

		/* Scaling operator */
		
		FORCEINLINE TTBTensor3x3<T, CoordType>& operator*=(const T& Other);
		FORCEINLINE TTBTensor3x3<T, CoordType> operator*(const T& Other);
		
		FORCEINLINE TTBTensor3x3<T, CoordType>& operator/=(const T& Other);
		FORCEINLINE TTBTensor3x3<T, CoordType> operator/(const T& Other);


		/* Equality */

		FORCEINLINE bool operator==(const TTBTensor3x3<T, CoordType>& Other) const;
		FORCEINLINE bool operator!=(const TTBTensor3x3<T, CoordType>& Other) const;


		FORCEINLINE bool IsIdentity() const;


		FORCEINLINE			T Get(int i);
		FORCEINLINE	const	T Get(int i) const;

		FORCEINLINE			T Get(int row, int col);
		FORCEINLINE	const	T Get(int row, int col) const;

		FORCEINLINE void Set(int i, const T& Value);
		FORCEINLINE void Set(int row, int col, const T& Value);


		inline T GetDeterminant() const;
		
		inline TTBTensor3x3<T, CoordType>& Transpose();
		inline TTBTensor3x3<T, CoordType> GetTransposed() const;

		inline TTBTensor3x3<T, CoordType> GetCofactorMatrix() const;

		inline TTBTensor3x3<T, CoordType> GetAdjoint() const;

		inline TTBTensor3x3<T, CoordType>& Inverse();
		inline TTBTensor3x3<T, CoordType> GetInverse() const;


		inline RowType GetRow(int i) const;
		inline void SetRow(int i, const T& Value) const;
		inline void SetRow(int i, const RowType& Value) const;
		inline void SetRow(int i, const T& V0, const T& V1, const T& V2) const;

		inline ColumnType GetColumn(int i) const;
		inline void SetColumn(int i, const T& Value) const;
		inline void SetColumn(int i, const ColumnType& Value) const;
		inline void SetColumn(int i, const T& V0, const T& V1, const T& V2) const;

		inline bool ContainsNaN() const;
	};

	template<ETensorCoordinateType CoordType>
	using TBTensor3x3 = TTBTensor3x3<double, CoordType>;
	using TBTensor3x3_Cartesian = TTBTensor3x3<double, Coord_Cartesian>;
	using TBTensor3x3_Cylindrical = TTBTensor3x3<double, Coord_Cylindrical>;

	template<ETensorCoordinateType CoordType>
	using TBStressTensor = TTBTensor3x3<double, CoordType>;
	using TBStressTensor_Cartesian = TTBTensor3x3<double, Coord_Cartesian>;
	using TBStressTensor_Cylindrical = TTBTensor3x3<double, Coord_Cylindrical>;


	// Inline functions

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3()
		: M{
			{T(), T(), T()},
			{T(), T(), T()},
			{T(), T(), T()}
		}
	{}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(EForceInit)
	{
		FMemory::Memzero(this, sizeof(*this));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(ETensorInitIdentity)
		: M{
			{(T)1.0, (T)0.0, (T)0.0},
			{(T)0.0, (T)1.0, (T)0.0},
			{(T)0.0, (T)0.0, (T)1.0}
		}
	{}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(T arr[3][3])
	{
		FMemory::Memcpy(&M, &arr, sizeof(M));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const T m00, const T m01, const T m02, const T m10, const T m11, const T m12, const T m20, const T m21, const T m22)
		: M{
			{m00, m01, m02},
			{m10, m11, m12},
			{m20, m21, m22}
		}
	{
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const T m00, const T m11, const T m22)
		: M{
			{m00, T(), T()},
			{T(), m11, T()},
			{T(), T(), m22}
		}
	{
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const RowType& Diagonal)
		: M{
			{ std::get<0>(Diagonal), T(),					T()					  },
			{ T(),					std::get<1>(Diagonal),	T()					  },
			{ T(),					T(),					std::get<2>(Diagonal) }
		}
	{
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const std::vector<T>& Diagonal)
		: M{
			{Diagonal[0], T(), T()},
			{T(), Diagonal[1], T()},
			{T(), T(), Diagonal[2]}
		}
	{
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const T& Value)
		: M{
			{Value, Value, Value},
			{Value, Value, Value},
			{Value, Value, Value}
		}
	{
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	template<ETensorCoordinateType OtherCoordType, typename std::enable_if_t<OtherCoordType != CoordType>>
	inline TTBTensor3x3<T, CoordType>::TTBTensor3x3(const TTBTensor3x3<T, OtherCoordType>& Other)
	{
		FMemory::Memcpy(&M, &Other.M, sizeof(M));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, Coord_Cylindrical> TTBTensor3x3<T, CoordType>::CartesianToCylindrical(const TTBTensor3x3<T, Coord_Cartesian>& Tensor, const double x, const double y)
	{
		const auto xSqr_ySqr = x * x + y * y;
		const auto sqrt_xSqr_ySqr = sqrt(xSqr_ySqr);

		const TTBTensor3x3<T, CoordType> Jacobian = TTBTensor3x3<T, CoordType>(
			x / sqrt_xSqr_ySqr, y / sqrt_xSqr_ySqr, (T)0.0,
			-y / xSqr_ySqr, x / xSqr_ySqr, (T)0.0,
			(T)0.0, (T)0.0, (T)1.0
		);

		auto Transformed = Tensor * Jacobian;
		return TTBTensor3x3<T, Coord_Cylindrical>(MoveTemp(Transformed.M));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, Coord_Cartesian> TTBTensor3x3<T, CoordType>::CylindricalToCartesian(const TTBTensor3x3<T, Coord_Cylindrical>& Tensor, const double r, const double theta)
	{
		const TTBTensor3x3<T, CoordType> Jacobian = TTBTensor3x3<T, CoordType>(
			cos(theta), -r * sin(theta), (T)0.0,
			sin(theta), r * cos(theta), (T)0.0,
			(T)0.0, (T)0.0, (T)1.0
		);

		auto Transformed = Tensor * Jacobian;
		return TTBTensor3x3<T, Coord_Cartesian>(MoveTemp(Transformed.M));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetIdentity()
	{
		M[0][0] = (T)1.0; M[0][1] = (T)0.0; M[0][2] = (T)0.0;
		M[1][0] = (T)0.0; M[1][1] = (T)1.0; M[1][2] = (T)0.0;
		M[2][0] = (T)0.0; M[2][1] = (T)0.0; M[2][2] = (T)1.0;
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::GetIdentity() const
	{
		TTBTensor3x3<T, CoordType> I;
		I.SetIdentity();
		return I;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetToValue(const T& Value)
	{
		M[0][0] = Value; M[0][1] = Value; M[0][2] = Value;
		M[1][0] = Value; M[1][1] = Value; M[1][2] = Value;
		M[2][0] = Value; M[2][1] = Value; M[2][2] = Value;
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline std::tuple<T, T, T> TTBTensor3x3<T, CoordType>::GetDiagonal() const
	{
		return RowType(x00, x11, x22);
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline T TTBTensor3x3<T, CoordType>::GetMaxDiagonal() const
	{
		return FMath::Max3(abs(xx), abs(yy), abs(zz));
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetDiagonal(const T& Value)
	{
		M[0][0] = Value;
		M[1][1] = Value;
		M[2][2] = Value;
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetDiagonal(const T m00, const T m11, const T m22)
	{
		M[0][0] = m00;
		M[1][1] = m11;
		M[2][2] = m22;
		DiagnosticCheckNaN();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::operator*=(const TTBTensor3x3<T, CoordType>& Other)
	{
		TTBTensor3x3<T, CoordType> C = *this;
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				C.M[i][j] =
					M[i][0] * Other.M[0][j] +
					M[i][1] * Other.M[1][j] +
					M[i][2] * Other.M[2][j];
			}
		}
		*this = C;
		return *this;
	}

	template<typename T, ETensorCoordinateType CoordType>
	TTBTensor3x3<T, CoordType> operator*(const TTBTensor3x3<T, CoordType>& lhs, const TTBTensor3x3<T, CoordType>& rhs)
	{
		TTBTensor3x3<T, CoordType> result = lhs;
		result *= rhs;
		return rhs;
	}


	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::operator+=(const TTBTensor3x3<T, CoordType>& Other)
	{
		M[0][0] += Other.M[0][0];
		M[0][1] += Other.M[0][1];
		M[0][2] += Other.M[0][2];
		M[1][0] += Other.M[1][0];
		M[1][1] += Other.M[1][1];
		M[1][2] += Other.M[1][2];
		M[2][0] += Other.M[2][0];
		M[2][1] += Other.M[2][1];
		M[2][2] += Other.M[2][2];
		return *this;
	}
	template<typename T, ETensorCoordinateType CoordType>
	TTBTensor3x3<T, CoordType> operator+(const TTBTensor3x3<T, CoordType>& lhs, const TTBTensor3x3<T, CoordType>& rhs)
	{
		TTBTensor3x3<T, CoordType> result = lhs;
		result += rhs;
		return rhs;
	}


	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::operator-=(const TTBTensor3x3<T, CoordType>& Other)
	{
		M[0][0] -= Other.M[0][0];
		M[0][1] -= Other.M[0][1];
		M[0][2] -= Other.M[0][2];
		M[1][0] -= Other.M[1][0];
		M[1][1] -= Other.M[1][1];
		M[1][2] -= Other.M[1][2];
		M[2][0] -= Other.M[2][0];
		M[2][1] -= Other.M[2][1];
		M[2][2] -= Other.M[2][2];
		return *this;
	}
	template<typename T, ETensorCoordinateType CoordType>
	TTBTensor3x3<T, CoordType> operator-(const TTBTensor3x3<T, CoordType>& lhs, const TTBTensor3x3<T, CoordType>& rhs)
	{
		TTBTensor3x3<T, CoordType> result = lhs;
		result -= rhs;
		return rhs;
	}


	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::operator*=(const T& Other)
	{
		M[0][0] *= Other;
		M[0][1] *= Other;
		M[0][2] *= Other;
		M[1][0] *= Other;
		M[1][1] *= Other;
		M[1][2] *= Other;
		M[2][0] *= Other;
		M[2][1] *= Other;
		M[2][2] *= Other;
		return *this;
	}
	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::operator*(const T& Other)
	{
		TTBTensor3x3<T, CoordType> result = *this;
		result *= Other;
		return result;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::operator/=(const T& Other)
	{
		*this *= 1.0 / Other;
		return *this;
	}
	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::operator/(const T& Other)
	{
		TTBTensor3x3<T, CoordType> result = *this;
		result /= Other;
		return result;
	}


	template<typename T, ETensorCoordinateType CoordType>
	inline bool TTBTensor3x3<T, CoordType>::operator==(const TTBTensor3x3<T, CoordType>& Other) const
	{
		return M == Other.M;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline bool TTBTensor3x3<T, CoordType>::operator!=(const TTBTensor3x3<T, CoordType>& Other) const
	{
		return !(*this == Other);
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline bool TTBTensor3x3<T, CoordType>::IsIdentity() const
	{
		return *this == GetIdentity();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline T TTBTensor3x3<T, CoordType>::Get(int i)
	{
		const int row = fmod(i, 3);
		const int column = floor(i / 3);
		return M[row][column];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline const T TTBTensor3x3<T, CoordType>::Get(int i) const
	{
		const int row = fmod(i, 3);
		const int column = floor(i / 3);
		return M[row][column];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline T TTBTensor3x3<T, CoordType>::Get(int row, int col)
	{
		return M[row][col];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline const T TTBTensor3x3<T, CoordType>::Get(int row, int col) const
	{
		return M[row][col];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::Set(int i, const T& Value)
	{
		const int row = fmod(i, 3);
		const int column = floor(i / 3);
		M[row][column] = Value;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::Set(int row, int col, const T& Value)
	{
		M[row][col] = Value;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline T TTBTensor3x3<T, CoordType>::GetDeterminant() const
	{
		return
			M[0][0] * M[1][1] * M[2][2] +
			M[0][1] * M[1][2] * M[2][0] +
			M[0][2] * M[1][0] * M[2][1] -
			M[0][2] * M[1][1] * M[2][0] -
			M[0][1] * M[1][0] * M[2][2] -
			M[0][0] * M[1][2] * M[2][1];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::Transpose()
	{
		swap(M[0][1], M[1][0]);
		swap(M[0][2], M[2][0]);
		swap(M[1][2], M[2][1]);
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::GetTransposed() const
	{
		return TTBTensor3x3<T, CoordType>(
			M[0][0], M[1][0], M[2][0],
			M[0][1], M[1][1], M[2][1],
			M[0][2], M[1][2], M[2][2]
		);
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::GetCofactorMatrix() const
	{
		auto Det2x2 = [](const T& a, const T& b, const T& c, const T& d)
			{
				return a * d - b * c;
			};

		const RowType Row0 =
		{
			Det2x2(
				M[1][1], M[1][2],
				M[2][1], M[2][2]
			),
			-Det2x2(
				M[1][0], M[1][2],
				M[2][0], M[2][2]
			),
			Det2x2(
				M[1][0], M[1][1],
				M[2][0], M[2][1]
			)
		};

		const RowType Row1 =
		{
			-Det2x2(
				M[0][1], M[0][2],
				M[2][1], M[2][2]
			),
			Det2x2(
				M[0][0], M[0][2],
				M[2][0], M[2][2]
			),
			-Det2x2(
				M[0][0], M[0][1],
				M[2][0], M[2][1]
			)
		};

		const RowType Row2 =
		{
			Det2x2(
				M[0][1], M[0][2],
				M[1][1], M[1][2]
			),
			-Det2x2(
				M[0][0], M[0][2],
				M[1][0], M[1][2]
			),
			Det2x2(
				M[0][0], M[0][1],
				M[1][0], M[1][1]
			)
		};

		return TTBTensor3x3<T, CoordType>(Row0, Row1, Row2);
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::GetAdjoint() const
	{
		return GetCofactorMatrix().Transpose();
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType>& TTBTensor3x3<T, CoordType>::Inverse()
	{
		const T Det = GetDeterminant();
		if (!IsIdentity() || Det == (T)0.0)
		{
			const T InvDet = (T)1.0 / Det;
			*this = InvDet * GetAdjoint();
		}
		return *this;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline TTBTensor3x3<T, CoordType> TTBTensor3x3<T, CoordType>::GetInverse() const
	{
		TTBTensor3x3<T, CoordType> result = *this;
		result.Inverse();
		return result;;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline std::tuple<T, T, T> TTBTensor3x3<T, CoordType>::GetRow(int i) const
	{
		return { M[i][0], M[i][1], M[i][2] };
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetRow(int i, const T& Value) const
	{
		M[i][0] = Value;
		M[i][1] = Value;
		M[i][2] = Value;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetRow(int i, const RowType& Value) const
	{
		M[i][0] = Value[0];
		M[i][1] = Value[1];
		M[i][2] = Value[2];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetRow(int i, const T& V0, const T& V1, const T& V2) const
	{
		M[i][0] = V0;
		M[i][1] = V1;
		M[i][2] = V2;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline std::tuple<T, T, T> TTBTensor3x3<T, CoordType>::GetColumn(int i) const
	{
		return { M[0][i], M[1][i], M[2][i] };
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetColumn(int i, const T& Value) const
	{
		M[0][i] = Value;
		M[1][i] = Value;
		M[2][i] = Value;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetColumn(int i, const ColumnType& Value) const
	{
		M[0][i] = Value[0];
		M[1][i] = Value[1];
		M[2][i] = Value[2];
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline void TTBTensor3x3<T, CoordType>::SetColumn(int i, const T& V0, const T& V1, const T& V2) const
	{
		M[0][i] = V0;
		M[1][i] = V1;
		M[2][i] = V2;
	}

	template<typename T, ETensorCoordinateType CoordType>
	inline bool TTBTensor3x3<T, CoordType>::ContainsNaN() const
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (!FMath::IsFinite(M[i][j]))
				{
					return false;
				}
			}
		}
		return false;
	}
};
