// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAbilities/OnMetal_ProjectileAbility.h"
#include "LyraGame/LyraLogChannels.h"
#include "Subsystem/OnMetal_ProjectileSubsystem.h"
#include "Weapons/OnMetal_RangedWeaponInstance.h"
#include "AIController.h"
#include "NativeGameplayTags.h"
#include "Weapons/OnMetal_WeaponStateComponent.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Physics/LyraCollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_ProjectileAbility)

// Weapon fire will be blocked/canceled if the player has this tag
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_OnMetalWeaponFireBlocked, "Ability.Weapon.NoFiring");

UOnMetal_ProjectileAbility::UOnMetal_ProjectileAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileSubsystem = CreateDefaultSubobject<UOnMetal_ProjectileSubsystem>(TEXT("ProjectileSubsystem"));
	// Create projectile data
	//FOnMetal_ProjectileData ProjectileData = FOnMetal_ProjectileData();
}

UOnMetal_RangedWeaponInstance* UOnMetal_ProjectileAbility::GetWeaponInstance() const
{
	return Cast<UOnMetal_RangedWeaponInstance>(GetAssociatedEquipment());
}


bool UOnMetal_ProjectileAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (bResult)
	{
		if (GetWeaponInstance() == nullptr)
		{
			UE_LOG(LogLyraAbilitySystem, Error, TEXT("Weapon ability %s cannot be activated because there is no associated ranged weapon (equipment instance=%s but needs to be derived from %s)"),
				*GetPathName(),
				*GetPathNameSafe(GetAssociatedEquipment()),
				*ULyraRangedWeaponInstance::StaticClass()->GetName());
			bResult = false;
		}
	}
	return bResult;
}

void UOnMetal_ProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	
	UAbilitySystemComponent* MyAbilitySystemComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilitySystemComponent);
	
	UOnMetal_RangedWeaponInstance* WeaponData = GetWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();
	
	Super::ActivateAbility(CurrentSpecHandle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UOnMetal_ProjectileAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if(ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);
		
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UOnMetal_ProjectileAbility::FireProjectileWeapon(const FTransform& SpawnTransform, UOnMetal_ProjectileDataAsset* DataAsset)
{
	check(CurrentActorInfo);

	ULyraRangedWeaponInstance* WeaponInstance = GetWeaponInstance();
	if (!WeaponInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("GetWeaponInstance returned null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// similarly for other potential null pointers
	if (!DataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("DataAsset is null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!ProjectileSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileSubsystem is null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// Create projectile data
	FOnMetal_ProjectileData* ProjectileData = new FOnMetal_ProjectileData();

	if(ProjectileData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ProjectileData is null"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	ProjectileData->ProjectileID = FMath::Rand(); // Use a better method for generating unique IDs in a real game
	ProjectileData->Velocity = DataAsset->Velocity; // Use this as initial velocity for now
	ProjectileData->DragCoefficient = DataAsset->DragCoefficient; // Set an appropriate value
	ProjectileData->Lifetime = DataAsset->Lifetime; // Set an appropriate value
	ProjectileData->CollisionChannel = Lyra_TraceChannel_Weapon; // Set your desired collision channel
	ProjectileData->ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ProjectileData->ParticleData = DataAsset->ParticleData;
	ProjectileData->DebugData = DataAsset->DebugData;
	ProjectileData->ProjectileOwner = GetAvatarActorFromActorInfo();
	
	// Fire the projectile
	ProjectileSubsystem->FireProjectile(ProjectileData->ProjectileID, DataAsset, ProjectileData->ActorsToIgnore, SpawnTransform, nullptr, ProjectileData->OnImpact, FOnMetal_OnProjectilePositionUpdate(), FGameplayEffectSpecHandle(), ProjectileData->ProjectileOwner);

	// Update the last firing time
	//WeaponInstance->UpdateFiringTime();

	// Add spread
	//WeaponInstance->AddSpread();
}

/*void UOnMetal_ProjectileAbility::OnProjectileImpact(const FHitResult& HitResult, const FVector& Direction,
	const int32 ProjectileID, FGameplayEffectSpecHandle& DamageEffectSpecHandle, AActor* ProjectileOwner)
{
	// Get the TargetDataHandle from the ProjectileSubsystem
	FGameplayAbilityTargetDataHandle TargetDataHandle = ProjectileSubsystem->GetProjectileTargetData(ProjectileID);
	UAbilitySystemComponent* SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ProjectileOwner);

	FScopedPredictionWindow ScopedPrediction(SourceASC, CurrentActivationInfo.GetActivationPredictionKey());

	//ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetDataHandle.GetHitResult(0).GetActor()->GetAbilitySystemComponent(), SourceASC);

	ServerProjectileHit(TargetDataHandle, HitResult, UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ProjectileOwner));
	
	// Trigger the Blueprint event
	OnProjectileTargetDataReady(TargetDataHandle, HitResult);
}*/

/*void UOnMetal_ProjectileAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
	FGameplayTag ApplicationTag)
{
	    UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
    check(MyAbilityComponent);

    if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
    {
        FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

        // Take ownership of the target data
        FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

        // Get the HitResult from the first target in the handle (assuming SingleTargetHit)
        FHitResult HitResult;
        if (LocalTargetDataHandle.Num() > 0)
        {
            if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(0)))
            {
                HitResult = SingleTargetHit->HitResult;
            }
        }

        const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
        if (bShouldNotifyServer)
        {
            MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
        }

        bool bIsTargetDataValid = true; // You might need additional validation logic here

#if WITH_SERVER_CODE
        // Server-side hit confirmation (if applicable)
        // ... (You can add logic from Lyra's code if needed, but likely not necessary for projectiles)
#endif //WITH_SERVER_CODE

        // See if we still have ammo or other conditions for committing the ability
        if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
        {
            // Trigger the Blueprint event
            OnProjectileTargetDataReady(LocalTargetDataHandle, HitResult);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Weapon ability %s failed to commit (bIsTargetDataValid=%d)"), *GetPathName(), bIsTargetDataValid ? 1 : 0);
            K2_EndAbility();
        }
    }

    // We've processed the data
    MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}*/

/*void UOnMetal_ProjectileAbility::ServerProjectileHit_Implementation(const FGameplayAbilityTargetDataHandle& TargetData,
	const FHitResult& HitResult, UAbilitySystemComponent* SourceASC)
{
	// 1. Basic Server-Side Validation
	if (HitResult.GetActor() && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor()))
	{
		// 2. Apply Gameplay Effect
		//ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), HitResult.GetActor()->GetAbilitySystemComponent(), SourceASC);

		// 3. Multicast Hit Confirmation
		//MulticastProjectileHitConfirmed(TargetData, HitResult);
	}
}*/

// bool UOnMetal_ProjectileAbility::ServerProjectileHit_Validate(const FGameplayAbilityTargetDataHandle& TargetData,
// 	const FHitResult& HitResult, UAbilitySystemComponent* SourceASC)
// {
// 	return true;
// }

// void UOnMetal_ProjectileAbility::OnProjectileHitTargetDelegate(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
// {
// 	// Call SetReplicatedTargetData to trigger the AbilityTargetDataSetDelegate
// 	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
// 	if (ensure(MyAbilityComponent))
// 	{
// 		MyAbilityComponent->ServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), TargetDataHandle, FGameplayTag(), MyAbilityComponent->ScopedPredictionKey);
// 	}
// }




