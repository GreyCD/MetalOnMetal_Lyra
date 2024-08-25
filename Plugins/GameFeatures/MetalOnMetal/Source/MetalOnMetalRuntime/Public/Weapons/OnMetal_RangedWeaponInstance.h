// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "TerminalBallistics/Public/Core/TBBulletDataAsset.h"
#include "OnMetal_RangedWeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_RangedWeaponInstance : public ULyraRangedWeaponInstance
{
	GENERATED_BODY()

public:
	UOnMetal_RangedWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	TSoftObjectPtr<UBulletDataAsset> GetBulletDataAsset() const { return BulletDataAsset; }
	void SetBulletDataAsset(TSoftObjectPtr<UBulletDataAsset> NewBulletDataAsset) { BulletDataAsset = NewBulletDataAsset; }

	UBulletDataAsset* GetBulletDataAssetPtr() const { return BulletDataAsset.Get(); }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon Properties")
	bool bIsProjectileWeapon;

	virtual bool IsProjectileWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual ECollisionChannel GetWeaponTraceChannel() const
	{
		// Default implementation, can be overridden in blueprints
		return ECC_MAX; // ECC_MAX means "not set"
	}


protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Terminal Ballistics")
	TSoftObjectPtr<UBulletDataAsset> BulletDataAsset;
	
};
