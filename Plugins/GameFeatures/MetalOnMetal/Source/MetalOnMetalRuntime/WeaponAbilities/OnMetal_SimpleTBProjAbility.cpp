// Fill out your copyright notice in the Description page of Project Settings.


#include "OnMetal_SimpleTBProjAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "LyraLogChannels.h"
#include "OnMetal_SimpleProjectile.h"
#include "Core/TBStatics.h"
#include "Physics/LyraCollisionChannels.h"
#include "NativeGameplayTags.h"
#include "Player/LyraPlayerController.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBProjectileId.h"
#include "Types/TBProjectileInjury.h"
#include "Weapons/OnMetal_RangedWeaponInstance.h"
#include "Weapons/OnMetal_WeaponStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_SimpleTBProjAbility)


// Weapon fire will be blocked/canceled if the player has this tag
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_OnMetal_WeaponFireBlocked, "Ability.Weapon.NoFiring");


FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
		// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}

UOnMetal_SimpleTBProjAbility::UOnMetal_SimpleTBProjAbility(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	
}

UOnMetal_RangedWeaponInstance* UOnMetal_SimpleTBProjAbility::GetWeaponInstance() const
{
	return Cast<UOnMetal_RangedWeaponInstance>(GetAssociatedEquipment());
}

bool UOnMetal_SimpleTBProjAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
				*UOnMetal_RangedWeaponInstance::StaticClass()->GetName());
			bResult = false;
		}
	}

	return bResult;
}

void UOnMetal_SimpleTBProjAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{

	check(CurrentActorInfo);
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon ability %s failed to commit"), *GetPathName());
		K2_EndAbility();
		return;
	}

	// Update the last firing time
	UOnMetal_RangedWeaponInstance* WeaponData = GetWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UOnMetal_SimpleTBProjAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);
		
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


ECollisionChannel UOnMetal_SimpleTBProjAbility::DetermineTraceChannel(FCollisionQueryParams& TraceParams,
	bool bIsSimulated) const
{
	return Lyra_TraceChannel_Weapon;
}

void UOnMetal_SimpleTBProjAbility::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

FHitResult UOnMetal_SimpleTBProjAbility::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
                                                     float SweepRadius, bool bIsSimulated, TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;

	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated);

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// Filter the output list to prevent multiple hits on the same actor;
		// this is to prevent a single bullet dealing damage multiple times to
		// a single actor if using an overlap trace
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
	
}

FVector UOnMetal_SimpleTBProjAbility::GetWeaponTargetingSourceLocation() const
{
	// Use Pawn's location as a base
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());
	check(AvatarPawn);

	const FVector SourceLoc = AvatarPawn->GetActorLocation();
	const FQuat SourceRot = AvatarPawn->GetActorQuat();

	FVector TargetingSourceLocation = SourceLoc;

	//@TODO: Add an offset from the weapon instance and adjust based on pawn crouch/aiming/etc...

	return TargetingSourceLocation;
}


FTransform UOnMetal_SimpleTBProjAbility::GetTargetingTransform(APawn* SourcePawn,
	EMetalOnMetal_AbilityTargetingSource Source) const
{
		check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController();
	UOnMetal_WeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr) ? SourcePawnController->FindComponentByClass<UOnMetal_WeaponStateComponent>() : nullptr;

	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EMetalOnMetal_AbilityTargetingSource::Custom);

	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;


	if ((Controller != nullptr) && ((Source == EMetalOnMetal_AbilityTargetingSource::CameraTowardsFocus) || (Source == EMetalOnMetal_AbilityTargetingSource::PawnTowardsFocus) || (Source == EMetalOnMetal_AbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Get camera position for later
		bFoundFocus = true;

		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC != nullptr)
		{
			PC->GetPlayerViewPoint(/*out*/ CamLoc, /*out*/ CamRot);
		}
		else
		{
			SourceLoc = GetWeaponTargetingSourceLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		// Determine initial focal point to 
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);

		// Move the start and focal point up in front of pawn
		if (PC)
		{
			const FVector WeaponLoc = GetWeaponTargetingSourceLocation();
			CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
			FocalLoc = CamLoc + (AimDir * FocalDistance);
		}
		//Move the start to be the HeadPosition of the AI
		else if (AAIController* AIController = Cast<AAIController>(Controller))
		{
			CamLoc = SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
		}

		if (Source == EMetalOnMetal_AbilityTargetingSource::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EMetalOnMetal_AbilityTargetingSource::WeaponForward) || (Source == EMetalOnMetal_AbilityTargetingSource::WeaponTowardsFocus))
	{
		SourceLoc = GetWeaponTargetingSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EMetalOnMetal_AbilityTargetingSource::PawnTowardsFocus) || (Source == EMetalOnMetal_AbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}


void UOnMetal_SimpleTBProjAbility::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	UOnMetal_RangedWeaponInstance* WeaponData = GetWeaponInstance();
	if (AvatarPawn && WeaponData)
	{
		FOnMetal_RangedWeaponFiringInput InputData;
		InputData.WeaponData = WeaponData;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		//@TODO: Should do more complicated logic here when the player is close to a wall, etc...
		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, EMetalOnMetal_AbilityTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();



		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

FHitResult UOnMetal_SimpleTBProjAbility::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace,
	float SweepRadius, bool bIsSimulated, TArray<FHitResult>& OutHits) const
{


	FHitResult Impact;
	Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			
				if (bool bUseSweepHits = true)
				{
					OutHits = SweepHits;
				}
			}
	
	return Impact;
}

