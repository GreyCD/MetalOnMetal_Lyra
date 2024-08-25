// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "Weapons/LyraGameplayAbility_RangedWeapon.h"
#include "OnMetal_ProjectileWeaponAbility.generated.h"

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetalProjectileComplete, const FTBProjectileId, ProjectileId, const TArray<FPredictProjectilePathPointData>&, PathResults);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMetalBulletHit, const FTBImpactParams&, ImpactParams);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMetalBulletExitHit, const FTBImpactParams&, ImpactParams);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetalBulletInjure, const FTBImpactParams&, ImpactParams, const FTBProjectileInjuryParams&, InjuryParams);

USTRUCT(BlueprintType)
struct FOnMetalLaunchParams : public FTBLaunchParams
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

    UPROPERTY()
    FGameplayAbilitySpecHandle AbilityHandle;

    UPROPERTY()
    FGameplayEffectContextHandle EffectContext;

    FOnMetalLaunchParams() : Super() {}

    FOnMetalLaunchParams(const FTBLaunchParams& BaseLaunchParams)
       : Super(BaseLaunchParams)
    {}
};

UCLASS()
class UOnMetalTBProjectilePayload : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

    UPROPERTY()
    FGameplayAbilitySpecHandle AbilityHandle;

    UPROPERTY()
    FGameplayEffectContextHandle EffectContext;

    //virtual FGameplayAbilityTargetDataHandle CreateTargetData(const FTBImpactParams& ImpactParams) const;
};

UCLASS()
class METALONMETALRUNTIME_API UOnMetal_ProjectileWeaponAbility : public ULyraGameplayAbility_RangedWeapon
{
    GENERATED_BODY()

public:
    // Constructor
    UOnMetal_ProjectileWeaponAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    /*
    // Blueprint Events
    UPROPERTY(BlueprintAssignable, Category = "OnMetal|Ability")
    FOnMetalProjectileComplete OnProjectileComplete;

    UPROPERTY(BlueprintAssignable, Category = "OnMetal|Ability")
    FOnMetalBulletHit OnProjectileHit;

    UPROPERTY(BlueprintAssignable, Category = "OnMetal|Ability")
    FOnMetalBulletExitHit OnProjectileExitHit;

    UPROPERTY(BlueprintAssignable, Category = "OnMetal|Ability")
    FOnMetalBulletInjure OnProjectileInjure;
    */

    // Override Lyra's functions as needed
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
    
    void OnBulletHit(const FTBImpactParams& ImpactParams);

    // Terminal Ballistics Functions
    UFUNCTION(BlueprintCallable, Category = "OnMetal|Ability")
    virtual void FireTBProjectile();

    UFUNCTION(BlueprintCallable, Category = "OnMetal|Ability")
    FTBLaunchParams CreateTBLaunchParams() const;

protected:
    // Terminal Ballistics Delegates
    FBPOnBulletHit OnBulletHitDelegate;
    FBPOnBulletExitHit OnBulletExitHitDelegate;
    FBPOnBulletInjure OnBulletInjureDelegate;
    FBPOnProjectileComplete OnBulletCompleteDelegate;



private:
    // ... (Your existing private members, including ActiveProjectiles)
    TMap<FTBProjectileId, FOnMetalLaunchParams> ActiveProjectiles;
    void BindTBDelegates();
};
