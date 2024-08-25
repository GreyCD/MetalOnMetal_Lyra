// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/TBTraits.h"
#include "Misc/Optional.h"
#include "Delegates/Delegate.h"
#include <functional>
#include <type_traits>

#define _MakeDelegateParams(FuncPtr) FuncPtr, STATIC_FUNCTION_FNAME(TEXT(#FuncPtr))
#define _OptionalDelegateAdd(Obj, FuncPtr) __Add(Obj, _MakeDelegateParams(FuncPtr))

template<typename delegate_type>
struct TOptionalDelegate
{
private:
	inline static constexpr bool IsDynamicDelegate = std::is_base_of_v<TScriptDelegate<FWeakObjectPtr>, delegate_type>;
	inline static constexpr bool IsDynamicMulticastDelegate = std::is_base_of_v<TMulticastScriptDelegate<FWeakObjectPtr>, delegate_type>;
public:
	using DelegateType = delegate_type;
	TOptional<DelegateType> Delegate;

	TOptionalDelegate() = default;
	TOptionalDelegate(DelegateType&& Delegate)
		: Delegate(Delegate)
	{}
	TOptionalDelegate(const DelegateType& Delegate)
		: Delegate(Delegate)
	{}

	bool IsSet() const
	{
		return Delegate.IsSet();
	}

	void Reset()
	{
		Delegate.Reset();
	}

	DelegateType& Get()
	{
		return Delegate.GetValue();
	}

	operator DelegateType() const
	{
		return Delegate.Get(DelegateType());
	}
	
	template<typename UserObject, typename T>
	void __Add(UserObject* Obj, T FuncPtr, const FName& FuncName)
	{
		if constexpr (IsDynamicDelegate)
		{
			if (Delegate.IsSet())
			{
				Delegate.GetValue().__Internal_BindDynamic(Obj, FuncPtr, FuncName);
			}
		}
		else if constexpr (IsDynamicMulticastDelegate)
		{
			if (Delegate.IsSet())
			{
				Delegate.GetValue().__Internal_AddDynamic(Obj, FuncPtr, FuncName);
			}
		}
	}

	void Clear()
	{
		CallIfPossible(&DelegateType::Clear);
	}

	bool IsBound()
	{
		if constexpr (TB::Traits::THasFunction(&DelegateType::IsBound))
		{
			return CallIfPossible(&DelegateType::IsBound);
		}
		else
		{
			return false;
		}
	}

	template<typename... Args>
	void Broadcast(Args&&... args)
	{
		if constexpr (TB::Traits::THasFunction(&DelegateType::Broadcast))
		{
			CallIfPossible(&DelegateType::Broadcast, std::forward<Args>(args)...);
		}
	}

	template<typename... Args>
	void Execute(Args&&... args)
	{
		if constexpr (TB::Traits::THasFunction(&DelegateType::Execute))
		{
			CallIfPossible(&DelegateType::Execute, std::forward<Args>(args)...);
		}
	}

	template<typename... Args>
	void ExecuteIfBound(Args&&... args)
	{
		if constexpr (TB::Traits::THasFunction(&DelegateType::ExecuteIfBound))
		{
			CallIfPossible(&DelegateType::ExecuteIfBound, std::forward<Args>(args)...);
		}
	}

protected:
	template<typename Function, typename... Args>
	auto CallIfPossible(Function func, Args ...args)
	{
		using ReturnType = std::invoke_result_t<decltype(std::bind(std::declval<Function>(), std::declval<DelegateType>(), std::declval<Args>()...))>;
		if (Delegate.IsSet())
		{
			return (Delegate.GetValue().*func)(args...);
		}
		else
		{
			return ReturnType();
		}
	}
};
