// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/TBProjectileId.h"
#include "OnMetalProjectileTypes.h"
#include "OnMetal_ProjectileWeaponComp.generated.h"

class ULyraWeaponStateComponent;
struct FGameplayAbilityTargetDataHandle;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class METALONMETALRUNTIME_API UOnMetal_ProjectileWeaponComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOnMetal_ProjectileWeaponComp();

	void AddUnconfirmedProjectileFiring(const FGameplayAbilityTargetDataHandle& TargetData, const FTBProjectileId& ProjectileId);
	void ConfirmProjectileHit(const FTBProjectileId& ProjectileId, const FHitResult& HitResult);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FTBProjectileId, FGameplayAbilityTargetDataHandle> UnconfirmedProjectileFirings;

	UPROPERTY()
	ULyraWeaponStateComponent* LyraWeaponStateComponent;

	void ClearOldUnconfirmedFirings();

	FTimerHandle ClearUnconfirmedFiringsHandle;

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Projectile")
	float UnconfirmedFiringTimeout = 5.0f;
};
