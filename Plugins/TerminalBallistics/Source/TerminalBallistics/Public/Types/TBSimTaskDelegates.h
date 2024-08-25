// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "Types/TBOptionalDelegate.h"
#include "Types/TBProjectileFlightData.h"

namespace TB::SimTasks
{
	/* Template struct used to define and store delegates for use in TProjectileSimulationTask */
	template<typename CompleteDelegateType, typename HitDelegateType, typename ExitHitDelegateType, typename InjureDelegateType, typename UpdateDelegateType, typename FlightDataType = FTBProjectileFlightData>
	struct TSimTaskDelegates
	{
		using CompleteDelegate = CompleteDelegateType;
		using HitDelegate = HitDelegateType;
		using ExitHitDelegate = ExitHitDelegateType;
		using InjureDelegate = InjureDelegateType;
		using UpdateDelegate = UpdateDelegateType;
		using UpdateParamType = FlightDataType;

		TOptionalDelegate<CompleteDelegate>				OnComplete;
		TOptionalDelegate<HitDelegate>					OnHit;
		TOptionalDelegate<ExitHitDelegate>				OnExitHit;
		TOptionalDelegate<InjureDelegate>				OnInjure;
		TOptionalDelegate<UpdateDelegate>				OnUpdate;

		TSimTaskDelegates() = default;

		TSimTaskDelegates(EForceInit)
			: OnComplete(CompleteDelegate())
			, OnHit(HitDelegate())
			, OnExitHit(ExitHitDelegate())
			, OnInjure(InjureDelegate())
			, OnUpdate(UpdateDelegate())
		{}

		TSimTaskDelegates(CompleteDelegate OnComplete, HitDelegate OnHit, ExitHitDelegate OnExitHit, InjureDelegate OnInjure, UpdateDelegate OnUpdate)
			: OnComplete(OnComplete)
			, OnHit(OnHit)
			, OnExitHit(OnExitHit)
			, OnInjure(OnInjure)
			, OnUpdate(OnUpdate)
		{}

		template<typename UserObject, typename CompleteFuncPtr, typename HitFuncPtr, typename ExitHitFuncPtr, typename InjureFuncPtr, typename UpdateFuncPtr>
		TSimTaskDelegates(
			UserObject* Obj,
			CompleteFuncPtr completeFuncPtr, const FName& completeFuncName,
			HitFuncPtr hitFuncPtr, const FName& hitFuncName,
			ExitHitFuncPtr exitHitFuncPtr, const FName& exitHitFuncName,
			InjureFuncPtr injureFuncPtr, const FName& injureFuncName,
			UpdateFuncPtr updateFuncPtr, const FName& updateFuncName = NAME_None
		)
			: TSimTaskDelegates(ForceInit)
		{
			OnComplete.__Add(Obj, completeFuncPtr, completeFuncName);
			OnHit.__Add(Obj, hitFuncPtr, hitFuncName);
			OnExitHit.__Add(Obj, exitHitFuncPtr, exitHitFuncName);
			OnInjure.__Add(Obj, injureFuncPtr, injureFuncName);
			if (updateFuncName != NAME_None)
			{
				OnUpdate.__Add(Obj, updateFuncPtr, updateFuncName);
			}
			else
			{
				OnUpdate.Reset();
			}
		}

		FORCEINLINE void Clear()
		{
			OnComplete.Clear();
			OnComplete.Clear();
			OnHit.Clear();
			OnExitHit.Clear();
			OnInjure.Clear();
			OnUpdate.Clear();
		}

		~TSimTaskDelegates()
		{
			Clear();
		}
	};
};
