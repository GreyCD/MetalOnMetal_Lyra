// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAbilities/OnMetal_RangedWeaponAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "LyraLogChannels.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "Character/OnMetalCharacter.h"
#include "Core/TBStatics.h"
#include "Physics/LyraCollisionChannels.h"
#include "TargetDataTypes/OnMetal_GameplayAbilityTargetData_SingleHitTarget.h"
#include "Types/TBImpactParams.h"
#include "Types/TBProjectileId.h"
#include "Types/TBProjectileInjury.h"
#include "WeaponAbilities/OnMetal_ProjectileImpactAbility.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "Weapons/LyraWeaponStateComponent.h"
#include "Weapons/OnMetal_WeaponStateComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_RangedWeaponAbility)

namespace LyraConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuraton(
		TEXT("lyra.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHits(
		TEXT("lyra.Weapon.DrawBulletHitDuration"),
		DrawBulletHitDuration,
		TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitRadius = 3.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
		TEXT("lyra.Weapon.DrawBulletHitRadius"),
		DrawBulletHitRadius,
		TEXT(
			"When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
		ECVF_Default);
}

// Weapon fire will be blocked/canceled if the player has this tag
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WeaponFireBlocked, "Ability.Weapon.NoFiring");

//////////////////////////////////////////////////////////////////////

/*FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
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
}*/



UOnMetal_RangedWeaponAbility::UOnMetal_RangedWeaponAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SourceBlockedTags.AddTag(TAG_WeaponFireBlocked);
}

UOnMetal_RangedWeaponInstance* UOnMetal_RangedWeaponAbility::GetOnMetalWeaponInstance() const
{
	return Cast<UOnMetal_RangedWeaponInstance>(GetAssociatedEquipment());
}

bool UOnMetal_RangedWeaponAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayTagContainer* SourceTags,
                                                      const FGameplayTagContainer* TargetTags,
                                                      FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (bResult)
	{
		if (GetOnMetalWeaponInstance() == nullptr)
		{
			UE_LOG(LogLyraAbilitySystem, Error,
			       TEXT(
				       "Weapon ability %s cannot be activated because there is no associated ranged weapon (equipment instance=%s but needs to be derived from %s)"
			       ),
			       *GetPathName(),
			       *GetPathNameSafe(GetAssociatedEquipment()),
			       *UOnMetal_RangedWeaponInstance::StaticClass()->GetName());
			bResult = false;
		}
	}

	return bResult;
}

void UOnMetal_RangedWeaponAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	// Bind target data callback
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	//OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	// Bind the OnMetal delegates
	//OnProjectileCompleteDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::HandleProjectileCompleteWrapper);
	//OnProjectileHitDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::HandleProjectileHitWrapper);
	//OnProjectileExitHitDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::HandleProjectileExitHitWrapper);
	//OnProjectileInjureDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::HandleProjectileInjureWrapper);
	
	
	// Update the last firing time
	UOnMetal_RangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UOnMetal_RangedWeaponAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo,
			                                                      ActivationInfo, bReplicateEndAbility, bWasCancelled));
			return;
		}

		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegates
		//MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle,CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		
		// Unbind the OnMetal delegates
		//MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnProjectileCompleteDelegateHandle);
		//MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnProjectileHitDelegateHandle);
		//MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnProjectileExitHitDelegateHandle);
		//MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnProjectileInjureDelegateHandle);

		//MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,CurrentActivationInfo.GetActivationPredictionKey());

		// Clear the ProjectilePayloads map
		ProjectilePayloads.Empty();
		

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

bool UOnMetal_RangedWeaponAbility::IsProjectileWeapon() const
{
	if (UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance())
	{
		return WeaponInstance->IsProjectileWeapon();
	}
	return false;
}

