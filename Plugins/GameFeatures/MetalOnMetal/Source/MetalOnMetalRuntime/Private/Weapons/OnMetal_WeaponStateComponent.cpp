// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OnMetal_WeaponStateComponent.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffectTypes.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "Physics/PhysicalMaterialWithTags.h"
#include "Teams/LyraTeamSubsystem.h"
//#include "Weapons/LyraRangedWeaponInstance.h"
#include "Weapons/OnMetal_RangedWeaponInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_WeaponStateComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Gameplay_HitZone, "Gameplay.Zone");


UOnMetal_WeaponStateComponent::UOnMetal_WeaponStateComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UOnMetal_WeaponStateComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (ULyraEquipmentManagerComponent* EquipmentManager = Pawn->FindComponentByClass<
			ULyraEquipmentManagerComponent>())
		{
			if (UOnMetal_RangedWeaponInstance* CurrentWeapon = Cast<UOnMetal_RangedWeaponInstance>(
				EquipmentManager->GetFirstInstanceOfType(UOnMetal_RangedWeaponInstance::StaticClass())))
			{
				CurrentWeapon->Tick(DeltaTime);
			}
		}
	}
}

void UOnMetal_WeaponStateComponent::AddUnconfirmedServerSideHitMarkers(
	const FGameplayAbilityTargetDataHandle& InTargetData,
	const TArray<FHitResult>& FoundHits,
	bool bIsProjectileWeapon)
{
	FOnMetal_ServerSideHitMarkerBatch& NewUnconfirmedHitMarker = UnconfirmedServerSideHitMarkers.Emplace_GetRef(
		InTargetData.UniqueId);

	
	//NewUnconfirmedHitMarker.bIsProjectile = bIsProjectileWeapon;
	//NewUnconfirmedHitMarker.TargetData = InTargetData;

	if (APlayerController* OwnerPC = GetController<APlayerController>())
	{
		for (const FHitResult& Hit : FoundHits)
		{
			FVector2D HitScreenLocation;
			if(UGameplayStatics::ProjectWorldToScreen(OwnerPC, Hit.Location, /*out*/ HitScreenLocation,
													   /*bPlayerViewportRelative=*/ false))
			{
				FOnMetal_ScreenSpaceHitLocation& Entry = NewUnconfirmedHitMarker.Markers.AddDefaulted_GetRef();
				Entry.Location = HitScreenLocation;
				Entry.bShowAsSuccess = ShouldShowHitAsSuccess(Hit);

				// Determine the hit zone
				FGameplayTag HitZone;
				if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(
					Hit.PhysMaterial.Get()))
				{
					for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
					{
						if (MaterialTag.MatchesTag(TAG_Gameplay_HitZone))
						{
							Entry.HitZone = MaterialTag;
							break;
						}
					}
				}
			}
		}
	}
}

void UOnMetal_WeaponStateComponent::UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext)
{
	if (ShouldUpdateDamageInstigatedTime(EffectContext))
	{
		ActuallyUpdateDamageInstigatedTime();
	}
}

double UOnMetal_WeaponStateComponent::GetTimeSinceLastHitNotification() const
{
	UWorld* World = GetWorld();
	return World->TimeSince(LastWeaponDamageInstigatedTime);
}

void UOnMetal_WeaponStateComponent::ClientConfirmTargetData_Implementation(
	uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces)
{
	// UE_LOG(LogTemp, Warning,
	//        TEXT("ClientConfirmTargetData_Implementation called. IsServer: %s, UniqueId: %d, bSuccess: %s"),
	//        GetOwner()->HasAuthority() ? TEXT("True") : TEXT("False"), UniqueId,
	//        bSuccess ? TEXT("True") : TEXT("False"));
	
	for (int i = 0; i < UnconfirmedServerSideHitMarkers.Num(); i++)
	{
		FOnMetal_ServerSideHitMarkerBatch& Batch = UnconfirmedServerSideHitMarkers[i];
		if (Batch.UniqueId == UniqueId)
		{
			if (bSuccess && (HitReplaces.Num() != Batch.Markers.Num()))
			{
				UWorld* World = GetWorld();
				bool bFoundShowAsSuccessHit = false;

				int32 HitLocationIndex = 0;
				// For projectiles, we confirm all hits as successful if bSuccess is true
				for (const FOnMetal_ScreenSpaceHitLocation& Entry : Batch.Markers)
				{
					if (!HitReplaces.Contains(HitLocationIndex) && Entry.bShowAsSuccess)
					{
						// Only need to do this once
						if (!bFoundShowAsSuccessHit)
						{
							ActuallyUpdateDamageInstigatedTime();
						}

						bFoundShowAsSuccessHit = true;

						LastWeaponDamageScreenLocations.Add(Entry);
					}
					++HitLocationIndex;
				}
			}
			
			UnconfirmedServerSideHitMarkers.RemoveAt(i);
			break;
		}
	}
}


bool UOnMetal_WeaponStateComponent::ShouldShowHitAsSuccess(const FHitResult& Hit) const
{
	AActor* HitActor = Hit.GetActor();

	//@TODO: Don't treat a hit that dealt no damage (due to invulnerability or similar) as a success
	UWorld* World = GetWorld();
	if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
	{
		return TeamSubsystem->CanCauseDamage(GetController<APlayerController>(), Hit.GetActor());
	}

	return false;
}

bool UOnMetal_WeaponStateComponent::ShouldUpdateDamageInstigatedTime(
	const FGameplayEffectContextHandle& EffectContext) const
{
	//@TODO: Implement me, for the purposes of this component we really only care about damage caused by a weapon
	// or projectile fired from a weapon, and should filter to that
	// (or perhaps see if the causer is also the source of our active reticle config)
	return EffectContext.GetEffectCauser() != nullptr;
}

void UOnMetal_WeaponStateComponent::ActuallyUpdateDamageInstigatedTime()
{
	// If our LastWeaponDamageInstigatedTime was not very recent, clear our LastWeaponDamageScreenLocations array
	UWorld* World = GetWorld();
	if (World->GetTimeSeconds() - LastWeaponDamageInstigatedTime > 0.1)
	{
		LastWeaponDamageScreenLocations.Reset();
	}
	LastWeaponDamageInstigatedTime = World->GetTimeSeconds();
}

void UOnMetal_WeaponStateComponent::ProcessConfirmedHits(const FGameplayAbilityTargetDataHandle& TargetData)
{
	// Process the confirmed hits using the TargetData
	// This is where you would implement game-specific logic for confirmed hits

	// Example implementation:
	for (int32 i = 0; i < TargetData.Num(); ++i)
	{
		if (const FGameplayAbilityTargetData* Data = TargetData.Get(i))
		{
			// You might want to cast to your specific target data type here
			// const FLyraGameplayAbilityTargetData_SingleTargetHit* SingleTargetData = 
			//     static_cast<const FLyraGameplayAbilityTargetData_SingleTargetHit*>(Data);

			// Process each confirmed hit
			// This could involve applying damage, spawning effects, updating game state, etc.
			// Example:
			// if (SingleTargetData)
			// {
			//     FHitResult HitResult = SingleTargetData->HitResult;
			//     // Apply damage, spawn effects, etc. using HitResult
			// }
		}
	}
}
