// Copyright Erik Hedberg. All rights reserved.

#include "CoreMinimal.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Types/TBTensor.inl"
#include <tuple>

#define TestAccessor(variable, member) bPassed &= TestEqual(FString::Printf(TEXT("%s == Tensor.%s"), TEXT(PREPROCESSOR_TO_STRING(variable)), TEXT(PREPROCESSOR_TO_STRING(Tensor.member))), variable, Tensor.member)
#define TestAccessors(variable, a, b, c) \
TestAccessor(variable, a); \
TestAccessor(variable, b); \
TestAccessor(variable, c)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTBTensorTest, "Terminal Ballistics.Types.TBTensor", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter);
bool FTBTensorTest::RunTest(const FString& Parameters)
{
	using namespace TB::ContactMechanics;
	
	using FTensor = TTBTensor3x3<>;

	auto Rand = []() { return FMath::RandRange(0.0, 100.0); };

	auto [
		m0, m1, m2,
		m3, m4, m5,
		m6, m7, m8
	] = std::tuple<double, double, double, double, double, double, double, double, double>(
		Rand(), Rand(), Rand(),
		Rand(), Rand(), Rand(),
		Rand(), Rand(), Rand()
	);

	FTensor Tensor = FTensor(
		m0, m1, m2,
		m3, m4, m5,
		m6, m7, m8
	);

	bool bPassed = true;

	/* Test accessors */
	{
		TestAccessors(m0, x00, xx, rr);
		TestAccessors(m1, x01, xy, rt);
		TestAccessors(m2, x02, xz, rz);

		TestAccessors(m3, x10, yx, tr);
		TestAccessors(m4, x11, yy, tt);
		TestAccessors(m5, x12, yz, tz);

		TestAccessors(m6, x20, zx, zr);
		TestAccessors(m7, x21, zy, zt);
		TestAccessors(m8, x22, zz, zz);

		double elements[3][3] =
		{
			{ m0, m1, m2 },
			{ m3, m4, m5 },
			{ m6, m7, m8 }
		};

		bool bPassedGet = true;
		bool bPassedIndex = true;
		for (int i = 0; i < 9; i++)
		{
			const int row = fmod(i, 3);
			const int column = floor(i / 3);
			bPassedGet &= elements[row][column] == Tensor.Get(row, column);
			bPassedIndex &= elements[row][column] == Tensor.M[row][column];
		}
		bPassed &= TestTrue(FString("Getters should work properly"), bPassedGet && bPassedIndex);
	}

	return bPassed;
}

#endif
