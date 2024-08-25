// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "OnMetal_SimpleProjectileAbility.generated.h"

class UGameplayEffect;

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_SimpleProjectileAbility : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	UOnMetal_SimpleProjectileAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FTransform& SpawnTransform);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AOnMetal_SimpleProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
};
