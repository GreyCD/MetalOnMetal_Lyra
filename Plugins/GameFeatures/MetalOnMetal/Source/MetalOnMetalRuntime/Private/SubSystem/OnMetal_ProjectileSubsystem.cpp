// Fill out your copyright notice in the Description page of Project Settings.


#include "SubSystem/OnMetal_ProjectileSubsystem.h"

#include "AbilitySystemGlobals.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/WindDirectionalSourceComponent.h"
#include "Engine/WindDirectionalSource.h"
#include "Engine/World.h"
#include "SceneManagement.h"
#include "AbilitySystem/LyraGameplayAbilityTargetData_SingleTargetHit.h"
#include "Kismet/KismetMathLibrary.h"

DECLARE_CYCLE_STAT(TEXT("Tick"), STAT_OnMetalTick, STATGROUP_OnMetalProjectile);


constexpr double ProjectileGravity = -982.0;

void FOnMetal_ProjectileData::Initialize(UWorld* World)
{
	Location = LaunchTransform.GetLocation();
	ForwardVelocity = LaunchTransform.GetRotation().GetForwardVector() * Velocity;
	WorldPtr = World;

	if (bHandleVisualComponent && !VisualComponent && ParticleData && ParticleData.ParticleSpawnDelayDistance == 0.0f)
	{
		VisualComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldPtr, ParticleData.Particle, Location, ForwardVelocity.Rotation());
	}
}

void FOnMetal_ProjectileData::PerformStep(const FVector& Wind, float DeltaSeconds)
{
	const double DragForce = 0.5f * FMath::Square(ForwardVelocity.Size() * 0.01) * 1.225 * (UE_DOUBLE_PI * FMath::Square(0.0057)) * DragCoefficient;

	const FVector ProjectileGravityVelocity = FVector(0.0, 0.0, ProjectileGravity);
	const FVector DragVelocity = (FVector(DragForce) * (ForwardVelocity * 0.01)) * 10.0;
	const FVector AccumulativeVelocity = -DragVelocity + ProjectileGravityVelocity + Wind;
	ForwardVelocity += AccumulativeVelocity * DeltaSeconds;
	End = Location + ForwardVelocity * DeltaSeconds;

#if WITH_EDITOR
	if (DebugData.bDebugPath)
	{
		DrawDebugLine(WorldPtr, Location, End, FColor::Red, false, DebugData.DebugLifetime, 0, DebugData.LineThickness);
	}
#endif
	PreviousLocation = Location;
	Location = End;
}

FHitResult FOnMetal_ProjectileData::PerformTrace() const
{
	FHitResult NewHitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bReturnPhysicalMaterial = true;
	WorldPtr->LineTraceSingleByChannel(NewHitResult, PreviousLocation, End, CollisionChannel, Params);
	return NewHitResult;
}

void UOnMetal_ProjectileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	World = GetWorld();
}

void UOnMetal_ProjectileSubsystem::Deinitialize()
{
	Super::Deinitialize();

	for (int32 i = 0; i < Projectiles.Num(); ++i)
	{
		RemoveProjectile(i);
	}
}

void UOnMetal_ProjectileSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!World)
	{
		World = GetWorld();
	}
	
	SCOPE_CYCLE_COUNTER(STAT_OnMetalTick);
	const float CurrentTimeSeconds = GetWorld()->GetTimeSeconds();
	for (int32 i = 0; i < Projectiles.Num(); ++i)
	{
		FOnMetal_ProjectileData& Projectile = Projectiles[i];
		PerformProjectileStep(Projectile, GetWindSourceVelocity(Projectile));
		if (Projectile.bHadImpact || CurrentTimeSeconds - Projectile.LaunchStartTime > Projectile.Lifetime)
		{
			RemoveProjectile(i);
		}
	}
}

// FGameplayAbilityTargetDataHandle UOnMetal_ProjectileSubsystem::GetProjectileTargetData(int32 ProjectileID) const
// {
// 	for (const FOnMetal_ProjectileData& Projectile : Projectiles)
// 	{
// 		if (Projectile.ProjectileID == ProjectileID)
// 		{
// 			return Projectile.TargetDataHandle;
// 		}
// 	}
// 	return FGameplayAbilityTargetDataHandle();
// }