void UOnMetal_RangedWeaponAbility::K2_ApplyGameplayEffectToTarget(TSubclassOf<UGameplayEffect> GameplayEffectClass,
	AActor* TargetActor, float Level, FGameplayTagContainer ApplicationTag)
{
	if (!GameplayEffectClass || !TargetActor)
	{
		// Handle invalid input (e.g., log an error)
		return;
	}

	// Get the Ability System Component from the target actor
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!TargetASC)
	{
		// Handle case where the target doesn't have an ASC
		return;
	}

	// Create the Gameplay Effect Spec Handle
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(GameplayEffectClass, Level);

	// Apply Gameplay Effect to Target
	TargetASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void UOnMetal_RangedWeaponAbility::K2_FireProjectile(
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
	const FOnMetalProjectileInjureDelegate& OnInjure)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;
	
	check(CurrentActorInfo);
	if (!CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon ability %s failed to commit"), *GetPathName());
		K2_EndAbility();
		return;
	}

	UOnMetal_RangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
	check(WeaponData);
	//WeaponData->AddSpread();

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* SourceASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(SourceASC);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
	UOnMetal_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UOnMetal_WeaponStateComponent>();

	//FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	// Generate a unique projectile ID
	OutProjectileId = FTBProjectileId::CreateNew();
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(OnMetal_DamageEffectClass, 1.0f, SourceASC->MakeEffectContext());
	const int32 CartridgeID = FMath::Rand();

	// Create the payload
	UOnMetal_TBPayload* TBPayload = NewObject<UOnMetal_TBPayload>();

	TBPayload->OwningActor = GetOwningActorFromActorInfo();
	TBPayload->ApplicationTag = ApplicationTag;
	TBPayload->DamageEffectSpecHandle = SpecHandle;
	TBPayload->SourceASC = SourceASC;
	// Set up the custom target data
	TBPayload->CustomTargetData.CartridgeID = CartridgeID; // Implement this in your weapon instance
	
	//ProjectilePayloads.Add(OutProjectileId, TBPayload);
	FCollisionQueryParams TraceParams;
	ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, false);

	// Create launch params
	FTBLaunchParams LaunchParams = UTerminalBallisticsStatics::MakeLaunchParams(
		ProjectileSpeed,
		EffectiveRange,
		1.0f, // Timescale
		FireLocation,
		FireRotation,
		ActorsToIgnore,
		UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		TraceChannel,
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

	/*ProjectileDataAsset = WeaponData->GetBulletDataAssetPtr();
	check(ProjectileDataAsset)*/

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



int32 UOnMetal_RangedWeaponAbility::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// If we hit a pawn, we're good
			return Idx;
		}
		else
		{
			AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(
				HitActor->GetAttachParentActor()) != nullptr))
			{
				// If we hit something attached to a pawn, we're good
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

FHitResult UOnMetal_RangedWeaponAbility::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
                                                     float SweepRadius, bool bIsSimulated,
                                                     TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;

	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/
	                                  GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;

	const ECollisionChannel TraceChannel = DetermineTraceChannel(TraceParams, bIsSimulated);

	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel,
		                                FCollisionShape::MakeSphere(SweepRadius), TraceParams);
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

FHitResult UOnMetal_RangedWeaponAbility::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace,
                                                             float SweepRadius, bool bIsSimulated,
                                                             TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
	if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false,
		              LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
	}
#endif // ENABLE_DRAW_DEBUG

	FHitResult Impact;

	// Trace and process instant hit if something was hit
	// First trace without using sweep radius
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// If we had a blocking hit in our line trace that occurs in SweepHits before our
				// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

void UOnMetal_RangedWeaponAbility::TraceBulletsInCartridge(const FOnMetalRangedWeaponFiringInput& InputData,
                                                           TArray<FHitResult>& OutHits) const
{
	/*ULyraRangedWeaponInstance* WeaponData = InputData.WeaponData;
	check(WeaponData);

	const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();

	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians,
		                                                      WeaponData->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(),
		                                        /*bIsSimulated=#1# false, /*out#1# AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
#if ENABLE_DRAW_DEBUG
			if (LyraConsoleVariables::DrawBulletHitDuration > 0.0f)
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, LyraConsoleVariables::DrawBulletHitRadius, FColor::Red,
				               false, LyraConsoleVariables::DrawBulletHitRadius);
			}
#endif

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
	}*/
}

