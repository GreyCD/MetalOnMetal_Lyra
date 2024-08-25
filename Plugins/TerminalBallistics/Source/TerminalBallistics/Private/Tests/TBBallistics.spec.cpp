// Copyright Erik Hedberg. All rights reserved

#include "Tests/TBTestingUtils.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Core/TBBullets.h"
#include "Misc/TBBulletUtils.h"
#include "Core/TBBallisticFunctions.h"
#include "PhysMatManager/PhysMatManager.h"
#include "PhysMatManager/PhysMat.h"
#include <functional>


#define AFTER_EACH AfterEach(GetAfterEachFunc())
#define BEFORE_EACH BeforeEach(GetBeforeEachFunc())


struct FBulletFixture
{
	FHitResult HitResult;

	FBulletFixture()
		: HitResult(FVector::ZeroVector, FVector::ForwardVector)
	{
		HitResult.ImpactNormal = -FVector::ForwardVector;
		HitResult.Normal = -FVector::ForwardVector;
	}
};

struct FBulletTestResult
{
	double ObjectThickness = 0.0;
	double Velocity = 0.0;
	double ExitVelocity = 0.0;
	bool bIsZero = false;
	double dE = 0.0;
	double DepthOfPenetration = 0.0;
};

BEGIN_DEFINE_SPEC(FBulletCalculateExitVelocitySpec, "Terminal Ballistics.Ballistics.CalculateExitVelocity (Bullet)", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)
public:
	TUniquePtr<FBulletFixture> TestFixture;
protected:
	TFunction<void()> GetBeforeEachFunc()
	{
		return [&]()
		{
			TestFixture = MakeUnique<FBulletFixture>();
		};
	};
	TFunction<void()> GetAfterEachFunc()
	{
		return [&]()
		{
			TestFixture.Reset();
		};
	}

	void TestBullet(const ETBBulletNames NameOfBullet);

	inline static TMap<ETBBulletNames, TMap<FPhysMatProperties, TArray<FBulletTestResult>>> PenetrationResults;
END_DEFINE_SPEC(FBulletCalculateExitVelocitySpec);

void FBulletCalculateExitVelocitySpec::TestBullet(const ETBBulletNames NameOfBullet)
{
	BulletPointer Bullet = MakeShared<FTBBullet>(UBulletUtils::GetFullBulletFromName(NameOfBullet));
	TMap<FPhysMatProperties, TArray<FBulletTestResult>> Results;
	const UTBPhysMatManager& PhysMatManager = UTBPhysMatManager::Get();
	for (const auto& PhysMat : PhysMatManager.GetAllMaterials())
	{
		TArray<FBulletTestResult> PhysMatResults;
		FBulletTestResult BaseTestResult{};
		BaseTestResult.Velocity = UBulletUtils::GetTypicalMuzzleVelocityForBullet(Bullet->BulletType);

		const double StartThickness = 0.001; // 1mm
		const double MaxThickness = 0.5; // 50 cm


		const double DeltaThickness = 0.001; // 1mm step size
		double PenetrationThickness = 0.0;
		while (PenetrationThickness < MaxThickness)
		{
			FBulletTestResult TestResult = BaseTestResult;
			TestResult.ObjectThickness = PenetrationThickness * 100;

			const FVector ExitVelocity = TB::BallisticFunctions::CalculateExitVelocity(TestFixture->HitResult, Bullet, Bullet->MuzzleVelocity * 100.0 * FVector::ForwardVector, PenetrationThickness * 100, PhysMat, TestResult.bIsZero, TestResult.dE, TestResult.DepthOfPenetration);
			TestResult.ExitVelocity = ExitVelocity.Size() / 100.0;
			TestResult.DepthOfPenetration /= 100;

			PhysMatResults.Emplace(TestResult);

			if (TestResult.bIsZero)
			{
				break;
			}

			PenetrationThickness += DeltaThickness;
		}
		Results.Add(PhysMat, PhysMatResults);
	}
	PenetrationResults.Add(NameOfBullet, Results);
}

