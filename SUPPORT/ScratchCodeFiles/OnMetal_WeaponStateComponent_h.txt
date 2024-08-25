// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ControllerComponent.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

#include "OnMetal_WeaponStateComponent.generated.h"

class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;
struct FGameplayEffectContextHandle;
struct FHitResult;

// Hit markers are shown for ranged weapon impacts in the reticle
// A 'successful' hit marker is shown for impacts that damaged an enemy
struct FOnMetal_ScreenSpaceHitLocation
{
	/** Hit location in viewport screenspace */
	FVector2D Location;	
	FGameplayTag HitZone;
	bool bShowAsSuccess = false;
};

struct FOnMetal_ServerSideHitMarkerBatch
{
	FOnMetal_ServerSideHitMarkerBatch() { }

	FOnMetal_ServerSideHitMarkerBatch(uint8 InUniqueId) :
		UniqueId(InUniqueId)
	{ }

	TArray<FOnMetal_ScreenSpaceHitLocation> Markers;

	uint8 UniqueId = 0;

	FGameplayAbilityTargetDataHandle TargetData;

	bool bIsProjectile = false;
};

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_WeaponStateComponent : public UControllerComponent
{
	GENERATED_BODY()

public:
	UOnMetal_WeaponStateComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Client, Reliable)
	void ClientConfirmTargetData(uint16 UniqueId, bool bSuccess, const TArray<uint8>& HitReplaces);

	void AddUnconfirmedServerSideHitMarkers(const FGameplayAbilityTargetDataHandle& InTargetData, const TArray<FHitResult>& FoundHits, bool bIsProjectileWeapon);

	/** Updates this player's last damage instigated time */
	void UpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext);

	/** Gets the array of most recent locations this player instigated damage, in screen-space */
	void GetLastWeaponDamageScreenLocations(TArray<FOnMetal_ScreenSpaceHitLocation>& WeaponDamageScreenLocations)
	{
		WeaponDamageScreenLocations = LastWeaponDamageScreenLocations;
	}

	/** Returns the elapsed time since the last (outgoing) damage hit notification occurred */
	double GetTimeSinceLastHitNotification() const;

	int32 GetUnconfirmedServerSideHitMarkerCount() const
	{
		return UnconfirmedServerSideHitMarkers.Num();
	}

	/** The unconfirmed hits */
	TArray<FOnMetal_ServerSideHitMarkerBatch> UnconfirmedServerSideHitMarkers;

protected:
	// This is called to filter hit results to determine whether they should be considered as a successful hit or not
	// The default behavior is to treat it as a success if being done to a team actor that belongs to a different team
	// to the owning controller's pawn
	virtual bool ShouldShowHitAsSuccess(const FHitResult& Hit) const;

	virtual bool ShouldUpdateDamageInstigatedTime(const FGameplayEffectContextHandle& EffectContext) const;

	void ActuallyUpdateDamageInstigatedTime();

	void ProcessConfirmedHits(const FGameplayAbilityTargetDataHandle& TargetData);


private:
	/** Last time this controller instigated weapon damage */
	double LastWeaponDamageInstigatedTime = 0.0;

	/** Screen-space locations of our most recently instigated weapon damage (the confirmed hits) */
	TArray<FOnMetal_ScreenSpaceHitLocation> LastWeaponDamageScreenLocations;
	
};
