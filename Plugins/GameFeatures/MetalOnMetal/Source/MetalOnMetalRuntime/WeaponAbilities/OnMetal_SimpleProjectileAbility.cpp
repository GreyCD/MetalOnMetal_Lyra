// Fill out your copyright notice in the Description page of Project Settings.


#include "OnMetal_SimpleProjectileAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "OnMetal_SimpleProjectile.h"

UOnMetal_SimpleProjectileAbility::UOnMetal_SimpleProjectileAbility(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

void UOnMetal_SimpleProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                       const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UOnMetal_SimpleProjectileAbility::SpawnProjectile(const FTransform& SpawnTransform)
{
	if (!ProjectileClass) return;

	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	for (int32 i = 0; i < NumProjectiles; i++)
	{
		const FTransform SpawnedTransform = SpawnTransform;
		AOnMetal_SimpleProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AOnMetal_SimpleProjectile>(ProjectileClass,
			SpawnedTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, SourceASC->MakeEffectContext());
		SpawnedProjectile->DamageEffectSpecHandle = SpecHandle;
		

		SpawnedProjectile->FinishSpawning(SpawnedTransform);
	}
}


	

