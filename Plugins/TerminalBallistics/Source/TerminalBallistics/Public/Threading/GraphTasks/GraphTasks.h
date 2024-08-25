// Copyright Erik Hedberg. All Rights Reserved.

#pragma once

#include "Async/TaskGraphInterfaces.h"
#include "Misc/TBLogChannels.h"
#include "TaskParam.h"
#include "TBTaskGuard.h"
#include "Templates/Function.h"



namespace TB::Configuration
{
	extern TERMINALBALLISTICS_API bool bIgnoreImpactEventsWithInvalidData;
};

namespace TB::SimTasks
{

	/* Allows for an optional condition to be attached to a task. Will prevent task execution if the condition evaluates to true. */
	class FTaskWithCheckValue
	{
	public:
		FTaskWithCheckValue(bool* CheckValue);

		/* If this returns false, do not procceed with task execution. */
		bool Check() const;
	protected:
		bool* CheckValue;
		bool HasCheck = false;
	};
	
	class FLambdaTask : public FPendingTask, public FTaskWithCheckValue
	{
	public:
		using GraphTask = TGraphTask<FLambdaTask>;

		FLambdaTask(TUniqueFunction<void()> Function, FPendingTaskSynch* SynchObject = nullptr, bool* CheckValue = nullptr)
			: FPendingTask(SynchObject)
			, FTaskWithCheckValue(CheckValue)
			, Function(MoveTemp(Function))
		{}

		virtual ~FLambdaTask() = default;

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FLambdaTask, STATGROUP_TaskGraphTasks);
		}

		FORCEINLINE static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::GameThread;
		}

		virtual void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			DoTaskImmediate();
		}

		virtual void DoTaskImmediate()
		{
			if (!Check())
			{
				return;
			}
			::Invoke(Function);
		}

		TUniqueFunction<void()> Function;
	};


	/**
	* Generic task to broadcast a multicast delegate with no parameters
	*/
	template<typename DelegateType>
	class TDelegateBroadcastTask : public FTaskWithCheckValue
	{
	public:
		using GraphTask = TGraphTask<TDelegateBroadcastTask<DelegateType>>;

		TDelegateBroadcastTask(DelegateType Delegate, FPendingTaskSynch* SynchObject = nullptr, bool* CheckValue = nullptr)
			: FTaskWithCheckValue(CheckValue)
			, Delegate(Delegate)
		{}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(TDelegateBroadcastTask, STATGROUP_TaskGraphTasks);
		}

		FORCEINLINE static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::GameThread;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			DoTaskImmediate();
		}

		virtual void DoTaskImmediate()
		{
			if (!Check())
			{
				return;
			}
			Delegate.Broadcast();
		}

		DelegateType Delegate;
	};
	

	/**
	* Generic task to broadcast a multicast delegate with one parameter
	*/
	template<typename DelegateType, typename ParamType>
	class TDelegateBroadcastTask_OneParam : public FPendingTask, public FTaskWithCheckValue
	{
	public:
		using GraphTask = TGraphTask<TDelegateBroadcastTask_OneParam<DelegateType, ParamType>>;

		TDelegateBroadcastTask_OneParam(DelegateType Delegate, ParamType InParam, FPendingTaskSynch* SynchObject = nullptr, bool* CheckValue = nullptr)
			: FPendingTask(SynchObject)
			, FTaskWithCheckValue(CheckValue)
			, Delegate(Delegate)
			, Param(InParam)
		{}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(TDelegateBroadcastTask_OneParam, STATGROUP_TaskGraphTasks);
		}

		FORCEINLINE static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::GameThread;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			DoTaskImmediate();
		}

		virtual void DoTaskImmediate()
		{
			if (!Check())
			{
				return;
			}
			if (!Param.IsValid())
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TDelegateBroadcastTask_OneParam::DoTask::Invalid);
				TRACE_BOOKMARK(TEXT("InvalidTaskParam"));
				UE_LOG(LogTerminalBallistics, Error, TEXT("Invalid TTaskParam"));
				if (TB::Configuration::bIgnoreImpactEventsWithInvalidData)
				{
					return;
				}
				else
				{
					Delegate.Broadcast(Param.GetDefault());
				}
			}
			else
			{
				Delegate.Broadcast(Param.GetValue());
			}
		}


		DelegateType Delegate;
		TTaskParam<ParamType> Param;
	};

	/**
	* Generic task to broadcast a multicast delegate with two parameters
	*/
	template<typename DelegateType, typename Param1Type, typename Param2Type>
	class TDelegateBroadcastTask_TwoParams : public FPendingTask, public FTaskWithCheckValue
	{
	public:
		using GraphTask = TGraphTask<TDelegateBroadcastTask_TwoParams<DelegateType, Param1Type, Param2Type>>;

		TDelegateBroadcastTask_TwoParams(DelegateType Delegate, Param1Type Param1, Param2Type Param2, FPendingTaskSynch* SynchObject = nullptr, bool* CheckValue = nullptr)
			: FPendingTask(SynchObject)
			, FTaskWithCheckValue(CheckValue)
			, Delegate(Delegate)
			, Param1(Param1)
			, Param2(Param2)
		{}

		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(TDelegateBroadcastTask_TwoParams, STATGROUP_TaskGraphTasks);
		}

		FORCEINLINE static ESubsequentsMode::Type GetSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}

		static ENamedThreads::Type GetDesiredThread()
		{
			return ENamedThreads::GameThread;
		}

		void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
		{
			DoTaskImmediate();
		}

		void DoTaskImmediate()
		{
			if (!Check())
			{
				return;
			}
			if (!Param1.IsValid() || !Param2.IsValid())
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(TDelegateBroadcastTask_TwoParams::DoTask::Invalid);
				TRACE_BOOKMARK(TEXT("InvalidTaskParam"));
				UE_LOG(LogTerminalBallistics, Error, TEXT("Invalid TTaskParam"));
				if(TB::Configuration::bIgnoreImpactEventsWithInvalidData)
				{
					return;
				}
				else
				{
					const bool Param1IsValid = Param1.IsValid();
					const bool Param2IsValid = Param2.IsValid();
					Delegate.Broadcast(
						Param1IsValid ? Param1.GetValue() : Param1.GetDefault(),
						Param2IsValid ? Param2.GetValue() : Param2.GetDefault()
					);
				}
			}
			else
			{
				Delegate.Broadcast(Param1.GetValue(), Param2.GetValue());
			}
		}

		DelegateType Delegate;
		TTaskParam<Param1Type> Param1;
		TTaskParam<Param2Type> Param2;
	};

	template<typename TaskType, typename... Args>
	void InitiateTask(bool bImmediate, Args&&... args)
	{
		if (bImmediate)
		{
			TaskType Task(std::forward<Args>(args)...);
			Task.DoTaskImmediate();
		}
		else
		{
			TaskType::GraphTask::CreateTask().ConstructAndDispatchWhenReady(std::forward<Args>(args)...);
		}
	}
};

/**
 * Convenience function for executing code asynchronously on the Game Thread.
 *
 * @param Function The function to execute.
*/
void GameThreadTask(TUniqueFunction<void()> Function, TB::SimTasks::FPendingTaskSynch* SynchObject = nullptr);
