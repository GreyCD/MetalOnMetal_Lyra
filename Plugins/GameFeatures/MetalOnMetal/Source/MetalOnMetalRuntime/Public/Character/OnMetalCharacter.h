// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"
#include "Camera/LyraCameraComponent.h"
#include "OnMetalCharacter.generated.h"

/**
 * 
 */
UCLASS()
class METALONMETALRUNTIME_API AOnMetalCharacter : public ALyraCharacter
{
	GENERATED_BODY()
	
public:
	AOnMetalCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	// TObjectPtr<ULyraCameraComponent> FPS_Camera;

//virtual void BeginPlay() override;
	
};
