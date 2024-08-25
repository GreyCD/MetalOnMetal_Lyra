// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/LyraGameplayAbility_FromEquipment.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "OnMetal_RangedProjectileAbility.generated.h"

class UOnMetal_RangedWeaponInstance;
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

USTRUCT(BlueprintType)
struct FOnMetalProjectileLaunchParams : public FTBLaunchParams
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY()
	FGameplayEffectContextHandle EffectContext;

	FOnMetalProjectileLaunchParams() : Super() {}

	FOnMetalProjectileLaunchParams(const FTBLaunchParams& BaseLaunchParams)
		: Super(BaseLaunchParams)
	{}
};

UCLASS()
class UOnMetalProjectilePayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY()
	FGameplayAbilitySpecHandle AbilityHandle;

	UPROPERTY()
	FGameplayEffectContextHandle EffectContext;

	// virtual FGameplayAbilityTargetDataHandle CreateTargetData(const FTBImpactParams& ImpactParams) const;
};

/** Defines where an ability starts its trace from and where it should face */
UENUM(BlueprintType)
enum class EOnMetalAbilityTargetingSource : uint8
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
class METALONMETALRUNTIME_API UOnMetal_RangedProjectileAbility : public ULyraGameplayAbility_FromEquipment
{
	GENERATED_BODY()

public:
	UOnMetal_RangedProjectileAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Lyra|Ability")
	UOnMetal_RangedWeaponInstance* GetOnMetalWeaponInstance() const;

	UPROPERTY()
	FBPOnProjectileComplete OnBulletCompleteDelegate;

	UPROPERTY()
	FBPOnBulletHit OnBulletHitDelegate;

	UPROPERTY()
	FBPOnBulletExitHit OnBulletExitHitDelegate;

	UPROPERTY()
	FBPOnBulletInjure OnBulletInjureDelegate;

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	UFUNCTION()
	void OnBulletComplete(const FTBProjectileId Id, const TArray<FPredictProjectilePathPointData>& PathResults);

	UFUNCTION()
	void OnBulletHit(const FTBImpactParams& ImpactParams);

	UFUNCTION()
	void OnBulletExitHit(const FTBImpactParams& ImpactParams);

	UFUNCTION()
	void OnBulletInjure(const FTBImpactParams& ImpactParams, const FTBProjectileInjuryParams& InjuryParams);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "OnMetal|Ability")
	void OnBulletHitBP(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable, Category = "OnMetal|Ability")
	void FireTBProjectile();

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
		UOnMetal_RangedWeaponInstance* WeaponData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FOnMetalRangedWeaponFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};

protected:
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);

	// Does a single weapon trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;

	// Wrapper around WeaponTrace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;

	// Traces all of the bullets in a single cartridge
	void TraceBulletsInCartridge(const FOnMetalRangedWeaponFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;

	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);
	
	UFUNCTION(BlueprintCallable)
	FVector GetWeaponTargetingSourceLocation() const;
	
	UFUNCTION(BlueprintCallable)
	FTransform GetTargetingTransform(APawn* SourcePawn, EOnMetalAbilityTargetingSource Source) const;

	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);
	
	UFUNCTION(BlueprintCallable)
	FOnMetalProjectileLaunchParams CreateLaunchParams();

	UFUNCTION(BlueprintCallable)
	void StartRangedWeaponTargeting();

	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedWeaponTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

	UFUNCTION(BlueprintCallable, Category = "OnMetal|Ability")
	FTBLaunchParams CreateTBLaunchParams() const;

	void BindTBDelegates();




private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
	TMap<FTBProjectileId, FOnMetalLaunchParams> ActiveProjectiles;
	
};
