// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OnMetal_RangedProjectileAbility.h"
#include "Weapons/OnMetal_RangedWeaponInstance.h"
#include "Physics/LyraCollisionChannels.h"
#include "LyraLogChannels.h"
#include "AIController.h"
#include "NativeGameplayTags.h"
#include "Weapons/LyraWeaponStateComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "TerminalBallistics/Public/Core/TBStatics.h"
#include "DrawDebugHelpers.h"
#include "Weapons/OnMetal_ProjectileWeaponComp.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(OnMetal_RangedProjectileAbility)

/*namespace LyraConsoleVariables
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
		TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
		ECVF_Default);
}*/

// Weapon fire will be blocked/canceled if the player has this tag
//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_WeaponFireBlocked, "Ability.Weapon.NoFiring");
////////////////////////////////////////////////////////////////////////////////////

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

UOnMetal_RangedProjectileAbility::UOnMetal_RangedProjectileAbility(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	//SourceBlockedTags.AddTag(TAG_WeaponFireBlocked);
}



UOnMetal_RangedWeaponInstance* UOnMetal_RangedProjectileAbility::GetOnMetalWeaponInstance() const
{
	return Cast<UOnMetal_RangedWeaponInstance>(GetAssociatedEquipment());
}

bool UOnMetal_RangedProjectileAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);

	if (bResult)
	{
		if (GetOnMetalWeaponInstance() == nullptr)
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



int32 UOnMetal_RangedProjectileAbility::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
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
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
			{
				// If we hit something attached to a pawn, we're good
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

void UOnMetal_RangedProjectileAbility::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(/*out*/ AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

ECollisionChannel UOnMetal_RangedProjectileAbility::DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const
{
	return Lyra_TraceChannel_Weapon;
}

void UOnMetal_RangedProjectileAbility::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	APawn* const AvatarPawn = Cast<APawn>(GetAvatarActorFromActorInfo());

	UOnMetal_RangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
	if (AvatarPawn && AvatarPawn->IsLocallyControlled() && WeaponData)
	{
		FOnMetalRangedWeaponFiringInput InputData;
		InputData.WeaponData = WeaponData;
		InputData.bCanPlayBulletFX = (AvatarPawn->GetNetMode() != NM_DedicatedServer);

		//@TODO: Should do more complicated logic here when the player is close to a wall, etc...
		const FTransform TargetTransform = GetTargetingTransform(AvatarPawn, EOnMetalAbilityTargetingSource::CameraTowardsFocus);
		InputData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		InputData.StartTrace = TargetTransform.GetTranslation();

		InputData.EndAim = InputData.StartTrace + InputData.AimDir * WeaponData->GetMaxDamageRange();

#if ENABLE_DRAW_DEBUG
		// if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
		// {
		// 	static float DebugThickness = 2.0f;
		// 	DrawDebugLine(GetWorld(), InputData.StartTrace, InputData.StartTrace + (InputData.AimDir * 100.0f), FColor::Yellow, false, LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		// }
#endif

		TraceBulletsInCartridge(InputData, /*out*/ OutHits);
	}
}

FVector UOnMetal_RangedProjectileAbility::GetWeaponTargetingSourceLocation() const
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

FTransform UOnMetal_RangedProjectileAbility::GetTargetingTransform(APawn* SourcePawn,
	EOnMetalAbilityTargetingSource Source) const
{
	
	check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController(); 
	ULyraWeaponStateComponent* WeaponStateComponent = (SourcePawnController != nullptr) ? SourcePawnController->FindComponentByClass<ULyraWeaponStateComponent>() : nullptr;

	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EOnMetalAbilityTargetingSource::Custom);

	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;


	if ((Controller != nullptr) && ((Source == EOnMetalAbilityTargetingSource::CameraTowardsFocus) || (Source == EOnMetalAbilityTargetingSource::PawnTowardsFocus) || (Source == EOnMetalAbilityTargetingSource::WeaponTowardsFocus)))
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

		if (Source == EOnMetalAbilityTargetingSource::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EOnMetalAbilityTargetingSource::WeaponForward) || (Source == EOnMetalAbilityTargetingSource::WeaponTowardsFocus))
	{
		SourceLoc = GetWeaponTargetingSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EOnMetalAbilityTargetingSource::PawnTowardsFocus) || (Source == EOnMetalAbilityTargetingSource::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}




FHitResult UOnMetal_RangedProjectileAbility::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace,
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

FHitResult UOnMetal_RangedProjectileAbility::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace,
	float SweepRadius, bool bIsSimulated, TArray<FHitResult>& OutHits) const
{
// #if ENABLE_DRAW_DEBUG
// 	if (LyraConsoleVariables::DrawBulletTracesDuration > 0.0f)
// 	{
// 		static float DebugThickness = 1.0f;
// 		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, LyraConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
// 	}
// #endif // ENABLE_DRAW_DEBUG

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

void UOnMetal_RangedProjectileAbility::TraceBulletsInCartridge(const FOnMetalRangedWeaponFiringInput& InputData,
	TArray<FHitResult>& OutHits)
{
	/*
	UOnMetal_RangedWeaponInstance* WeaponData = InputData.WeaponData;
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

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), /*bIsSimulated=#1# false, /*out#1# AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
// #if ENABLE_DRAW_DEBUG
// 			if (LyraConsoleVariables::DrawBulletHitDuration > 0.0f)
// 			{
// 				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, LyraConsoleVariables::DrawBulletHitRadius, FColor::Red, false, LyraConsoleVariables::DrawBulletHitRadius);
// 			}
// #endif

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

void UOnMetal_RangedProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// Bind target data callback
	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	OnTargetDataReadyCallbackDelegateHandle = MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	// Update the last firing time
	UOnMetal_RangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
	check(WeaponData);
	WeaponData->UpdateFiringTime();

	BindTBDelegates();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UOnMetal_RangedProjectileAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
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

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		// Unbind Terminal Ballistics delegates
		OnBulletCompleteDelegate.Unbind();
		OnBulletHitDelegate.Unbind();
		OnBulletExitHitDelegate.Unbind();
		OnBulletInjureDelegate.Unbind();
	
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UOnMetal_RangedProjectileAbility::OnBulletComplete(const FTBProjectileId Id,
	const TArray<FPredictProjectilePathPointData>& PathResults)
{
	// Handle bullet completion
	//UE_LOG(LogTemp, Log, TEXT("Bullet %s completed its trajectory"), *Id.ToString());
    
	// You might want to clean up any stored data for this projectile
	// If this was the last active projectile, you might want to end the ability
	// K2_EndAbility();
}

void UOnMetal_RangedProjectileAbility::OnBulletHit(const FTBImpactParams& ImpactParams)
{
	// Handle bullet hit
	UE_LOG(LogTemp, Log, TEXT("Bullet hit at location: %s"), *ImpactParams.HitResult.Location.ToString());

	if (ImpactParams.Payload)
	{
		check(ImpactParams.Payload->IsA<UOnMetalProjectilePayload>());
        
		// FGameplayAbilityTargetDataHandle TargetData = static_cast<UOnMetalProjectilePayload*>(ImpactParams.Payload)->CreateTargetData(ImpactParams);

		// Confirm the hit with the ProjectileWeaponComponent
		UOnMetal_ProjectileWeaponComp* ProjectileWeaponComponent = GetOwningActorFromActorInfo()->FindComponentByClass<UOnMetal_ProjectileWeaponComp>();
		if (ProjectileWeaponComponent)
		{
			ProjectileWeaponComponent->ConfirmProjectileHit(ImpactParams.ProjectileId, ImpactParams.HitResult);
		}
		

		// // Call the blueprint implementable event
		// OnBulletHitBP(TargetData);
	}
}

void UOnMetal_RangedProjectileAbility::OnBulletExitHit(const FTBImpactParams& ImpactParams)
{
	// Handle bullet exit hit
	UE_LOG(LogTemp, Log, TEXT("Bullet exited at location: %s"), *ImpactParams.HitResult.Location.ToString());
	// You might want to spawn effects or apply additional gameplay logic here
}

void UOnMetal_RangedProjectileAbility::OnBulletInjure(const FTBImpactParams& ImpactParams,
	const FTBProjectileInjuryParams& InjuryParams)
{
	// Handle bullet injury
	UE_LOG(LogTemp, Log, TEXT("Bullet caused injury at location: %s"), *ImpactParams.HitResult.Location.ToString());
	// Here you might want to apply more specific damage or effects based on the injury parameters
}


void UOnMetal_RangedProjectileAbility::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
                                                                 FGameplayTag ApplicationTag)
{
		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	if (const FGameplayAbilitySpec* AbilitySpec = MyAbilityComponent->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(MyAbilityComponent);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			MyAbilityComponent->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyAbilityComponent->ScopedPredictionKey);
		}

		const bool bIsTargetDataValid = true;

		bool bProjectileWeapon = false;

#if WITH_SERVER_CODE
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority)
				{
					// Confirm hit markers
					if (ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>())
					{
						TArray<uint8> HitReplaces;
						for (uint8 i = 0; (i < LocalTargetDataHandle.Num()) && (i < 255); ++i)
						{
							if (FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(LocalTargetDataHandle.Get(i)))
							{
								if (SingleTargetHit->bHitReplaced)
								{
									HitReplaces.Add(i);
								}
							}
						}

						WeaponStateComponent->ClientConfirmTargetData(LocalTargetDataHandle.UniqueId, bIsTargetDataValid, HitReplaces);
					}

				}
			}
		}
#endif //WITH_SERVER_CODE


		// See if we still have ammo
		if (bIsTargetDataValid && CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// We fired the weapon, add spread
			UOnMetal_RangedWeaponInstance* WeaponData = GetOnMetalWeaponInstance();
			check(WeaponData);
			WeaponData->AddSpread();

			// Let the blueprint do stuff like apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
		else
		{
			UE_LOG(LogLyraAbilitySystem, Warning, TEXT("Weapon ability %s failed to commit (bIsTargetDataValid=%d)"), *GetPathName(), bIsTargetDataValid ? 1 : 0);
			K2_EndAbility();
		}
	}

	// We've processed the data
	MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

FOnMetalProjectileLaunchParams UOnMetal_RangedProjectileAbility::CreateLaunchParams()
{
	UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance();
	check(WeaponInstance);

	APawn* OwnerPawn = Cast<APawn>(GetOwningActorFromActorInfo());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector FireLocation = GetWeaponTargetingSourceLocation();
	FRotator FireRotation = OwnerPawn->GetBaseAimRotation();

	// // Create the payload
UOnMetalProjectilePayload* Payload = NewObject<UOnMetalProjectilePayload>();
	// Payload->SourceASC = CurrentActorInfo->AbilitySystemComponent;
	// Payload->AbilityHandle = CurrentSpecHandle;
	// Payload->EffectContext = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);

	FTBLaunchParams LaunchParams = UTerminalBallisticsStatics::MakeLaunchParams(
		WeaponInstance->GetMaxDamageRange(),  // ProjectileSpeed
		WeaponInstance->GetMaxDamageRange(),  // EffectiveRange
		1.0f,  // Timescale
		FireLocation,
		FireRotation,
		TArray<AActor*>(),  // ToIgnore
		UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		ECC_GameTraceChannel1,  // Assuming this is your weapon trace channel
		true,  // bIgnoreOwner
		true,  // bAddToOwnerVelocity
		false,  // bForceNoTracer
		OwnerPawn,
		OwnerController,
		1.0f,  // GravityMultiplier
		10.0f,  // OwnerIgnoreDistance
		25.0f,  // TracerActivationDistance
		Payload  // Our payload
	);
	
	return LaunchParams;
}

void UOnMetal_RangedProjectileAbility::StartRangedWeaponTargeting()
{

check(CurrentActorInfo);

AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
check(AvatarActor);

UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
check(MyAbilityComponent);

AController* Controller = GetControllerFromActorInfo();
check(Controller);
ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>();

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
		FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
		NewTargetData->HitResult = FoundHit;
		NewTargetData->CartridgeID = CartridgeID;

		TargetData.Add(NewTargetData);
	}
}

// Send hit marker information
const bool bProjectileWeapon = false;
if (!bProjectileWeapon && (WeaponStateComponent != nullptr))
{
	WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
}

// Process the target data immediately
OnTargetDataReadyCallback(TargetData, FGameplayTag());
}


FTBLaunchParams UOnMetal_RangedProjectileAbility::CreateTBLaunchParams() const
{
	UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance();
	check(WeaponInstance);

	APawn* OwnerPawn = Cast<APawn>(GetOwningActorFromActorInfo());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	FVector FireLocation = GetWeaponTargetingSourceLocation();
	FRotator FireRotation = OwnerPawn->GetBaseAimRotation();

	UOnMetalProjectilePayload* Payload = NewObject<UOnMetalProjectilePayload>();
	Payload->SourceASC = CurrentActorInfo->AbilitySystemComponent;
	Payload->AbilityHandle = CurrentSpecHandle;
	Payload->EffectContext = MakeEffectContext(CurrentSpecHandle, CurrentActorInfo);

	return UTerminalBallisticsStatics::MakeLaunchParams(
		WeaponInstance->GetMaxDamageRange(),  // ProjectileSpeed
		WeaponInstance->GetMaxDamageRange(),  // EffectiveRange
		1.0f,  // Timescale
		FireLocation,
		FireRotation,
		TArray<AActor*>(),  // ToIgnore
		UTerminalBallisticsStatics::GetDefaultObjectTypes(),
		ECC_GameTraceChannel1,  // Assuming this is your weapon trace channel
		true,  // bIgnoreOwner
		true,  // bAddToOwnerVelocity
		false,  // bForceNoTracer
		OwnerPawn,
		OwnerController,
		1.0f,  // GravityMultiplier
		10.0f,  // OwnerIgnoreDistance
		25.0f,  // TracerActivationDistance
		Payload
	);
}


void UOnMetal_RangedProjectileAbility::BindTBDelegates()
{
	OnBulletCompleteDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletComplete);
	OnBulletHitDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletHit);
	OnBulletExitHitDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletExitHit);
	OnBulletInjureDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletInjure);
}


void UOnMetal_RangedProjectileAbility::FireTBProjectile()
{
	UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance();
	if (!WeaponInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid weapon instance type for OnMetal_RangedProjectileAbility"));
		K2_EndAbility();
		return;
	}

	FTBLaunchParams LaunchParams = CreateTBLaunchParams();

	TSoftObjectPtr<UBulletDataAsset> BulletDataAsset = WeaponInstance->GetBulletDataAsset();
	if (!BulletDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid BulletDataAsset for OnMetal_RangedProjectileAbility"));
		K2_EndAbility();
		return;
	}

	// Ensure delegates are bound
	 if (!OnBulletCompleteDelegate.IsBound() || !OnBulletHitDelegate.IsBound() ||
	 	!OnBulletExitHitDelegate.IsBound() || !OnBulletInjureDelegate.IsBound())
	 {
		UE_LOG(LogTemp, Warning, TEXT("One or more Terminal Ballistics delegates are not bound. Binding now."));
	 	BindTBDelegates();
	 }
	
	FTBProjectileId ProjectileId = UTerminalBallisticsStatics::AddAndFireBulletWithCallbacks(
		BulletDataAsset,
		LaunchParams,
		OnBulletCompleteDelegate,
		OnBulletHitDelegate,
		OnBulletExitHitDelegate,
		OnBulletInjureDelegate,
		FTBProjectileId::CreateNew(),
		0  // Debug type
	);

	if (!ProjectileId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to fire bullet in OnMetal_RangedProjectileAbility"));
		K2_EndAbility();
		return;
	}

	// Send hit marker information
	// Note: We're not passing FoundHits here because Terminal Ballistics will handle hit detection

	
	// // Store the launch params for later use in callbacks 
	// ActiveProjectiles.Add(ProjectileId, LaunchParams);

	// We fired the weapon, add spread
	WeaponInstance->AddSpread();
}

/*FGameplayAbilityTargetDataHandle UOnMetalProjectilePayload::CreateTargetData(const FTBImpactParams& ImpactParams) const
{
	FGameplayAbilityTargetDataHandle TargetData;

	FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();

	NewTargetData->HitResult = ImpactParams.HitResult;
	NewTargetData->CartridgeID = ImpactParams.ProjectileId.Guid.A;

	// // Add GAS-related information
	// NewTargetData->SourceASC = SourceASC;
	// NewTargetData->AbilityHandle = AbilityHandle;
	// NewTargetData->EffectContext = EffectContext;

	TargetData.Add(NewTargetData);

	return TargetData;
}*/

////////////////////////////////////////////////////////////////////////////////////
////////////////////////PREVIOUSLY DEVELOPED TB PROJECTILE CODE//////////////////////
////////////////////////////////////////////////////////////////////////////////////
/*void UOnMetal_RangedProjectileAbility::StartRangedWeaponTargeting()
{
    check(CurrentActorInfo);

    AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
    check(AvatarActor);

    UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
    check(MyAbilityComponent);

    AController* Controller = GetControllerFromActorInfo();
    check(Controller);
    ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>();

    FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

    // Get the OnMetal weapon instance
    UOnMetal_RangedWeaponInstance* WeaponInstance = GetOnMetalWeaponInstance();
    if (!WeaponInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid weapon instance type for OnMetal_RangedProjectileAbility"));
        K2_EndAbility();
        return;
    }

    // Create launch params for Terminal Ballistics
    FOnMetalLaunchParams LaunchParams = CreateLaunchParams();

    // Get the bullet data asset
    TSoftObjectPtr<UBulletDataAsset> BulletDataAsset = WeaponInstance->GetBulletDataAsset();
	if (!BulletDataAsset.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid BulletDataAsset for OnMetal_RangedProjectileAbility"));
		K2_EndAbility();
		return;
	}

	// Bind the delegates
	OnBulletCompleteDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletComplete);
	OnBulletHitDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletHit);
	OnBulletExitHitDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletExitHit);
	OnBulletInjureDelegate.BindDynamic(this, &UOnMetal_RangedProjectileAbility::OnBulletInjure);

	// Fire the bullet using Terminal Ballistics
	FTBProjectileId ProjectileId = UTerminalBallisticsStatics::AddAndFireBulletWithCallbacks(
		BulletDataAsset,
		LaunchParams,
		OnBulletCompleteDelegate,
		OnBulletHitDelegate,
		OnBulletExitHitDelegate,
		OnBulletInjureDelegate,
		FTBProjectileId::CreateNew(),
		0  // Debug type
	);
    if (!ProjectileId.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to fire bullet in OnMetal_RangedProjectileAbility"));
        K2_EndAbility();
        return;
    }

    // Store the launch params for later use in callbacks
    //ActiveProjectiles.Add(ProjectileId, LaunchParams);

    // We fired the weapon, add spread
    WeaponInstance->AddSpread();

    // Fill out the target data
    FGameplayAbilityTargetDataHandle TargetData;
    TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

    // Add the projectile ID to the target data
    FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
    NewTargetData->CartridgeID = ProjectileId.Guid.A;  // Using part of the GUID as CartridgeID
    TargetData.Add(NewTargetData);

    // Send hit marker information
    // Note: We're not passing FoundHits here because Terminal Ballistics will handle hit detection
    if (WeaponStateComponent != nullptr)
    {
        WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, TArray<FHitResult>());
    }

    // Process the target data immediately
    OnTargetDataReadyCallback(TargetData, FGameplayTag());
}*/



/*void UOnMetal_RangedProjectileAbility::StartRangedWeaponTargeting()
{
check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyAbilityComponent);

	AController* Controller = GetControllerFromActorInfo();
	check(Controller);
	ULyraWeaponStateComponent* WeaponStateComponent = Controller->FindComponentByClass<ULyraWeaponStateComponent>();

	FScopedPredictionWindow ScopedPrediction(MyAbilityComponent, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(/*out#1# FoundHits);

// Fill out the target data from the hit results
FGameplayAbilityTargetDataHandle TargetData;
TargetData.UniqueId = WeaponStateComponent ? WeaponStateComponent->GetUnconfirmedServerSideHitMarkerCount() : 0;

if (FoundHits.Num() > 0)
{
	const int32 CartridgeID = FMath::Rand();

	for (const FHitResult& FoundHit : FoundHits)
	{
		FLyraGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FLyraGameplayAbilityTargetData_SingleTargetHit();
		NewTargetData->HitResult = FoundHit;
		NewTargetData->CartridgeID = CartridgeID;

		TargetData.Add(NewTargetData);
	}
}

// Send hit marker information
const bool bProjectileWeapon = false;
if (!bProjectileWeapon && (WeaponStateComponent != nullptr))
{
	WeaponStateComponent->AddUnconfirmedServerSideHitMarkers(TargetData, FoundHits);
}

// Process the target data immediately
OnTargetDataReadyCallback(TargetData, FGameplayTag());
}*/



