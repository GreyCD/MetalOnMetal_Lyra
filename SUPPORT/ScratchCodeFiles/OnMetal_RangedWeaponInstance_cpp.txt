// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/OnMetal_RangedWeaponInstance.h"

UOnMetal_RangedWeaponInstance::UOnMetal_RangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

bool UOnMetal_RangedWeaponInstance::IsProjectileWeapon() const
{
	// This could be based on a property set in the weapon's data asset,
	// or determined by the presence of certain components or abilities
	return bIsProjectileWeapon; // Assuming you have such a boolean property
}
