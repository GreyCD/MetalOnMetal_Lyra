// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "OnMetal_ProjectileImpactAbility.generated.h"

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_ProjectileImpactAbility : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UOnMetal_ProjectileImpactAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;
	
};
