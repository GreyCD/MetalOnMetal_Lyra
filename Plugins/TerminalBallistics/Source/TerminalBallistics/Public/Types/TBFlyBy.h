// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/TBConfiguration.h"
#include "Misc/TBMacrosAndFunctions.h"
#include "Types/TBEnums.h"
#include "Types/TBProjectileId.h"
#include "UObject/Object.h"

#include "TBFlyBy.generated.h"

USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FTBFlyBy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	FTBProjectileId ProjectileId;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	FVector ProjectilePosition = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	FVector ProjectileVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	double Distance = 0.0;
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "FlyBy", meta = (ToolTip = "Maximum distance from the projectile that the actor could be while still triggering this event."))
	double MaximumDistance = 200.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	TObjectPtr<AActor> ProjectileOwner = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	ETBProjectileSize ProjectileSize = ETBProjectileSize::None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	ETBBulletCaliber BulletCaliber = ETBBulletCaliber::None;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FlyBy")
	uint8 bIsBullet : 1;

	FTBFlyBy()
		: bIsBullet(false)
	{
		MaximumDistance = TB::Configuration::FlyByTraceRadius;
	}

	FTBFlyBy(const FTBProjectileId& ProjectileId, const FVector& ProjectilePosition, const FVector& ProjectileVelocity, const double Distance, AActor* HitActor, AActor* ProjectileOwner, const ETBBulletCaliber BulletCaliber)
		: ProjectileId(ProjectileId)
		, ProjectilePosition(ProjectilePosition)
		, ProjectileVelocity(ProjectileVelocity)
		, Distance(Distance)
		, HitActor(HitActor)
		, ProjectileOwner(ProjectileOwner)
		, BulletCaliber(BulletCaliber)
		, bIsBullet(true)
	{
		MaximumDistance = TB::Configuration::FlyByTraceRadius;
	}

	FTBFlyBy(const FTBProjectileId& ProjectileId, const FVector& ProjectilePosition, const FVector& ProjectileVelocity, const double Distance, AActor* HitActor, AActor* ProjectileOwner, const ETBProjectileSize ProjectileSize)
		: ProjectileId(ProjectileId)
		, ProjectilePosition(ProjectilePosition)
		, ProjectileVelocity(ProjectileVelocity)
		, Distance(Distance)
		, HitActor(HitActor)
		, ProjectileOwner(ProjectileOwner)
		, ProjectileSize(ProjectileSize)
		, bIsBullet(false)
	{
		MaximumDistance = TB::Configuration::FlyByTraceRadius;
	}

	void NotifyActorOfFlyBy();
	
	FORCEINLINE bool IsValid() const
	{
		return ::IsValid(HitActor)
			&& ::IsValid(ProjectileOwner)
			&& ProjectileId.IsValid();
	}

	FORCEINLINE bool operator==(const FTBFlyBy& rhs) const
	{
		return ProjectileId == rhs.ProjectileId
			&& ProjectileSize == rhs.ProjectileSize
			&& BulletCaliber == rhs.BulletCaliber
			&& bIsBullet == rhs.bIsBullet;
	}

	FORCEINLINE bool operator!=(const FTBFlyBy& other) const
	{
		return !(*this == other);
	}
};
