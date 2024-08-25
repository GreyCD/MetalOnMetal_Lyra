// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/SKGProjectileWorldSubsystem.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "OnMetalProjectileWorldSubsystem.generated.h"

/**
 * 
 */

UCLASS()
class METALONMETALRUNTIME_API UOnMetalProjectileWorldSubsystem : public USKGProjectileWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void FireProjectile(const int32 ProjectileID, USKGPDAProjectile* DataAsset, const TArray<AActor*>& ActorsToIgnore, const FTransform& LaunchTransform, UPrimitiveComponent* VisualComponentOverride, FSKGOnProjectileImpact OnImpact, FSKGOnProjetilePositionUpdate OnPositionUpdate, FGameplayEffectSpecHandle DamageEffectSpecHandle,AActor* ProjectileOwner) override;

	UFUNCTION(BlueprintCallable, Category = "OnMetal|Projectile")
	void HandleProjectileImpact(const FHitResult& HitResult, const FVector& Direction, const int32 ProjectileID, FGameplayEffectSpecHandle DamageEffectSpecHandle, AActor* ProjectileOwner, FGameplayEffectContextHandle& OutEffectContext, UAbilitySystemComponent*& OutTargetASC) const;
	
	UFUNCTION(BlueprintCallable, Category = "OnMetal|Projectile")
	void OnMetal_ApplyDamageEffect(UPARAM(DisplayName="Target ASC") UAbilitySystemComponent* TargetASC, UPARAM(DisplayName="Effect Context")  FGameplayEffectContextHandle& EffectContext, UPARAM(DisplayName="Damage Effect Class") TSubclassOf<UGameplayEffect> DamageEffectClass, FActiveGameplayEffectHandle& ActiveEffectHandle);

	UPROPERTY()
	FSKGProjectileData OnMetalProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SKGShooterWorldSubsystem|Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
};