void UOnMetal_RangedWeaponAbility::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

ECollisionChannel UOnMetal_RangedWeaponAbility::DetermineTraceChannel(FCollisionQueryParams& TraceParams,
                                                                      bool bIsSimulated) const
{
	// First, try to get the trace channel from the weapon instance
	if (UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance())
	{
		ECollisionChannel WeaponTraceChannel = WeaponInstance->GetWeaponTraceChannel();
		if (WeaponTraceChannel != ECC_MAX)
		{
			return WeaponTraceChannel;
		}
	}

	// If no specific channel is set on the weapon, fall back to the default
	return Lyra_TraceChannel_Weapon;
}

void UOnMetal_RangedWeaponAbility::PerformLocalTargeting(TArray<FHitResult>& OutHits) const
{
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	ULyraRangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponData)
	{
		FOnMetalRangedWeaponFiringInput InputData;
		InputData.WeaponData = WeaponData;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		//@TODO: Should do more complicated logic here when the player is close to a wall, etc...
		const FTransform TargetTransform = GetTargetingTransform(
			AvatarPawn, EOnMetal_AbilityTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
		if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
		{
			static float DebugThickness = 2.0f;
			DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f),
			              FColor::Yellow, false, LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif

		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

FVector UOnMetal_RangedWeaponAbility::GetWeaponTargetingSourceLocation() const
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

FTransform UOnMetal_RangedWeaponAbility::GetTargetingTransform(APawn* SourcePawn,
                                                               EOnMetal_AbilityTargetingSource Source) const
{
	check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController();
	UOnMetal_WeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr)
		                                                      ? SourcePawnController->FindComponentByClass<
			                                                      UOnMetal_WeaponStateComponent>()
		                                                      : nullptr;

	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EOnMetal_AbilityTargetingSource::Custom);

	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;


	if ((Controller != nullptr) && ((Source == EOnMetal_AbilityTargetingSource::CameraTowardsFocus) || (Source ==
		EOnMetal_AbilityTargetingSource::PawnTowardsFocus) || (Source ==
		EOnMetal_AbilityTargetingSource::WeaponTowardsFocus)))
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

		if (Source == EOnMetal_AbilityTargetingSource::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EOnMetal_AbilityTargetingSource::WeaponForward) || (Source ==
		EOnMetal_AbilityTargetingSource::WeaponTowardsFocus))
	{
		SourceLoc = GetWeaponTargetingSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EOnMetal_AbilityTargetingSource::PawnTowardsFocus) || (Source ==
		EOnMetal_AbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

void UOnMetal_RangedWeaponAbility::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
	UOnMetal_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<
		UOnMetal_WeaponStateComponent>();

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out*/ FoundHits);

	// Fill out the target data from the hit results
	FGameplayAbilityTargetDataHandle TargetData;
	TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

	if (FoundHits.Num() > 0)
	{
		const int32 CartridgeID = FMath::Rand();

		for (const FHitResult& FoundHit : FoundHits)
		{
			FOnMetal_GameplayAbilityTargetData_SingleHitTarget* NewTargetData = new
				FOnMetal_GameplayAbilityTargetData_SingleHitTarget();
			NewTargetData->HitResult = FoundHit;
			NewTargetData->CartridgeID = CartridgeID;

			TargetData.Add(NewTargetData);
		}
	}

	// Send hit marker information
	const bool bProjectileWeapon = false;
	if (!bProjectileWeapon && (WeaponStateComponent != nullptr))
	{
		WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits, bProjectileWeapon);
	}

	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());
}


