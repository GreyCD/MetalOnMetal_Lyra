// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "TBDynamicMaterialManagerInterface.generated.h"


UINTERFACE(BlueprintType)
class TERMINALBALLISTICS_API UTBDynamicMaterialPropertyManager : public UInterface
{
	GENERATED_BODY()
};
class ITBDynamicMaterialPropertyManager
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Terminal Ballistics", meta = (AdvancedDisplay = "HitResult", AutoCreateRefTerm = "HitResult"))
	struct FPhysMatProperties GetMaterialProperties(const struct FHitResult& HitResult);

	/*
	* @param bYielded			Whether or not the damage event caused material failure (yielding)
	* @param ImpartedEnergy		Amount of energy imparted during damage event (J)
	* @param HitResult			HitResult instigating this damage event
	*/
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Terminal Ballistics", meta = (ForceAsFunction))
	void OnDamaged(const bool bYielded, const double ImpartedEnergy, const struct FHitResult& HitResult);
};

namespace TB
{
	void DispatchMaterialDamageEvent(const bool bYielded, const double ImpartedEnergy, const struct FHitResult& HitResult);

	namespace Private
	{
		bool _ImplementsDynamicMaterialPropertyManager(UObject* Object);
	};

	template<typename T>
	bool ImplementsDynamicMaterialPropertyManager(T* Object)
	{
		return Private::_ImplementsDynamicMaterialPropertyManager((UObject*)Object);
	}
}
