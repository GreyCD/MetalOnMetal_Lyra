// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "OnMetalAttributeSet.generated.h"

struct FGameplayEffectModCallbackData;

/**
 * 
 */
UCLASS(BlueprintType)
class LYRAGAME_API UOnMetalAttributeSet : public ULyraAttributeSet
{
	GENERATED_BODY()
	
public:
	UOnMetalAttributeSet();
	
	ATTRIBUTE_ACCESSORS(UOnMetalAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(UOnMetalAttributeSet, MaxStamina);

	// // Delegate when Stamina changes due to damage/healing, some information may be missing on the client
	mutable FLyraAttributeEvent OnStaminaChanged;
	//
	// // Delegate when max Stamina changes
	// mutable FLyraAttributeEvent OnMaxStaminaChanged;
	//
	// // Delegate to broadcast when the Stamina attribute reaches zero
	// mutable FLyraAttributeEvent OnOutOfStamina;

protected:

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue);

	//virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	//virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	//virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:
	// The current Stamina attribute.  The Stamina will be capped by the max Stamina attribute.  Stamina is hidden from modifiers so only executions can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Lyra|Stamina", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Stamina;

	// The current max Stamina attribute.  Max Stamina is an attribute since gameplay effects can modify it.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Lyra|Stamina", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxStamina;

	// Used to track when the Stamina reaches 0.
	//bool bOutOfStamina;

	// Store the Stamina before any changes 
	//float MaxStaminaBeforeAttributeChange;
	//float StaminaBeforeAttributeChange;
	
};