void UOnMetal_RangedWeaponAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
                                                             FGameplayTag ApplicationTag)
{
	UE_LOG(LogTemp, Warning, TEXT("OnTargetDataReadyCallback called. IsServer: %s"),
	       GetOwningActorFromActorInfo()->HasAuthority() ? TEXT("True") : TEXT("False"));

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(
			MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
			                                                      CurrentActivationInfo.GetActivationPredictionKey(),
			                                                      LocalTargetDataHandle, FGameplayTag(),
			                                                      MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;

		// Use the IsProjectileWeapon flag from the weapon instance
		bool bProjectileWeapon = false;
		if (UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance())
		{
			bProjectileWeapon = WeaponInstance->IsProjectileWeapon();
		}
#if WITH_SERVER_CODE
		// Handle hit confirmation for non-projectile weapons
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority)
				{
					// Server side hit confirmation
					if (UOnMetal_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<
						UOnMetal_WeaponStateComponent>())
					{
						TArray<uint8> HitReplaces;
						for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
						{
							if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<
								FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
							{
								if (SingleTargetHit->bHitReplaced)
								{
									HitReplaces.Add(i);
								}
							}
						}

						WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bIsTargetDataValid,
																	  HitReplaces);
					}
				}
			}
		}
#endif //WITH_SERVER_CODE
		// See if we still have ammo
		if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// We fired the weapon, add spread
			ULyraRangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
			check(WeaponData);
			WeaponData->AddSpread();

			UE_LOG(LogTemp, Warning,
			       TEXT("About to call OnRangedWeaponTargetDataReady. IsServer: %s, IsProjectile: %s"),
			       GetOwningActorFromActorInfo()->HasAuthority() ? TEXT("True") : TEXT("False"),
			       bProjectileWeapon ? TEXT("True") : TEXT("False"));

			// Let the blueprint do stuff like apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			UE_LOG(LogLyraAbilitySystem, Warning, TEXT("Weapon ability %s failed to commit (bIsTargetDataValid=%d)"),
			       *GetPathName(), bIsTargetDataValid ? 1 : 0);
			K2_EndAbility();
		}
	}

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle,
	                                                      CurrentActivationInfo.GetActivationPredictionKey());
}



