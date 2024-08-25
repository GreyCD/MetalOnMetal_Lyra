// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTargetTypes.h"



#include "LyraGameplayAbilityTargetData_SingleTargetHit.generated.h"

class FArchive;
struct FGameplayEffectContextHandle;


/** Game-specific additions to SingleTargetHit tracking */
USTRUCT()
struct LYRAGAME_API FLyraGameplayAbilityTargetData_SingleTargetHit : public FGameplayAbilityTargetData_SingleTargetHit
{
	GENERATED_BODY()

	FLyraGameplayAbilityTargetData_SingleTargetHit()
		: CartridgeID(-1)
	{ }

	virtual void AddTargetDataToContext(FGameplayEffectContextHandle& Context, bool bIncludeActorArray) const override;

	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID;
	
	UPROPERTY()
	bool bHitConfirmed = false; // This is set to true when the hit is confirmed by the server

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FLyraGameplayAbilityTargetData_SingleTargetHit::StaticStruct();
	}
};

template<>
struct TStructOpsTypeTraits<FLyraGameplayAbilityTargetData_SingleTargetHit> : public TStructOpsTypeTraitsBase2<FLyraGameplayAbilityTargetData_SingleTargetHit>
{
	enum
	{
		WithNetSerializer = true	// For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
	};
};

