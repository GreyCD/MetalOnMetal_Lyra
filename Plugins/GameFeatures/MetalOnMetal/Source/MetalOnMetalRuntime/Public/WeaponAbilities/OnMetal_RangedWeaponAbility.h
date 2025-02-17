// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBSimData.h"
#include "Weapons/LyraWeaponStateComponent.h"
#include "Weapons/OnMetal_RangedWeaponInstance.h"
#include "Projectile/OnMetal_TBPayload.h"
#include "OnMetal_RangedWeaponAbility.generated.h"

enum ECollisionChannel : int;

class APawn;
class ULyraRangedWeaponInstance;
class UObject;
struct FCollisionQueryParams;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;
struct FGameplayTag;
struct FGameplayTagContainer;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMetalProjectileCompleteDelegate, const FTBProjectileId&, CompletedProjectileId, const TArray<FPredictProjectilePathPointData>&, PathData);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMetalProjectileHitDelegate, const FTBImpactParams&, ImpactParams, const FGameplayAbilityTargetDataHandle&, TargetData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnMetalProjectileExitHitDelegate, const FTBImpactParams&, ImpactParams);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMetalProjectileInjureDelegate, const FTBImpactParams&, ImpactParams, const FTBProjectileInjuryParams&, InjuryParams);

USTRUCT(BlueprintType)
struct FOnMetal_ProjectileLaunchParams : public FTBLaunchParams
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY()
	FGameplayEffectContextHandle EffectContext;

	FOnMetal_ProjectileLaunchParams() : Super() {}

	FOnMetal_ProjectileLaunchParams(const FTBLaunchParams& BaseLaunchParams)
		: Super(BaseLaunchParams)
	{}
};


/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EOnMetal_AbilityTargetingSource : uint8
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
class METALONMETALRUNTIME_API UOnMetal_RangedWeaponAbility : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	UOnMetal_RangedWeaponAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	
	UFUNCTION(BlueprintCallable, Category="Lyra|Ability")
	UOnMetal_RangedWeaponInstance* GetOnMetalWeaponInstance() const;

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


	UFUNCTION(BlueprintCallable, Category = "OnMetal|Weapon", meta = (DisplayName = "Fire Projectile"))
	void K2_FireProjectile(
		FVector FireLocation, 
		FRotator FireRotation, 
		float ProjectileSpeed, 
		float EffectiveRange,
		TArray<AActor*> ActorsToIgnore,
		FGameplayTag ApplicationTag,
		FTBProjectileId& OutProjectileId,
		UOnMetal_TBPayload*& OutPayload,
		const FOnMetalProjectileCompleteDelegate& OnComplete,
		const FOnMetalProjectileHitDelegate& OnHit,
		const FOnMetalProjectileExitHitDelegate& OnExitHit,
		const FOnMetalProjectileInjureDelegate& OnInjure
	);
	
	virtual bool IsProjectileWeapon() const;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSoftObjectPtr<UBulletDataAsset> ProjectileDataAsset;

	UFUNCTION(BlueprintCallable, Category = "Gameplay Abilities|Effects", meta = (DisplayName = "Apply Gameplay Effect to Target", ExpandEnumAsExecs = "ApplicationTag"))
	void K2_ApplyGameplayEffectToTarget(
		TSubclassOf<UGameplayEffect> GameplayEffectClass,  // Input pin for the Gameplay Effect class
		AActor* TargetActor,                              // Input pin for the target actor
		float Level = 1.0f,                                // Optional input pin for effect level
		FGameplayTagContainer ApplicationTag = FGameplayTagContainer() // Optional input pin for application tags
	);

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<UGameplayEffect> OnMetal_DamageEffectClass;

protected:
	struct FOnMetalRangedWeaponFiringInput
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

		FOnMetalRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			  , EndAim(ForceInitToZero)
			  , AimDir(ForceInitToZero)
		{
		}
	};
	
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated,
	                       OUT TArray<FHitResult>& OutHitResults) const;

	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius,
	                               bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;

	// Traces all of the bullets in a single cartridge
	void TraceBulletsInCartridge(const FOnMetalRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits) const;

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;

	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits) const;
	FVector GetWeaponTargetingSourceLocation() const;
	FTransform GetTargetingTransform(APawn* SourcePawn, EOnMetal_AbilityTargetingSource Source) const;

	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);
	
	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	
	// Callback functions
	UFUNCTION()
	void HandleProjectileComplete(const FTBProjectileId& CompletedProjectileId, const TArray<FPredictProjectilePathPointData>& PathData, const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);
    
	UFUNCTION()
	void HandleProjectileHit(const FTBImpactParams& ImpactParams, FGameplayTag ApplicationTag);
    
	UFUNCTION()
	void HandleProjectileExitHit(const FTBImpactParams& ImpactParams, const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);
	
	UFUNCTION()
	void HandleProjectileInjure(const FTBImpactParams& ImpactParams, const FTBProjectileInjuryParams& InjuryParams, const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);

	//Wrapper functions for AbilityTargetDataSetDelegate

	void HandleProjectileCompleteWrapper(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);


	void HandleProjectileHitWrapper(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);


	void HandleProjectileExitHitWrapper(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);


	void HandleProjectileInjureWrapper(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);
	



private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
	TMap<FTBProjectileId, UOnMetal_TBPayload*> ProjectilePayloads;

	FOnMetalProjectileCompleteDelegate BlueprintOnComplete;
	FOnMetalProjectileHitDelegate BlueprintOnHit;
	FOnMetalProjectileExitHitDelegate BlueprintOnExitHit;
	FOnMetalProjectileInjureDelegate BlueprintOnInjure;

	FDelegateHandle OnProjectileCompleteDelegateHandle;
	FDelegateHandle OnProjectileHitDelegateHandle;
	FDelegateHandle OnProjectileExitHitDelegateHandle;
	FDelegateHandle OnProjectileInjureDelegateHandle;
	


};
