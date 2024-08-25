// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/LyraOnMetalStatsComponent.h"

#include "LyraLogChannels.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/OnMetalAttributeSet.h"


ULyraOnMetalStatsComponent::ULyraOnMetalStatsComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
	AbilitySystemComponent = nullptr;
	OnMetalAttributeSet = nullptr;
	
}

void ULyraOnMetalStatsComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if(AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraOnMetalStatsComponent: OnMetalStats component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if(!AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraOnMetalStatsComponent: Cannot initialize OnMetalStats component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	OnMetalAttributeSet = AbilitySystemComponent->GetSet<UOnMetalAttributeSet>();
	if(!OnMetalAttributeSet)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraOnMetalStatsComponent: Cannot initialize OnMetalStats component for owner [%s] with NULL OnMetalAttributeSet."), *GetNameSafe(Owner));
		return;
	}

	//Register to listen to attribute changes
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OnMetalAttributeSet->GetStaminaAttribute()).AddUObject(this, &ULyraOnMetalStatsComponent::HandleStaminaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(OnMetalAttributeSet->GetMaxStaminaAttribute()).AddUObject(this, &ULyraOnMetalStatsComponent::HandleMaxStaminaChanged);
}

void ULyraOnMetalStatsComponent::UninitializeFromAbilitySystem()
{
	OnMetalAttributeSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float ULyraOnMetalStatsComponent::GetStamina() const
{
	return (OnMetalAttributeSet ? OnMetalAttributeSet->GetStamina() : 0.0f);
}

float ULyraOnMetalStatsComponent::GetMaxStamina() const
{
	return (OnMetalAttributeSet ? OnMetalAttributeSet->GetMaxStamina() : 0.0f);
}

float ULyraOnMetalStatsComponent::GetStaminaNormalized() const
{
	if(OnMetalAttributeSet)
	{
		const float Stamina = OnMetalAttributeSet->GetStamina();
		const float MaxStamina = OnMetalAttributeSet->GetMaxStamina();
		return ((MaxStamina > 0.0f) ? (Stamina / MaxStamina) : 0.0f);
	}
	return 0.0f;
}

void ULyraOnMetalStatsComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	Super::OnUnregister();
}

void ULyraOnMetalStatsComponent::HandleStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
	//UE_LOG (LogLyra, Warning, TEXT("Stamina Changed"));
}

void ULyraOnMetalStatsComponent::HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxStaminaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}


