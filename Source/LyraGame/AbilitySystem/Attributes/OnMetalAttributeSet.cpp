// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/OnMetalAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

UOnMetalAttributeSet::UOnMetalAttributeSet()
: Stamina(100.0f)
, MaxStamina(100.0f)
{
	//bOutOfStamina = false;
	//MaxStaminaBeforeAttributeChange = 0.0f;
	//StaminaBeforeAttributeChange = 0.0f;
}

void UOnMetalAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UOnMetalAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UOnMetalAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
}

void UOnMetalAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOnMetalAttributeSet, Stamina, OldValue);

	/*//
	 *
	 *Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	// These events on the client should not be changing attributes

	const float CurrentStamina = GetStamina();
	const float EstimatedMagnitude = CurrentStamina - OldValue.GetCurrentValue();
	
	OnStaminaChanged.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);

	if (!bOutOfStamina && CurrentStamina <= 0.0f)
	{
		OnOutOfStamina.Broadcast(nullptr, nullptr, nullptr, EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentStamina);
	}

	bOutOfStamina = (CurrentStamina <= 0.0f);
	*/
	
}

void UOnMetalAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UOnMetalAttributeSet, MaxStamina, OldValue);

	// Call the change callback, but without an instigator
	// This could be changed to an explicit RPC in the future
	//OnMaxStaminaChanged.Broadcast(nullptr, nullptr, nullptr, GetMaxStamina() - OldValue.GetCurrentValue(), OldValue.GetCurrentValue(), GetMaxStamina());
}

/*bool UOnMetalAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Save the current stamina
	StaminaBeforeAttributeChange = GetStamina();
	MaxStaminaBeforeAttributeChange = GetMaxStamina();

	return true;
}*/

/*void UOnMetalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Clamp and fall into out of stamina handling below
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		// Notify on any requested max stamina changes
		OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxStaminaBeforeAttributeChange, GetMaxStamina());
	}

	// If stamina has actually changed, activate callbacks
	if (GetStamina() != StaminaBeforeAttributeChange)
	{
		OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
	}

	if ((GetStamina() <= 0.0f) && !bOutOfStamina)
	{
		OnOutOfStamina.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
	}

	// Check stamina again in case an event above changed it.
	bOutOfStamina = (GetStamina() <= 0.0f);
}*/

/*void UOnMetalAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	UAbilitySystemComponent* Source = EffectContext.GetOriginalInstigatorAbilitySystemComponent();
	AActor* Instigator = EffectContext.GetOriginalInstigator();
	AActor* Causer = EffectContext.GetEffectCauser();

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		// Clamp the stamina value between 0 and MaxStamina
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));

		// If stamina has actually changed, activate callbacks
		if (GetStamina() != StaminaBeforeAttributeChange)
		{
			OnStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
		}

		// Check if stamina has reached 0 and trigger the out-of-stamina event
		if ((GetStamina() <= 0.0f) && !bOutOfStamina)
		{
			OnOutOfStamina.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, StaminaBeforeAttributeChange, GetStamina());
			bOutOfStamina = true;
		}
		else if ((GetStamina() > 0.0f) && bOutOfStamina)
		{
			// Stamina has regenerated above 0, so reset the out-of-stamina flag
			bOutOfStamina = false;
		}
	}
	else if (Data.EvaluatedData.Attribute == GetMaxStaminaAttribute())
	{
		// Notify on any requested max stamina changes
		OnMaxStaminaChanged.Broadcast(Instigator, Causer, &Data.EffectSpec, Data.EvaluatedData.Magnitude, MaxStaminaBeforeAttributeChange, GetMaxStamina());

		// Ensure that the current stamina doesn't exceed the new max stamina value
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}*/

void UOnMetalAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UOnMetalAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

/*void UOnMetalAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxStaminaAttribute())
	{
		// Make sure current stamina is not greater than the new max stamina.
		if (GetStamina() > NewValue)
		{
			ULyraAbilitySystemComponent* LyraASC = GetLyraAbilitySystemComponent();
			check(LyraASC);

			LyraASC->ApplyModToAttribute(GetStaminaAttribute(), EGameplayModOp::Override, NewValue);
		}
	}

	if (bOutOfStamina && (GetStamina() > 0.0f))
	{
		bOutOfStamina = false;
	}
}*/

void UOnMetalAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetStaminaAttribute())
	{
		// Do not allow stamina to go negative or above max stamina.
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		// Do not allow max stamina to drop below 1.
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}