void FBulletCalculateExitVelocitySpec::Define()
{
	Describe("Functionality", [&]()
		{
			BEFORE_EACH;

			It("Shouldn't allow penetration with zero impact velocity", [&]()
				{
					const double PenetrationThickness = 1.0;
					const UTBPhysMatManager& PhysMatManager = UTBPhysMatManager::Get();
					const TB::Bullets::BulletTypes& BulletTypes = TB::Bullets::BulletTypes::Get();
					BulletPointer Bullet = MakeShared<FTBBullet>(BulletTypes.Bullet_556x45NATO);
					for (const auto& PhysMat : PhysMatManager.GetAllMaterials())
					{
						bool bIsZero = false;
						double dE = 0.0;
						double DepthOfPenetration = 0.0;
						const FVector ExitVelocity = TB::BallisticFunctions::CalculateExitVelocity(TestFixture->HitResult, Bullet, FVector::ZeroVector, PenetrationThickness, PhysMat, bIsZero, dE, DepthOfPenetration);
						TestTrue(FString::Printf(TEXT("Impacts against %s with zero velocity result in no penetration."), *PhysMat.MaterialName.ToString()), ExitVelocity == FVector::ZeroVector && bIsZero);
					}
				}
			);

			
			It("Shouldn't allow penetration of excessively thick objects", [&]()
				{
					const double PenetrationThickness = 100; // 1 meter
					const UTBPhysMatManager& PhysMatManager = UTBPhysMatManager::Get();

					const TB::Bullets::BulletTypes& BulletTypes = TB::Bullets::BulletTypes::Get();

					for (const auto& NameAndBullet : BulletTypes.GetValidBullets())
					{
						BulletPointer Bullet = MakeShared<FTBBullet>(NameAndBullet.Value);
						for (const auto& PhysMat : PhysMatManager.GetAllMaterials())
						{
							bool bIsZero = false;
							double dE = 0.0;
							double DepthOfPenetration = 0.0;
							const FVector ExitVelocity = TB::BallisticFunctions::CalculateExitVelocity(TestFixture->HitResult, Bullet, UBulletUtils::GetTypicalMuzzleVelocityForBullet(Bullet->BulletType) * 100.0 * FVector::ForwardVector, PenetrationThickness, PhysMat, bIsZero, dE, DepthOfPenetration);
							TestFalse(FString::Printf(TEXT("Impacts against %s with a thickness of 1 meter don't result in full penetration. (%s)"), *PhysMat.MaterialName.ToString(), *Bullet->BulletName.ToString()), ExitVelocity.Size() > 0.0 || !bIsZero);
						}
					}
				}
			);

			AFTER_EACH;
		}
	);

	Describe("Test Penetration", [&]()
		{
			It("Clear Results", [&]()
				{
					PenetrationResults.Empty();
				}
			);

			BEFORE_EACH;

			const TB::Bullets::BulletTypes& BulletTypes = TB::Bullets::BulletTypes::Get();
			for (const auto& NameAndBullet : BulletTypes.GetValidBullets())
			{
				It(*StaticEnum<ETBBulletNames>()->GetValueAsString(NameAndBullet.Key), std::bind(&FBulletCalculateExitVelocitySpec::TestBullet, this, NameAndBullet.Key));
			}

			AFTER_EACH;
		}
	);

	Describe("Print Results", [&]()
		{
			It("By Bullet", [&]()
				{
					if (PenetrationResults.IsEmpty())
					{
						AddError("No results.");
					}
					for (const auto& Pair : PenetrationResults)
					{
						FString OutStr = FString::Printf(TEXT("%s:"), *StaticEnum<ETBBulletNames>()->GetValueAsString(Pair.Key));
						OutStr = OutStr.Appendf(TEXT("Velocity: %f m/s"), UBulletUtils::GetFullBulletFromName(Pair.Key).MuzzleVelocity);
						for (const auto& Results : Pair.Value)
						{
							OutStr += "\n--------------------------\n";
							OutStr += Results.Key.MaterialName.ToString();
							OutStr += ": ObjectThickness, ExitVelocity, dV, dE, DepthOfPenetration";
							OutStr += "\n--------------------------";
							for (const auto& Result : Results.Value)
							{
								OutStr = OutStr.Appendf(TEXT("\n%f, %f, %f, %f, %f,"), Result.ObjectThickness, Result.ExitVelocity, Result.Velocity - Result.ExitVelocity, Result.dE, Result.DepthOfPenetration);
							}
							OutStr.RemoveFromEnd(",");
							AddInfo(OutStr);
						}
					}
				}
			);

			It("By Material", [&]()
				{
					if (PenetrationResults.IsEmpty())
					{
						AddError("No results.");
					}

					TMap<FPhysMatProperties, FString> OutStrings;


					for (const auto& Pair : PenetrationResults)
					{
						const FString BulletName = *StaticEnum<ETBBulletNames>()->GetValueAsString(Pair.Key);

						for (const auto& Results : Pair.Value)
						{
							if (!OutStrings.Contains(Results.Key))
							{
								FString Str = "--------------------------\n";
								Str += Results.Key.MaterialName.ToString();
								Str += ": ObjectThickness, ExitVelocity, dV, dE, DepthOfPenetration";
								Str += "\n--------------------------";
								OutStrings.Add(Results.Key, Str);
							}
							for (const auto& Result : Results.Value)
							{
								FString& Str = OutStrings.FindOrAdd(Results.Key);
								Str = Str.Appendf(TEXT("\n%s: %f, %f, %f, %f, %f,"), *BulletName, Result.ObjectThickness, Result.ExitVelocity, Result.Velocity - Result.ExitVelocity, Result.dE, Result.DepthOfPenetration);
							}
						}
					}

					for (auto& Pair : OutStrings)
					{
						Pair.Value.RemoveFromEnd(",");
						AddInfo(Pair.Value);
					}
				}
			);
		}
	);
}

#endif
