// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "Misc/Optional.h"
#include "Templates/Function.h"
#include "Types/TBImpactParams.h"
#include "Types/TBProjectileFlightData.h"
#include "Types/TBSimData.h"
#include "Types/TBSimTaskDelegates.h"


namespace TB::SimTasks
{
	typedef TSimTaskDelegates<FOnComplete, FOnBulletHit, FOnBulletExitHit, FOnBulletInjure, FOnUpdate> FBulletTaskDelegates;
	typedef TSimTaskDelegates<FOnComplete, FOnProjectileHit, FOnProjectileExitHit, FOnProjectileInjure, FOnUpdate> FProjectileTaskDelegates;
};
namespace TB::Traits
{
	template<> struct TIsSimTaskDelegateStruct<TB::SimTasks::FBulletTaskDelegates>
	{
		enum
		{
			Value = true
		};
	};
	template<> struct TIsSimTaskDelegateStruct<TB::SimTasks::FProjectileTaskDelegates>
	{
		enum
		{
			Value = true
		};
	};
};

/* Wrapper struct containing delegates to be bound to a projectile task. */
template<typename CompleteDelegateType, typename HitDelegateType, typename ExitHitDelegateType, typename InjureDelegateType, typename UpdateDelegateType, typename NativeDelegatesStruct>
struct TTBTaskCallbacks
{
	typedef TB::SimTasks::TSimTaskDelegates<CompleteDelegateType, HitDelegateType, ExitHitDelegateType, InjureDelegateType, UpdateDelegateType> FDelegatesBP;

	FDelegatesBP DelegatesBP;
	NativeDelegatesStruct NativeDelegates;

	TOptional<TFunction<void(const FTBProjectileFlightData&)>> OnUpdateFunction;

	TTBTaskCallbacks() = default;

	TTBTaskCallbacks(
		CompleteDelegateType CompleteDelegate,
		HitDelegateType HitDelegate,
		ExitHitDelegateType ExitHitDelegate,
		InjureDelegateType InjureDelegate,
		UpdateDelegateType UpdateDelegate,
		NativeDelegatesStruct NativeDelegates = NativeDelegatesStruct(),
		decltype(OnUpdateFunction) onUpdateFunction = TOptional<TFunction<void(const FTBProjectileFlightData&)>>()
	)
		: DelegatesBP(CompleteDelegate, HitDelegate, ExitHitDelegate, InjureDelegate, UpdateDelegate)
		, NativeDelegates(NativeDelegates)
		, OnUpdateFunction(onUpdateFunction)
	{}

	/* Construct from BP delegates */
	TTBTaskCallbacks(FDelegatesBP DelegatesBP)
		: DelegatesBP(DelegatesBP)
	{}

	/* Construct from native delegates */
	TTBTaskCallbacks(NativeDelegatesStruct NativeDelegates)
		: NativeDelegates(NativeDelegates)
	{}

	~TTBTaskCallbacks()
	{
		Reset();
	}

	TTBTaskCallbacks(const FDelegatesBP& DelegatesBP, const NativeDelegatesStruct& NativeDelegates, TOptional<TFunction<void(const FTBProjectileFlightData&)>> OnUpdateFunction)
		: DelegatesBP(DelegatesBP)
		, NativeDelegates(NativeDelegates)
		, OnUpdateFunction(OnUpdateFunction)
	{}

	void Reset() noexcept
	{
		DelegatesBP.Clear();
		NativeDelegates.Clear();
		if (OnUpdateFunction.IsSet())
		{
			OnUpdateFunction.GetValue().Reset();
		}
	}
};
typedef TTBTaskCallbacks<FBPOnProjectileComplete, FBPOnBulletHit, FBPOnBulletExitHit, FBPOnBulletInjure, FBPOnProjectileUpdate, TB::SimTasks::FBulletTaskDelegates> FTBBulletTaskCallbacks;
typedef TTBTaskCallbacks<FBPOnProjectileComplete, FBPOnProjectileHit, FBPOnProjectileExitHit, FBPOnProjectileInjure, FBPOnProjectileUpdate, TB::SimTasks::FProjectileTaskDelegates> FTBProjectileTaskCallbacks;