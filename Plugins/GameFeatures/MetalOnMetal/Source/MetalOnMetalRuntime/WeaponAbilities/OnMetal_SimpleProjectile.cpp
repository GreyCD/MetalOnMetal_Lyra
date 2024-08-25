// Fill out your copyright notice in the Description page of Project Settings.


#include "OnMetal_SimpleProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AOnMetal_SimpleProjectile::AOnMetal_SimpleProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->MaxSpeed = 0.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

// Called when the game starts or when spawned
void AOnMetal_SimpleProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AOnMetal_SimpleProjectile::OnSphereOverlap);
	
	
}

void AOnMetal_SimpleProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
	{
		// 1. Get the Ability System Component associated with the projectile's owner
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
		if (!SourceASC) 
		{
			// Handle the case where SourceASC is not found (e.g., log an error)
			return; 
		}

		// 2. Get all actors associated with the SourceASC
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(SourceASC->GetAvatarActor()); // Add the Avatar actor

		// Add any actors attached to the Avatar or its children
		TArray<AActor*> AttachedActors;
		SourceASC->GetAvatarActor()->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors)
		{
			ActorsToIgnore.Add(AttachedActor);
			AttachedActor->GetAttachedActors(AttachedActors); // Recursively get attached actors
		}

		// 3. Check if OtherActor is in the ActorsToIgnore list
		if (ActorsToIgnore.Contains(OtherActor))
		{
			// Ignore the overlap if OtherActor is associated with the SourceASC
			return;
		}

		// 4. Proceed with the rest of the logic if OtherActor is not to be ignored
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetASC found"));
			SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);
		}

		Destroy();
		
		/*if(UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			UE_LOG(LogTemp, Warning, TEXT("TargetASC found")); //Log if the target has an ASC
			UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
			if(SourceASC)
			{
				UE_LOG(LogTemp, Warning, TEXT("SourceASC found")); //Log if the target has an ASC
				SourceASC->ApplyGameplayEffectSpecToTarget(*DamageEffectSpecHandle.Data.Get(), TargetASC);
			}
		}
		
		Destroy();*/
	}
}