void UOnMetal_RangedWeaponAbility::HandleProjectileHit(const FTBImpactParams& ImpactParams,FGameplayTag ApplicationTag)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHit called. IsServer: %s"),
		   GetOwningActorFromActorInfo()->HasAuthority() ? TEXT("True") : TEXT("False"));
	
	TArray<FHitResult> FoundHits;
	FoundHits.Add(ImpactParams.HitResult);
	if (FoundHits.Num() >0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FoundHits array has items")); //Log if there were hits
		if(GetOwningActorFromActorInfo()->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("Has Authority")); //Log if the server has authority
			AActor* TargetActor = ImpactParams.HitResult.GetActor();
			if(TargetActor != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("TargetActor is valid: %s"), *TargetActor->GetName()); //Log the target actor's name
				if(UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
				{
					UE_LOG(LogTemp, Warning, TEXT("TargetASC found")); //Log if the target has an ASC
					if (UObject* Payload = ImpactParams.Payload)
					{
						UE_LOG(LogTemp, Warning, TEXT("Payload found")); //Log if the payload is valid
						if (const UOnMetal_TBPayload* TBPayload = Cast<UOnMetal_TBPayload>(Payload))
						{
							UE_LOG(LogTemp, Warning, TEXT("TBPayload is valid")); //Log if the payload is a TBPayload
							UAbilitySystemComponent* SourceASC = ImpactParams.InstigatingActor->FindComponentByClass<UAbilitySystemComponent>();
							if(SourceASC)
								{
								UE_LOG(LogTemp, Warning, TEXT("SourceASC found")); //Log if the target has an ASC
								UE_LOG(LogTemp, Warning, TEXT("Applying Gameplay Effect")); //Log before applying the effect
								SourceASC->ApplyGameplayEffectSpecToTarget(*TBPayload->DamageEffectSpecHandle.Data.Get(),
									TargetASC);
								UE_LOG(LogTemp, Warning, TEXT("Gameplay Effect Applied")); //Log after applying the effect
								}
							else
							{
								UE_LOG(LogTemp, Error, TEXT("SourceASC is invalid")); //Log if the SourceASC is invalid
							}
						}
						else
						{
							UE_LOG(LogTemp, Error, TEXT("Invalid TBPayload cast")); //Log if the payload cast fails
						}
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Payload is null")); //Log if the payload is null
					}
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("TargetASC not found")); //Log if the target doesn't have an ASC
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TargetActor is null")); //Log if the target actor is null
			}
			
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Does not have Authority")); //Log if the server doesn't have authority
		}
		UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHit Bone: %s"),
			   *FoundHits[0].BoneName.ToString());
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No hits found")); //Log if no hits were registered
	}

	// Create a new target data handle with our custom data
	FGameplayAbilityTargetDataHandle TargetDataHandle;

	// Execute blueprint logic
	BlueprintOnHit.ExecuteIfBound(ImpactParams, TargetDataHandle);


	/*
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);
	
	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
    
	UOnMetal_WeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<UOnMetal_WeaponStateComponent>();
	check(WeaponStateComponent);
	
	// UOnMetal_TBPayload* Payload = Cast<UOnMetal_TBPayload>(ImpactParams.Payload);
	// check(Payload);

	// // Update the custom target data with hit information
	// Payload->CustomTargetData.HitResult = ImpactParams.HitResult;
	// Payload->CustomTargetData.bHitReplaced = false;
	// Payload->CustomTargetData.ProjectileId = ImpactParams.ProjectileId;

	// Create a new target data handle with our custom data
	FGameplayAbilityTargetDataHandle TargetDataHandle;
	//TargetDataHandle.Add(new FOnMetal_GameplayAbilityTargetData_SingleHitTarget(Payload->CustomTargetData));
	TargetDataHandle.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;
	
	// Add unconfirmed server-side hit markers
	TArray<FHitResult> FoundHits;
	FoundHits.Add(ImpactParams.HitResult);
	if (FoundHits.Num() >0)
	{
		//const int32 CartridgeID = Payload->CustomTargetData.CartridgeID;
		const int32 CartridgeID = FMath::Rand();
		
		for (const FHitResult& Hit : FoundHits)
		{
			FOnMetal_GameplayAbilityTargetData_SingleHitTarget* NewTargetData = new FOnMetal_GameplayAbilityTargetData_SingleHitTarget();
			NewTargetData->HitResult = Hit;
			NewTargetData->CartridgeID = CartridgeID;

			TargetDataHandle.Add(NewTargetData);
			
			UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *Hit.BoneName.ToString());
		}
		
	}
	if(WeaponStateComponent != nullptr)
	{
		WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetDataHandle, FoundHits, true);
	}
	
	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		
		// Create a local copy of the TargetDataHandle and transfer ownership
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(TargetDataHandle));
		
		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{

			FScopedPredictionWindow	ScopedPrediction(MyAbilityComponent);
			
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle,
													CurrentActivationInfo.GetActivationPredictionKey(),
													LocalTargetDataHandle, FGameplayTag(),
													MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;

#if WITH_SERVER_CODE
		// We always want to confirm hits for projectile weapons, so we remove the !bProjectileWeapon check
		if (Controller != nullptr)
		{
			if (Controller->GetLocalRole() == ROLE_Authority)
			{
				// Confirm hit markers
				if (WeaponStateComponent != nullptr)
				{
					TArray<uint8> HitReplaces;
					for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
					{
						if (FOnMetal_GameplayAbilityTargetData_SingleHitTarget* SingleTargetHit = 
							static_cast<FOnMetal_GameplayAbilityTargetData_SingleHitTarget*>(LocalTargetDataHandle.Get(i)))
						{
							if (SingleTargetHit->bHitReplaced)
							{
								HitReplaces.Add(i);
								// Add logging here
								UE_LOG(LogTemp, Warning, TEXT("Hit at index %d was replaced."), i);
								
							}
						}
					}

					WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bIsTargetDataValid, HitReplaces);
				}
			}
		}
#endif //WITH_SERVER_CODE
		
		// Execute blueprint logic
		BlueprintOnHit.ExecuteIfBound(ImpactParams, LocalTargetDataHandle);
		
		//HandleProjectileHitWrapper(LocalTargetDataHandle, ApplicationTag);
		// Remove Projectile from the Projectile Payload Array
		ProjectilePayloads.Remove(ImpactParams.ProjectileId);
	}*/
}

