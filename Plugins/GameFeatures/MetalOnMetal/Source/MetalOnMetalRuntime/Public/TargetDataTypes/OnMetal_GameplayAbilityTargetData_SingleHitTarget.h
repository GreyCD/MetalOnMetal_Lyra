#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Types/TBProjectileId.h"

#include "OnMetal_GameplayAbilityTargetData_SingleHitTarget.generated.h"

class FArchive;
struct FGameplayEffectContextHandle;

USTRUCT()
struct METALONMETALRUNTIME_API FOnMetal_GameplayAbilityTargetData_SingleHitTarget : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	 FOnMetal_GameplayAbilityTargetData_SingleHitTarget()
	 : CartridgeID(-1)
	{ }
	
	
	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID;
	
	UPROPERTY()
	bool bHitConfirmed = false; // This is set to true when the hit is confirmed by the server
	
	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FOnMetal_GameplayAbilityTargetData_SingleHitTarget::StaticStruct();
	}
	
	
};

template<>
struct TStructOpsTypeTraits<FOnMetal_GameplayAbilityTargetData_SingleHitTarget> : public TStructOpsTypeTraitsBase2<FOnMetal_GameplayAbilityTargetData_SingleHitTarget>
{
	enum
	{
		WithNetSerializer = true,	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};
