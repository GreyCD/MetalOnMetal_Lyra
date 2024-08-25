// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/OnMetal_TBPayload.h"
#include "Weapons/OnMetalProjectileTypes.h"

#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"

/*void UOnMetal_TBPayload::InitializePayload(UAbilitySystemComponent* InSourceASC,
                                           const FGameplayAbilitySpecHandle& InAbilityHandle, const FGameplayEffectContextHandle& InEffectContext,
                                           AActor* InOwningActor, const FGameplayTag& InApplicationTag)
{

	SourceASC = InSourceASC;
	AbilityHandle = InAbilityHandle;
	EffectContext = InEffectContext;
	OwningActor = InOwningActor;
	ApplicationTag = InApplicationTag;
}

bool UOnMetal_TBPayload::IsValid() const
{
	return SourceASC.IsValid() && AbilityHandle.IsValid() && OwningActor.IsValid();
}*/

bool UOnMetal_TBPayload::HasAuthority() const
{
	return OwningActor.IsValid() && OwningActor->HasAuthority();
}

/*FGameplayAbilityTargetDataHandle UOnMetal_TBPayload::CreateTargetDataHandleFromImpact(const FHitResult& HitResult,
	const FTBProjectileId& ProjectileId) const
{
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	if (!IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateTargetDataHandleFromImpact: Payload is not valid"));
		return TargetDataHandle;
	}

	FLyraGameplayAbilityTargetData_SingleTargetHit* TargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
	TargetData->HitResult = HitResult;
	TargetData->CartridgeID = GetTypeHash(ProjectileId);
	// TargetData->SourceASC = SourceASC;
	// TargetData->AbilityHandle = AbilityHandle;
	// TargetData->EffectContext = EffectContext;

	TargetDataHandle.Add(TargetData);
	return TargetDataHandle;
}*/