void UOnMetal_RangedWeaponAbility::HandleProjectileHitWrapper(const FGameplayAbilityTargetDataHandle& TargetData,
	FGameplayTag ApplicationTag)
{
	/*FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(TargetData)));

	if(LocalTargetDataHandle.Num() > 0)
	{
		if (FGameplayAbilityTargetData_SingleTargetHit* HitTargetData = static_cast<FGameplayAbilityTargetData_SingleTargetHit*> (LocalTargetDataHandle.Get(0)))
		{
			AActor* TargetActor = HitTargetData->HitResult.GetActor();
			if (TargetActor)
			{
			
			}
		}
		
	}*/
	/*// Extract Target Information from TargetData
	if (LocalTargetDataHandle.Num() > 0)
	{
		if (FGameplayAbilityTargetData_SingleTargetHit* HitTargetData = static_cast<FGameplayAbilityTargetData_SingleTargetHit*> (LocalTargetDataHandle.Get(0)))
		{
			AActor* TargetActor = HitTargetData->HitResult.GetActor();
			if (TargetActor)
			{
				// Get the Ability System Component of the Target Actor
				UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
				if (TargetASC)
				{
					const bool bShouldNotifySever = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
					if(bShouldNotifySever)
					{
						// Create the FGameplayAbilitySpec first
						FGameplayAbilitySpec AbilitySpec(UOnMetal_ProjectileImpactAbility::StaticClass(), 1, 0, this); 

						// Now pass it to GiveAbilityAndActivateOnce
						TargetASC->GiveAbilityAndActivateOnce(AbilitySpec); 
						// Grant the ProjectileImpactAbility to the Target Actor
						//TargetASC->GiveAbility(
						//FGameplayAbilitySpec(UOnMetal_ProjectileImpactAbility::StaticClass(), 1, 0, this)); 
					
						// Optionally, you can activate the ability immediately:
						//TargetASC->GiveAbilityAndActivateOnce(FGameplayAbilitySpec(UOnMetal_ProjectileImpactAbility::StaticClass(), 1, 0, this));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHitWrapper: Target Actor's Ability System Component not found"));
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHitWrapper: TargetData is empty"));
	}

	// --- Consume Target Data ---
	if (TargetData.Num() > 0)
	{
		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		if (MyAbilityComponent)
		{
			MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
		}
	}

	////////////////THIS LOGIC IS FOR A GAMEPLAY EVENT APPROACH AND CAN BE EXTENDED WITH A GAMEPLAY EVENT LISTENER ABILITY IN THE FUTURE////////////////
	/#1#/ UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	// // We've processed the data
	// MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(TargetData)));
	
	// --- Trigger Gameplay Event (Both Server and Client) ---
	FGameplayEventData Payload;
	Payload.EventTag = FGameplayTag::RequestGameplayTag("GameplayEvent.Projectile.Impact"); // Use your registered tag
	Payload.Instigator = GetAvatarActorFromActorInfo();

	// Extract Target Information from TargetData
	if (TargetData.Num() > 0)
	{
		// Cast to your specific target data struct (assuming you have one)
		if (FGameplayAbilityTargetData_SingleTargetHit* HitTargetData = static_cast<FGameplayAbilityTargetData_SingleTargetHit*> (LocalTargetDataHandle.Get(0)))
		{
			// Create a new target data handle to hold the HitResult
			FGameplayAbilityTargetDataHandle HitResultHandle;
			HitResultHandle.Add(new FGameplayAbilityTargetData_SingleTargetHit(HitTargetData->HitResult));
			Payload.TargetData = HitResultHandle;
			// Add any other relevant data from HitTargetData to Payload
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHitWrapper: TargetData is empty"));
	}

	// Get the Ability System Component and send the event
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetAvatarActorFromActorInfo());
	if (ASC)
	{
		ASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleProjectileHitWrapper: Ability System Component not found"));
	}

	// --- Consume Target Data ---
	if (TargetData.Num() > 0)
	{
		ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
	}#1#*/
}

