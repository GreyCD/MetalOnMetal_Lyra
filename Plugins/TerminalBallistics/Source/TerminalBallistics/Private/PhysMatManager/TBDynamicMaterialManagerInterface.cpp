// Copyright Erik Hedberg. All rights reserved.

#include "PhysMatManager/TBDynamicMaterialManagerInterface.h"
#include "Engine/HitResult.h"
#include "PhysMatManager/PhysMat.h"

namespace TB
{
	bool Private::_ImplementsDynamicMaterialPropertyManager(UObject* Object)
	{
		if(Object)
		{
			return Object->Implements<UTBDynamicMaterialPropertyManager>() || Cast<ITBDynamicMaterialPropertyManager>(Object);
		}
		else
		{
			return false;
		}
	}

	void DispatchMaterialDamageEvent(const bool bYielded, const double ImpartedEnergy, const FHitResult& HitResult)
	{
		if (ImplementsDynamicMaterialPropertyManager(HitResult.GetComponent()))
		{
			ITBDynamicMaterialPropertyManager::Execute_OnDamaged((UObject*)HitResult.GetComponent(), bYielded, ImpartedEnergy, HitResult);
		}
		if (ImplementsDynamicMaterialPropertyManager(HitResult.GetActor()))
		{
			ITBDynamicMaterialPropertyManager::Execute_OnDamaged((UObject*)HitResult.GetActor(), bYielded, ImpartedEnergy, HitResult);
		}
	}
};