bool UOnMetal_ProjectileSubsystem::IsValidHit(const FHitResult& HitResult) const
{
	// Check if the hit actor is valid
	if (!HitResult.GetActor())
	{
		return false; // No actor was hit
	}

	//Check if the hit actor has an AbilitySystemComponent (if applicable)
	if (!HitResult.GetActor()->FindComponentByClass<UAbilitySystemComponent>())
	{
		return false; // Actor cannot take damage
	}

	// Add more specific validation checks
	// For example:
	// - Check distance between shooter and target
	// - Check line of sight between shooter and target
	// - Check if the target is on the same team as the shooter

	return true; // Hit is valid
}

void UOnMetal_ProjectileSubsystem::ServerHandleProjectileHit_Implementation(
	const FGameplayAbilityTargetDataHandle& TargetData, const FHitResult& HitResult, UAbilitySystemComponent* SourceASC)
{
	// 1. Server-Side Validation (Implement your validation logic here)
	if (IsValidHit(HitResult))
	{
		// Send Gameplay Event with TargetData as payload
		if (SourceASC)
		{
			FGameplayEventData Payload;
			Payload.TargetData = TargetData;
			SourceASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Projectile.Impact")), &Payload);
		}
	}
}

bool UOnMetal_ProjectileSubsystem::ServerHandleProjectileHit_Validate(const FGameplayAbilityTargetDataHandle& TargetData,
	const FHitResult& HitResult, UAbilitySystemComponent* SourceASC)
{
	return true;
}

void UOnMetal_ProjectileSubsystem::PerformProjectileStep(FOnMetal_ProjectileData& Projectile, const FVector& Wind)
{
	Projectile.PerformStep(Wind, Projectile.WorldPtr->DeltaTimeSeconds);
	Projectile.OnPositionUpdate.ExecuteIfBound(Projectile.Location, Projectile.ForwardVelocity, Projectile.ProjectileID);

	if (Projectile.bHandleVisualComponent)
	{
		if (Projectile.VisualComponent)
		{
			//Projectile.VisualComponent->SetWorldLocation(Projectile.Location);
			Projectile.VisualComponent->SetWorldLocationAndRotation(Projectile.Location, Projectile.ForwardVelocity.Rotation());
		}
		else if (Projectile.ParticleData && FVector::Dist(Projectile.LaunchTransform.GetLocation(), Projectile.Location) > Projectile.ParticleData.ParticleSpawnDelayDistance)
		{
			Projectile.VisualComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(Projectile.WorldPtr, Projectile.ParticleData.Particle, Projectile.Location);
		}
	}
	
	const FHitResult NewHitResult = Projectile.PerformTrace();
	if (NewHitResult.GetActor() != Projectile.HitResult.GetActor())
	{
		Projectile.HitResult = NewHitResult;
		Projectile.bHadImpact = true;

		AActor* HitActor = NewHitResult.GetActor();
		Projectile.HitASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(NewHitResult);
		TargetDataHandle.Add(TargetData);
		//TargetDataHandle.Add(NewObject<FLyraGameplayAbilityTargetData_SingleTargetHit>(this, TEXT("SingleTargetHit"), NewHitResult));
		Projectile.TargetDataHandle = TargetDataHandle;
		if (GetWorld()->GetNetMode() < NM_Client) // Server Only
		{
			ServerHandleProjectileHit(Projectile.TargetDataHandle, Projectile.HitResult, Projectile.SourceASC);
		}
		else // Client Prediction
		{
			OnProjectileHitTarget.Broadcast(Projectile.TargetDataHandle, NewHitResult, Projectile.ProjectileOwner); // Broadcast hit with additional params
		}
		//OnProjectileHitTarget.Broadcast(TargetDataHandle);

		//Call the OnTargetDataReady delegate
		//Projectile.OnProjectileTargetDataReady.ExecuteIfBound(TargetDataHandle);
		
		Projectile.OnImpact.ExecuteIfBound(Projectile.HitResult, Projectile.ForwardVelocity, Projectile.ProjectileID, Projectile.DamageEffectSpecHandle, Projectile.ProjectileOwner);
#if WITH_EDITOR
		if (Projectile.DebugData.bDebugPath)
		{
			DrawDebugSphere(World, Projectile.HitResult.Location, Projectile.DebugData.ImpactRadius, 6.0f, FColor::Green, false, Projectile.DebugData.DebugLifetime, 0, 1.0f);
		}
#endif
	}
	
}

