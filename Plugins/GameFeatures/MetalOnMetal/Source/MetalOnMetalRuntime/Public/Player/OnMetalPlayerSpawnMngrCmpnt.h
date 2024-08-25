// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/LyraPlayerSpawningManagerComponent.h"
#include "OnMetalPlayerSpawnMngrCmpnt.generated.h"

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API UOnMetalPlayerSpawnMngrCmpnt : public ULyraPlayerSpawningManagerComponent
{
	GENERATED_BODY()

protected:
	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts) override;	
	
};
