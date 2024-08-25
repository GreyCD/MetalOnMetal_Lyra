// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/Object.h"
#include "GameplayEffectTypes.h"
#include "TargetDataTypes/OnMetal_GameplayAbilityTargetData_SingleHitTarget.h"
#include "OnMetal_TBPayload.generated.h"

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_TBPayload : public UObject
{
	GENERATED_BODY()

public:
	 UPROPERTY()
	 TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	//
	// UPROPERTY()
	// FGameplayAbilitySpecHandle AbilityHandle;
	//
	// UPROPERTY()
	// FGameplayEffectContextHandle EffectContext;

	UPROPERTY()
	TWeakObjectPtr<AActor> OwningActor;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "Projectile")
	FGameplayEffectSpecHandle DamageEffectSpecHandle;

	UPROPERTY()
	FGameplayTag ApplicationTag;

	UPROPERTY()
	FOnMetal_GameplayAbilityTargetData_SingleHitTarget CustomTargetData;

	void InitializePayload(UAbilitySystemComponent* InSourceASC, const FGameplayAbilitySpecHandle& InAbilityHandle, const FGameplayEffectContextHandle& InEffectContext, AActor* InOwningActor, const FGameplayTag& InApplicationTag);

	bool IsValid() const;
	bool HasAuthority() const;

	// UFUNCTION(BlueprintCallable, Category = "OnMetal|Projectile")
	// FGameplayAbilityTargetDataHandle CreateTargetDataHandleFromImpact(const FHitResult& HitResult, const FTBProjectileId& ProjectileId) const;
};