void UOnMetal_RangedWeaponAbility::HandleProjectileComplete(const FTBProjectileId& CompletedProjectileId,
                                                            const TArray<FPredictProjectilePathPointData>& PathData,const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag)
{
	/*BlueprintOnComplete.ExecuteIfBound(CompletedProjectileId, PathData);
	
	UOnMetal_TBPayload* Payload = nullptr;
	if (UOnMetal_TBPayload** FoundPayload = ProjectilePayloads.Find(CompletedProjectileId))
	{
		Payload = *FoundPayload;
		ProjectilePayloads.Remove(CompletedProjectileId); // Clean up the mapping
	}

	if (Payload)
	{
	
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HandleProjectileComplete: Invalid or missing payload for ProjectileId: %s"), 
			   *CompletedProjectileId.Guid.ToString());
	}*/
	
}

void UOnMetal_RangedWeaponAbility::HandleProjectileExitHit(const FTBImpactParams& ImpactParams, const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag)
{
	/*BlueprintOnExitHit.ExecuteIfBound(ImpactParams);
	
		UOnMetal_TBPayload* Payload = Cast<UOnMetal_TBPayload>(ImpactParams.Payload);
		if (Payload)
		{
			// Clean up the payload from the map
			ProjectilePayloads.Remove(ImpactParams.ProjectileId);
        
			// Create TargetDataHandle for exit hit
			FGameplayAbilityTargetDataHandle TargetDataHandle = Payload->CreateTargetDataHandleFromImpact(ImpactParams.HitResult, ImpactParams.ProjectileId);

			UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
			if (MyAbilityComponent && CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
			{
				MyAbilityComponent->CallServerSetReplicatedTargetData(
					CurrentSpecHandle,
					CurrentActivationInfo.GetActivationPredictionKey(),
					TargetDataHandle,
					Payload->ApplicationTag,
					MyAbilityComponent->ScopedPredictionKey);
			}

			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HandleProjectileExitHit: Invalid or missing payload"));
		}*/
}

void UOnMetal_RangedWeaponAbility::HandleProjectileInjure(const FTBImpactParams& ImpactParams,
	const FTBProjectileInjuryParams& InjuryParams, const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag)
{
	/*BlueprintOnInjure.ExecuteIfBound(ImpactParams, InjuryParams);
	
		UOnMetal_TBPayload* Payload = Cast<UOnMetal_TBPayload>(ImpactParams.Payload);
		if (Payload)
		{
			// Clean up the payload from the map
			ProjectilePayloads.Remove(ImpactParams.ProjectileId);
        
			// Create TargetDataHandle for injury
			FGameplayAbilityTargetDataHandle TargetDataHandle = Payload->CreateTargetDataHandleFromImpact(ImpactParams.HitResult, ImpactParams.ProjectileId);

			UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
			if (MyAbilityComponent && CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority())
			{
				MyAbilityComponent->CallServerSetReplicatedTargetData(
					CurrentSpecHandle,
					CurrentActivationInfo.GetActivationPredictionKey(),
					TargetDataHandle,
					Payload->ApplicationTag,
					MyAbilityComponent->ScopedPredictionKey);
			}

			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("HandleProjectileInjure: Invalid or missing payload"));
		}*/
}

void UOnMetal_RangedWeaponAbility::HandleProjectileCompleteWrapper(const FGameplayAbilityTargetDataHandle& TargetData,
	FGameplayTag ApplicationTag)
{

	
}



void UOnMetal_RangedWeaponAbility::HandleProjectileExitHitWrapper(const FGameplayAbilityTargetDataHandle& TargetData,
	FGameplayTag ApplicationTag)
{

	
}

void UOnMetal_RangedWeaponAbility::HandleProjectileInjureWrapper(const FGameplayAbilityTargetDataHandle& TargetData,
	FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	
	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}


