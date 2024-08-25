// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "Misc/CoreMiscDefines.h"
#include <atomic>

namespace TB::SimTasks
{
	class FPendingTaskSynch
	{
	public:
		FPendingTaskSynch() = default;
		UE_NONCOPYABLE(FPendingTaskSynch)

		bool IsLocked() const
		{
			return NumPending > 0;
		}

		bool Acquire()
		{
			if (!IsLocked() && !Acquired)
			{
				Acquired = true;
				return true;
			}
			return false;
		}

		void Release()
		{
			Acquired = false;
		}
	protected:
		friend class FPendingTaskGuard;
		void TaskAdded()
		{
			NumPending += !Acquired;
		}
		void TaskCompleted()
		{
			NumPending -= !Acquired;
		}

		std::atomic_bool Acquired = false;
		std::atomic<int> NumPending = 0;
	};

	/*
	* Scoped guard that increments a counter in a PendingTaskSynch when constructed, and decrements it when destroyed.
	* Intended for use in tasks sent to the task graph to track of their lifecycle.
	*/
	class FPendingTaskGuard
	{
	public:
		FPendingTaskGuard(FPendingTaskSynch* SynchObject)
			: SynchObject(SynchObject)
		{
			if (SynchObject)
			{
				SynchObject->TaskAdded();
			}
		}

		void Release()
		{
			if (SynchObject)
			{
				SynchObject->TaskCompleted();
				SynchObject = nullptr;
			}
		}

		~FPendingTaskGuard()
		{
			Release();
		}
	protected:
		FPendingTaskSynch* SynchObject;
	};

	class FPendingTask
	{
	public:
		FPendingTask(FPendingTaskSynch* SynchObject)
			: Guard(SynchObject)
		{}

		virtual ~FPendingTask() = default;
	protected:
		FPendingTaskGuard Guard;
	};
};