void UOnMetal_SimpleTBProjAbility::TraceBulletsInCartridge(const FOnMetal_RangedWeaponFiringInput& InputData,
                                                           TArray<FHitResult>& OutHits)
{
	ULyraRangedWeaponInstance*  WeaponData = InputData.WeaponData;
	check(WeaponData);

	const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();

	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), /*bIsSimulated=*/ false, /*out*/ AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
			
			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}

			HitLocation = Impact.ImpactPoint;
		}

		// Make sure there's always an entry in OutHits so the direction can be used for tracers, etc...
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				// Locate the fake 'impact' at the end of the trace
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
	
	// We fired the weapon, add spread
	WeaponData->AddSpread();
	
}

TArray<FHitResult> UOnMetal_SimpleTBProjAbility::PerformWeaponTrace()
{
	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);
	return FoundHits;
}

void UOnMetal_SimpleTBProjAbility::K2_FireProjectile(FVector FireLocation, FRotator FireRotation, float ProjectileSpeed,
                                                     float EffectiveRange, TArray<AActor*> ActorsToIgnore, FTBProjectileId& OutProjectileId,
                                                     UOnMetal_TBPayload*& OutPayload, const FOnMetal_ProjectileCompleteDelegate& OnComplete,
                                                     const FOnMetal_ProjectileHitDelegate& OnHit, const FOnMetal_ProjectileExitHitDelegate& OnExitHit,
                                                     const FOnMetal_ProjectileInjureDelegate& OnInjure)
{


	// Create the payload
	UOnMetal_TBPayload* TBPayload = NewObject<UOnMetal_TBPayload>();

	UBulletDataAsset* LoadedBulletDataAsset = ProjectileDataAsset.LoadSynchronous();
	if (!LoadedBulletDataAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ProjectileDataAsset"));
		return; // or handle this error appropriately
	}
	
	// Store the Blueprint-bound delegates
	BlueprintOnComplete = OnComplete;
	BlueprintOnHit = OnHit;
	BlueprintOnExitHit = OnExitHit;
	BlueprintOnInjure = OnInjure;

	FBPOnProjectileComplete WrappedOnComplete;
	WrappedOnComplete.BindUFunction(this, FName("HandleProjectileComplete"));

	FBPOnBulletHit WrappedOnHit;
	WrappedOnHit.BindUFunction(this, FName("HandleProjectileHit"));

	FBPOnBulletExitHit WrappedOnExitHit;
	WrappedOnExitHit.BindUFunction(this, FName("HandleProjectileExitHit"));

	FBPOnBulletInjure WrappedOnInjure;
	WrappedOnInjure.BindUFunction(this, FName("HandleProjectileInjure"));

	// Create launch params
	FTBLaunchParams LaunchParams = UTerminalBallisticsStatics::MakeLaunchParams(
		ProjectileSpeed,
		EffectiveRange,
		1.0f, // Timescale
		FireLocation,
		FireRotation,
		ActorsToIgnore,
		UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		Lyra_TraceChannel_Weapon,
		true, // bIgnoreOwner
		true, // bAddToOwnerVelocity
		false, // bForceNoTracer
		GetOwningActorFromActorInfo(),
		GetControllerFromActorInfo(),
		1.0f, // GravityMultiplier
		10.0f, // OwnerIgnoreDistance
		25.0f, // TracerActivationDistance
		TBPayload
	);

	UTerminalBallisticsStatics::AddAndFireBulletWithCallbacks(
LoadedBulletDataAsset,
LaunchParams,
WrappedOnComplete,
WrappedOnHit,
WrappedOnExitHit,
WrappedOnInjure,
OutProjectileId,
2 // DebugType
);
	

	// Set the out payload
	OutPayload = TBPayload;
	
}

void UOnMetal_SimpleTBProjAbility::SpawnProjectile(const FTransform& SpawnTransform)
{
	if (!ProjectileClass) return;

	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	for (int32 i = 0; i < NumProjectiles; i++)
	{
		const FTransform SpawnedTransform = SpawnTransform;
		AOnMetal_SimpleProjectile* SpawnedProjectile = GetWorld()->SpawnActorDeferred<AOnMetal_SimpleProjectile>(ProjectileClass,
			SpawnedTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, SourceASC->MakeEffectContext());
		SpawnedProjectile->DamageEffectSpecHandle = SpecHandle;
		

		SpawnedProjectile->FinishSpawning(SpawnedTransform);
	}
}

void UOnMetal_SimpleTBProjAbility::HandleProjectileHit(const FTBImpactParams& ImpactParams)
{
	BlueprintOnHit.ExecuteIfBound(ImpactParams);
	
}



void UOnMetal_SimpleTBProjAbility::HandleProjectileComplete(const FTBProjectileId& CompletedProjectileId,
                                                            const TArray<FPredictProjectilePathPointData>& PathData)
{
	BlueprintOnComplete.ExecuteIfBound(CompletedProjectileId, PathData);
	
}

void UOnMetal_SimpleTBProjAbility::HandleProjectileExitHit(const FTBImpactParams& ImpactParams)
{
	BlueprintOnExitHit.ExecuteIfBound(ImpactParams);
}

void UOnMetal_SimpleTBProjAbility::HandleProjectileInjure(const FTBImpactParams& ImpactParams,
	const FTBProjectileInjuryParams& InjuryParams)
{
	BlueprintOnInjure.ExecuteIfBound(ImpactParams, InjuryParams);
}