FVector UOnMetal_ProjectileSubsystem::GetWindSourceVelocity(const FOnMetal_ProjectileData& Projectile)
{
	FVector WindVelocity = FVector::ZeroVector;
	float CurrentClosestWind = 100000000.0f;
	UWindDirectionalSourceComponent* ClosestWindSource = nullptr;
	for (UWindDirectionalSourceComponent* WindSource : WindSources)
	{
		if (WindSource)
		{
			const FVector WindLocation = WindSource->GetComponentLocation();
			const float WindDistance = FVector::Dist(WindLocation, Projectile.Location);
			if (WindDistance < CurrentClosestWind)
			{
				CurrentClosestWind = WindDistance;
				ClosestWindSource = WindSource;
			}
		}
	}
	if (ClosestWindSource)
	{
		FWindData WindData;
		float Weight;
		ClosestWindSource->GetWindParameters(Projectile.Location, WindData, Weight);
		WindVelocity = WindData.Direction * WindData.Speed;
	}
	return WindVelocity;
}

void UOnMetal_ProjectileSubsystem::RemoveProjectile(const int32 Index)
{
	FOnMetal_ProjectileData& Projectile = Projectiles[Index];
	if (Projectile.VisualComponent)
	{
		Projectile.VisualComponent->DestroyComponent();
	}
	Projectiles.RemoveAt(Index, 1, false);
}

void UOnMetal_ProjectileSubsystem::SetWindSources(TArray<AWindDirectionalSource*> WindDirectionalSources)
{
	WindSources.Empty(WindDirectionalSources.Num());
	for (AWindDirectionalSource* WindDirectionalSource : WindDirectionalSources)
	{
		WindSources.Add(WindDirectionalSource->GetComponent());
	}
	
}

void UOnMetal_ProjectileSubsystem::FireProjectile(const int32 ProjectileID, UOnMetal_ProjectileDataAsset* DataAsset,
                                                  const TArray<AActor*>& ActorsToIgnore, const FTransform& LaunchTransform,
                                                  UPrimitiveComponent* VisualComponentOverride, FOnMetal_OnProjectileImpact OnImpact,
                                                  FOnMetal_OnProjectilePositionUpdate OnPositionUpdate, FGameplayEffectSpecHandle DamageEffectSpecHandle,
                                                  AActor* ProjectileOwner)
{
	if (ensureMsgf(DataAsset, TEXT("Data Asset INVALID for FireProjectile")))
	{
		// Create projectile data
		FOnMetal_ProjectileData* Projectile = new FOnMetal_ProjectileData();

		if(Projectile == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("ProjectileData is null"));
			return;
		}
		if(World != nullptr)
		{
		
		}
		else{
			UE_LOG(LogTemp, Error, TEXT("World is null"));
		}
		
		Projectile->ProjectileID = ProjectileID;
		Projectile->Velocity = DataAsset->Velocity;
		Projectile->DragCoefficient = DataAsset->DragCoefficient;
		Projectile->Lifetime = DataAsset->Lifetime;
		Projectile->CollisionChannel = DataAsset->CollisionChannel;
		Projectile->ActorsToIgnore = ActorsToIgnore;
		Projectile->ParticleData = DataAsset->ParticleData;
		Projectile->VisualComponent = VisualComponentOverride;
		Projectile->DebugData = DataAsset->DebugData;
		Projectile->ProjectileOwner = ProjectileOwner;
	

		// Set the damage effect spec handle
		Projectile->DamageEffectSpecHandle = DamageEffectSpecHandle;
		
		Projectile->OnImpact = OnImpact;
		Projectile->OnPositionUpdate = OnPositionUpdate;
		Projectile->LaunchTransform = LaunchTransform;
		if(Projectile && World) // Check if both Projectile and World are valid before using
		{
			Projectile->LaunchStartTime = World->GetTimeSeconds();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Either Projectile or World is null"));
		}
		/*if(Projectile)
		{
			
			if(GetWorld())
			{
				Projectile->LaunchStartTime = GetWorld()->GetTimeSeconds();
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("World is null"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Projectile is null"));
		}*/
		//Projectile->LaunchStartTime = World->GetTimeSeconds();
		Projectile->bHandleVisualComponent = Projectile->VisualComponent || DataAsset->ParticleData;
	
		Projectiles.Add(*Projectile);
		Projectile->Initialize(World);
	}
	
}


