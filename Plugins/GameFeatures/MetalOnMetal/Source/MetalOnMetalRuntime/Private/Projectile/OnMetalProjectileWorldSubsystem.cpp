// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/OnMetalProjectileWorldSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemBlueprintLibrary.h"

void UOnMetalProjectileWorldSubsystem::FireProjectile(const int32 ProjectileID, USKGPDAProjectile* DataAsset,
                                                      const TArray<AActor*>& ActorsToIgnore, const FTransform& LaunchTransform,
                                                      UPrimitiveComponent* VisualComponentOverride, FSKGOnProjectileImpact OnImpact,
                                                      FSKGOnProjetilePositionUpdate OnPositionUpdate, FGameplayEffectSpecHandle DamageEffectSpecHandle, AActor* ProjectileOwner)
{
	FSKGProjectileData Projectile;
	Projectile.DamageEffectSpecHandle = DamageEffectSpecHandle;
	
	if (!DamageEffectSpecHandle.IsValid())
	{
		if(const UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ProjectileOwner))
		{
			FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
			EffectContext.AddSourceObject(ProjectileOwner);
			DamageEffectSpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1, EffectContext);
		}
	}
	
	// Create a new delegate that will call both our custom handler and the original OnImpact
	FSKGOnProjectileImpact ModifiedOnImpact;
	ModifiedOnImpact.BindUFunction(this, FName("HandleProjectileImpact"));

	
	
	Super::FireProjectile(ProjectileID, DataAsset, ActorsToIgnore, LaunchTransform, VisualComponentOverride, ModifiedOnImpact, OnPositionUpdate, DamageEffectSpecHandle,ProjectileOwner);

	
}




void UOnMetalProjectileWorldSubsystem::HandleProjectileImpact(const FHitResult& HitResult, const FVector& Direction, const int32 ProjectileID, FGameplayEffectSpecHandle DamageEffectSpecHandle, AActor* ProjectileOwner, FGameplayEffectContextHandle& OutEffectContext, UAbilitySystemComponent*& OutTargetASC) const
{
	OutTargetASC = nullptr;
	if (AActor* HitActor = HitResult.GetActor())
	{
		if (HitActor != ProjectileOwner)
		{
			OutTargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);

			if (OutTargetASC && DamageEffectSpecHandle.IsValid())
			{
				OutEffectContext = DamageEffectSpecHandle.Data->GetContext();
				OutEffectContext.AddHitResult(HitResult);
			}
			else
			{
				if (!OutTargetASC)
				{
					UE_LOG(LogTemp, Warning, TEXT("No Ability System Component found on Hit Actor"));
				}
				if (!DamageEffectSpecHandle.IsValid())
				{
					UE_LOG(LogTemp, Warning, TEXT("Invalid DamageEffectSpecHandle"));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Hit Actor is the Projectile Owner"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Hit Actor found"));
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void UOnMetalProjectileWorldSubsystem::OnMetal_ApplyDamageEffect(UAbilitySystemComponent* TargetASC,  FGameplayEffectContextHandle& EffectContext, TSubclassOf<UGameplayEffect> DamageEffect, FActiveGameplayEffectHandle& ActiveEffectHandle)
{
	ActiveEffectHandle = FActiveGameplayEffectHandle();

	if (TargetASC && DamageEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(DamageEffectClass, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			if (ActiveEffectHandle.WasSuccessfullyApplied())
			{
				UE_LOG(LogTemp, Warning, TEXT("Damage Gameplay Effect successfully applied to target from source"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to apply Damage Gameplay Effect to target from source"));
			}
		}
	}
}

//////////////////////////////////////////////////////////////









