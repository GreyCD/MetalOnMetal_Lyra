

#include "TargetDataTypes/OnMetal_GameplayAbilityTargetData_SingleHitTarget.h"
#include "AbilitySystem//LyraGameplayEffectContext.h"
#include "Types/TBProjectileId.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_GameplayAbilityTargetData_SingleHitTarget)

struct FGameplayEffectContextHandle;

void FOnMetal_GameplayAbilityTargetData_SingleHitTarget::AddTargetDataToContext(FGameplayEffectContextHandle& Context,
	bool bIncludeActorArray) const
{
	FGameplayAbilityTargetData_SingleTargetHit::AddTargetDataToContext(Context, bIncludeActorArray);
	// Add game-specific data
	if (FLyraGameplayEffectContext* TypedContext = FLyraGameplayEffectContext::ExtractEffectContext(Context))
	{
		TypedContext->CartridgeID = CartridgeID;

	}
}

bool FOnMetal_GameplayAbilityTargetData_SingleHitTarget::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayAbilityTargetData_SingleTargetHit::NetSerialize(Ar, Map, bOutSuccess);
	
	Ar << CartridgeID;
	
	return true;
}
