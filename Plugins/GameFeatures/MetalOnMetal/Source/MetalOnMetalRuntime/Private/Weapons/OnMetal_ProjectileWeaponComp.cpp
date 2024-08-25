// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OnMetal_ProjectileWeaponComp.h"

#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/LyraWeaponStateComponent.h"


UOnMetal_ProjectileWeaponComp::UOnMetal_ProjectileWeaponComp()
{
	PrimaryComponentTick.bCanEverTick = false;


}

FVector2D ToVector2D(const FVector_NetQuantize& VectorNetQuantize)
{
	return FVector2D(VectorNetQuantize.X, VectorNetQuantize.Y);
}



void UOnMetal_ProjectileWeaponComp::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	if (Owner)
	{
		LyraWeaponStateComponent = Owner->FindComponentByClass<ULyraWeaponStateComponent>();
	}
}



void UOnMetal_ProjectileWeaponComp::AddUnconfirmedProjectileFiring(const FGameplayAbilityTargetDataHandle& TargetData, const FTBProjectileId& ProjectileId)
{
	UnconfirmedProjectileFirings.Add(ProjectileId, TargetData);
	
	// Access LyraWeaponStateComponent to add hit markers
	if (LyraWeaponStateComponent)
	{
		LyraWeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, TArray<FHitResult>());  // Let Lyra handle batch creation

		// Get reference to the batch (it will be created if it didn't exist)
		FLyraServerSideHitMarkerBatch& Batch = LyraWeaponStateComponent->UnconfirmedServerSideHitMarkers.Last(); // Get the most recently added batch (which should be the one we just created)
        
		// Iterate through TargetData and create hit markers 
		for (int32 i = 0; i < TargetData.Num(); ++i)
		{
			if (const FLyraGameplayAbilityTargetData_SingleTargetHit* HitTargetData = static_cast<const FLyraGameplayAbilityTargetData_SingleTargetHit*>(TargetData.Get(i)))
			{
				FVector2D ScreenLocation;
				APlayerController* PC = Cast<APlayerController>(LyraWeaponStateComponent->GetOwner());
				if (PC && PC->ProjectWorldLocationToScreen(HitTargetData->HitResult.Location, ScreenLocation))
				{
					Batch.Markers.Add({ScreenLocation, FGameplayTag(), false}); // Initially unconfirmed
				}
			}
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(ClearUnconfirmedFiringsHandle, this, &UOnMetal_ProjectileWeaponComp::ClearOldUnconfirmedFirings, UnconfirmedFiringTimeout, false);
}

void UOnMetal_ProjectileWeaponComp::ConfirmProjectileHit(const FTBProjectileId& ProjectileId, const FHitResult& HitResult)
{
	
    if (FGameplayAbilityTargetDataHandle* TargetData = UnconfirmedProjectileFirings.Find(ProjectileId))
    {
        // Update TargetData with HitResult
        // ... (your existing code to update TargetData) ...

        if (LyraWeaponStateComponent)
        {
            // Lyra-specific hit confirmation
            for (FLyraServerSideHitMarkerBatch& Batch : LyraWeaponStateComponent->UnconfirmedServerSideHitMarkers)
            {
                if (Batch.UniqueId == TargetData->UniqueId)
                {
                    // Project hit location to screen space (Lyra's approach)
                    if (APlayerController* OwnerPC = LyraWeaponStateComponent->GetController<APlayerController>())
                    {
                        FVector2D HitScreenLocation;
                        if (UGameplayStatics::ProjectWorldToScreen(OwnerPC, HitResult.Location, HitScreenLocation, false))
                        {
                            // Mark the hit marker as successful if screen locations match
                            for (FLyraScreenSpaceHitLocation& Marker : Batch.Markers)
                            {
                                if (Marker.Location.Equals(HitScreenLocation)) 
                                {
                                    Marker.bShowAsSuccess = true;
                                    break; // Assuming only one hit per projectile for now
                                }
                            }
                        }
                    }
                }
            }
            
            TArray<uint8> HitReplaces;  // You'll need to fill this based on Lyra's implementation (likely indices of replaced hits)
            LyraWeaponStateComponent->ClientConfirmTargetData(TargetData->UniqueId, true, HitReplaces);
        }

        UnconfirmedProjectileFirings.Remove(ProjectileId);

        // Stop the timer if there are no more unconfirmed projectiles
        if (UnconfirmedProjectileFirings.IsEmpty())
        {
            GetWorld()->GetTimerManager().ClearTimer(ClearUnconfirmedFiringsHandle);
        }
    }
}


void UOnMetal_ProjectileWeaponComp::ClearOldUnconfirmedFirings()
{
	UnconfirmedProjectileFirings.Empty();
}

