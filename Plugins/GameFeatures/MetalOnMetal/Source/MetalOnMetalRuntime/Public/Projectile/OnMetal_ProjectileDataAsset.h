// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataAsset.h"

#include "OnMetal_ProjectileDataAsset.generated.h"

class NiagaraSystem;

USTRUCT(BlueprintType)
struct FOnMetalProjectileDebugData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	bool bDebugPath {false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	float LineThickness {1.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	bool bDebugImpact {false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	float ImpactRadius {5.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	float DebugLifetime {0.0f};
};

USTRUCT(BlueprintType)
struct FOnMetalProjectileParticleData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	TObjectPtr<UNiagaraSystem> Particle;
	// The delayed range before the particle spawns, such as a tracer with a 100 meter delay would have a value of 10000
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	float ParticleSpawnDelayDistance {0.0f};
	
	operator bool () const
	{
		return Particle != nullptr;
	}
};

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetal_ProjectileDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	double Velocity {75000.0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	double DragCoefficient {0.283};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	float Lifetime {6.0f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	TEnumAsByte<ECollisionChannel> CollisionChannel {ECC_Visibility};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	FOnMetalProjectileParticleData ParticleData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OnMetal|Projectile")
	FOnMetalProjectileDebugData DebugData;
	
	
};
