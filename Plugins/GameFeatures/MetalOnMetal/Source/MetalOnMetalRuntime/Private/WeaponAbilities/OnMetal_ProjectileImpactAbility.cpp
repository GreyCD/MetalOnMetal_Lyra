// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAbilities/OnMetal_ProjectileImpactAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

UOnMetal_ProjectileImpactAbility::UOnMetal_ProjectileImpactAbility(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void UOnMetal_ProjectileImpactAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    /*// Check for valid target data
    if (ActivationInfo.TargetData.IsValid() && ActivationInfo.TargetData.Num() > 0)
    {
        // Cast to your specific target data struct (assuming you have one)
        if (const FGameplayAbilityTargetData_SingleTargetHit* HitTargetData = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(ActivationInfo.TargetData.Get(0)))
        {
            AActor* TargetActor = HitTargetData->HitResult.GetActor();
            const FHitResult& HitResult = HitTargetData->HitResult;

            // Get the Ability System Components (ASC)
            UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get(); // Get Source ASC from ActorInfo
            UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);

            if (SourceASC && TargetASC)
            {
                // Apply Gameplay Effects (Replace with your actual Gameplay Effect classes)
                FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(UDamageGameplayEffect::StaticClass());
                FGameplayEffectContextHandle EffectContext = DamageEffectSpecHandle.Data.Get()->MakeEffectContext();
                EffectContext.AddHitResult(HitResult); // Add the hit result for context
                // Apply the effect from the source to the target
                SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC, EffectContext);

                // (Optional) Apply additional gameplay effects based on specific conditions
                // For example, different effects for headshots, critical hits, etc.
            }
            else
            {
                // Handle cases where either ASC is not found (log a warning or take other action)
                UE_LOG(LogTemp, Warning, TEXT("OnMetal_ProjectileImpactAbility::ActivateAbility: Source or Target ASC not found"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("OnMetal_ProjectileImpactAbility::ActivateAbility: TargetData is invalid or empty"));
    }*/
    UE_LOG(LogTemp, Warning, TEXT("OnMetal_ProjectileImpactAbility::ActivateAbility: AbilityActivated"));
    // End the ability
    EndAbility(CurrentSpecHandle, CurrentActorInfo, ActivationInfo, true, false);
}
