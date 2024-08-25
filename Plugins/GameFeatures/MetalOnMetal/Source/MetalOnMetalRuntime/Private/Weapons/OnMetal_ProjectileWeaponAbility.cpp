// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OnMetal_ProjectileWeaponAbility.h"

#include "AbilitySystemComponent.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "Weapons/LyraWeaponStateComponent.h"


UOnMetal_ProjectileWeaponAbility::UOnMetal_ProjectileWeaponAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UOnMetal_ProjectileWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Get the owning actor and its Ability System Component
	AActor* Owner = ActorInfo->OwnerActor.Get();
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!Owner || !ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UOnMetal_ProjectileWeaponAbility::ActivateAbility: Owner or ASC is invalid!"));
		return;
	}

	// Get the Lyra Weapon State Component
	ULyraWeaponStateComponent* WeaponStateComponent = Owner->FindComponentByClass<ULyraWeaponStateComponent>();
	if (!WeaponStateComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UOnMetal_ProjectileWeaponAbility::ActivateAbility: LyraWeaponStateComponent not found!"));
		return;
	}

	// Get the current ranged weapon instance
	ULyraRangedWeaponInstance* CurrentWeapon = Cast<ULyraRangedWeaponInstance>(GetAssociatedEquipment());
	if (!CurrentWeapon)
	{
		UE_LOG(LogTemp, Error, TEXT("UOnMetal_ProjectileWeaponAbility::ActivateAbility: No ranged weapon equipped!"));
		return;
	}
	
	/*// Get the launch parameters (you'll need to implement this function)
	FOnMetalLaunchParams LaunchParams = CreateTBLaunchParams();

	// Set additional parameters for the projectile
	LaunchParams.SourceASC = ASC;
	LaunchParams.AbilityHandle = Handle;
	LaunchParams.EffectContext = ActivationInfo.GetEffectContext();

	// Fire the projectile using Terminal Ballistics
	FTBProjectileId ProjectileId = Weapon->FireProjectile(LaunchParams);

	// Store the projectile ID and launch parameters for later reference
	if (ProjectileId.IsValid())
	{
		ActiveProjectiles.Add(ProjectileId, LaunchParams);
	}*/

	// Bind Terminal Ballistics delegates
	BindTBDelegates();

	// ... (Your existing logic for firing projectiles)
}

void UOnMetal_ProjectileWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Unbind Terminal Ballistics delegates
	OnBulletCompleteDelegate.Unbind();
	OnBulletHitDelegate.Unbind();
	OnBulletExitHitDelegate.Unbind();
	OnBulletInjureDelegate.Unbind();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

// ... (Other functions: CreateTBLaunchParams, FireTBProjectile, etc.)

void UOnMetal_ProjectileWeaponAbility::OnBulletHit(const FTBImpactParams& ImpactParams)
{
	// 1. Retrieve Payload and Source Ability System Component (ASC)
	if (UOnMetalTBProjectilePayload* Payload = Cast<UOnMetalTBProjectilePayload>(ImpactParams.Payload))
	{
		if (UAbilitySystemComponent* SourceASC = Payload->SourceASC.Get())
		{
			// // 2. Create TargetData from ImpactParams
			// FGameplayAbilityTargetDataHandle TargetData = Payload->CreateTargetData(ImpactParams);
			//
			//
			//
			// // 5. Broadcast Blueprint Event (Optional)
			// OnProjectileHit.Broadcast(ImpactParams);
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnBulletHit: Source Ability System Component is invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnBulletHit: Invalid payload type."));
	}
}

void UOnMetal_ProjectileWeaponAbility::FireTBProjectile()
{
}

FTBLaunchParams UOnMetal_ProjectileWeaponAbility::CreateTBLaunchParams() const
{
	return FTBLaunchParams();
}

void UOnMetal_ProjectileWeaponAbility::BindTBDelegates()
{
}

// ... (Implement other delegate callbacks: OnBulletComplete, OnBulletExitHit, OnBulletInjure)
