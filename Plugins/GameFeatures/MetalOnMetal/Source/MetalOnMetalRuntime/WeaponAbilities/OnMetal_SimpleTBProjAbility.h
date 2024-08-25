// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "Projectile/OnMetal_TBPayload.h"
#include "OnMetal_SimpleTBProjAbility.generated.h"

enum ECollisionChannel : int;

class APawn;
class UOnMetal_RangedWeaponInstance;
class UObject;
struct FCollisionQueryParams;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;
struct FGameplayTag;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMetal_ProjectileCompleteDelegate, const FTBProjectileId&, CompletedProjectileId, const TArray<FPredictProjectilePathPointData>&, PathData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMetal_ProjectileHitDelegate, const FTBImpactParams&, ImpactParams);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMetal_ProjectileExitHitDelegate, const FTBImpactParams&, ImpactParams);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMetal_ProjectileInjureDelegate, const FTBImpactParams&, ImpactParams, const FTBProjectileInjuryParams&, InjuryParams);

/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EMetalOnMetal_AbilityTargetingSource : uint8
{
	// From the player's camera towards camera focus
	CameraTowardsFocus,
	// From the pawn's center, in the pawn's orientation
	PawnForward,
	// From the pawn's center, oriented towards camera focus
	PawnTowardsFocus,
	// From the weapon's muzzle or location, in the pawn's orientation
	WeaponForward,
	// From the weapon's muzzle or location, towards camera focus
	WeaponTowardsFocus,
	// Custom blueprint-specified source location
	Custom
};
/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_SimpleTBProjAbility : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	UOnMetal_SimpleTBProjAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="OnMetal|Ability")
	UOnMetal_RangedWeaponInstance* GetWeaponInstance() const;

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
									const FGameplayTagContainer* SourceTags = nullptr,
									const FGameplayTagContainer* TargetTags = nullptr,
									OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
							const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
							bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSoftObjectPtr<UBulletDataAsset> ProjectileDataAsset;

	UFUNCTION(BlueprintCallable, Category = "OnMetal|Weapon", meta = (DisplayName = "Fire Projectile"))
	void K2_FireProjectile(
		FVector FireLocation, 
		FRotator FireRotation, 
		float ProjectileSpeed, 
		float EffectiveRange,
		TArray<AActor*> ActorsToIgnore,
		FTBProjectileId& OutProjectileId,
		UOnMetal_TBPayload*& OutPayload,
		const FOnMetal_ProjectileCompleteDelegate& OnComplete,
		const FOnMetal_ProjectileHitDelegate& OnHit,
		const FOnMetal_ProjectileExitHitDelegate& OnExitHit,
		const FOnMetal_ProjectileInjureDelegate& OnInjure
	);

protected:
	struct FOnMetal_RangedWeaponFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// The weapon instance / source of weapon data
		ULyraRangedWeaponInstance* WeaponData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FOnMetal_RangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;

	FVector GetWeaponTargetingSourceLocation() const;

	FTransform GetTargetingTransform(APawn* SourcePawn, EMetalOnMetal_AbilityTargetingSource Source) const;

	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);

	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;
	
	void TraceBulletsInCartridge(const FOnMetal_RangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	UFUNCTION(BlueprintCallable)
	TArray<FHitResult> PerformWeaponTrace();

	
	// Callback functions
	UFUNCTION()
	void HandleProjectileComplete(const FTBProjectileId& CompletedProjectileId, const TArray<FPredictProjectilePathPointData>& PathData);
    
	UFUNCTION()
	void HandleProjectileHit(const FTBImpactParams& ImpactParams);
    
	UFUNCTION()
	void HandleProjectileExitHit(const FTBImpactParams& ImpactParams);
	
	UFUNCTION()
	void HandleProjectileInjure(const FTBImpactParams& ImpactParams, const FTBProjectileInjuryParams& InjuryParams);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FTransform& SpawnTransform);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<AOnMetal_SimpleProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	int32 NumProjectiles = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

private:
	FOnMetal_ProjectileCompleteDelegate BlueprintOnComplete;
	FOnMetal_ProjectileHitDelegate BlueprintOnHit;
	FOnMetal_ProjectileExitHitDelegate BlueprintOnExitHit;
	FOnMetal_ProjectileInjureDelegate BlueprintOnInjure;

	// FDelegateHandle OnProjectileCompleteDelegateHandle;
	// FDelegateHandle OnProjectileHitDelegateHandle;
	// FDelegateHandle OnProjectileExitHitDelegateHandle;
	// FDelegateHandle OnProjectileInjureDelegateHandle;
};