bool UOnMetal_ProjectileSubsystem::GetProjectileZero(UOnMetal_ProjectileDataAsset* DataAsset, double Distance,
                                                     FTransform LaunchTransform, FTransform OpticAimSocket, FRotator& OUTLookAtRotation)
{
	if (ensureMsgf(DataAsset, TEXT("Data Asset INVALID for GetProjectileZero")))
	{
		if (!LaunchTransform.GetLocation().Equals(FVector::ZeroVector) && !OpticAimSocket.GetLocation().Equals(FVector::ZeroVector))
		{
			const double ZOffset = UKismetMathLibrary::MakeRelativeTransform(OpticAimSocket, LaunchTransform).GetLocation().Z;
			LaunchTransform.SetRotation(FQuat());
			FVector LookAtStart = LaunchTransform.GetLocation();
			LookAtStart.Z += ZOffset;

			FVector ProjectileEndLocation;
			GetProjectileLocationAtDistance(DataAsset, Distance, LaunchTransform, ProjectileEndLocation);
			OUTLookAtRotation = UKismetMathLibrary::FindLookAtRotation(LookAtStart, ProjectileEndLocation);
			return true;
		}
	}
	return false;
}

bool UOnMetal_ProjectileSubsystem::GetProjectileZeroAtLocation(FRotator& OUTLookAtRotation, const FVector& Location,
	FTransform LaunchTransform, FTransform OpticAimSocket)
{
	if (!LaunchTransform.GetLocation().Equals(FVector::ZeroVector) && !Location.Equals(FVector::ZeroVector))
	{
		const double ZOffset = UKismetMathLibrary::MakeRelativeTransform(OpticAimSocket, LaunchTransform).GetLocation().Z;
		LaunchTransform.SetRotation(FQuat());
		FVector LookAtStart = LaunchTransform.GetLocation();
		LookAtStart.Z += ZOffset;

		OUTLookAtRotation = UKismetMathLibrary::FindLookAtRotation(LookAtStart, Location);
		return true;
	}
	return false;
}

bool UOnMetal_ProjectileSubsystem::GetProjectileLocationAtDistance(UOnMetal_ProjectileDataAsset* DataAsset,
	double Distance, FTransform LaunchTransform, FVector& OUTLocation)
{
	if (ensureMsgf(DataAsset, TEXT("Data Asset INVALID for GetProjectileLocationAtDistance")))
	{
		if (World->DeltaTimeSeconds > 0.0f)
		{
			LaunchTransform.SetRotation(FQuat(0.0));
			Distance *= 100.0f;
			FOnMetal_ProjectileData Projectile;
			Projectile.DragCoefficient = DataAsset->DragCoefficient;
			Projectile.Velocity = DataAsset->Velocity;
			Projectile.LaunchTransform = LaunchTransform;
			Projectile.Initialize(GetWorld());
		
			const FVector StartLocation = LaunchTransform.GetLocation();
			int32 Counter = 0;
			while (FVector::Distance(StartLocation, Projectile.Location) < Distance && Counter < 5000)
			{
				++Counter;
				Projectile.PerformStep(FVector::ZeroVector, World->DeltaTimeSeconds);
			}

			//DrawDebugSphere(World, Projectile.Location, 20.0f, 3.0f, FColor::Yellow, true, -1, 0);
			OUTLocation = Projectile.Location;
			return true;
		}
	}
	OUTLocation = FVector::ZeroVector;
	return false;
}









