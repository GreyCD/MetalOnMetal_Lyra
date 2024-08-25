// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "LyraOnMetalStatsComponent.generated.h"

class UOnMetalAttributeSet;
struct FOnAttributeChangeData;
class ULyraOnMetalStatsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStamina_AttributeChanged, ULyraOnMetalStatsComponent*, OnMetalStatsComponent, float,
	OldValue, float, NewValue);

class ULyraAbilitySystemComponent;


UCLASS()
class LYRAGAME_API ULyraOnMetalStatsComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULyraOnMetalStatsComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the OnMetalStatsComponent if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|OnMetalStats")
	static ULyraOnMetalStatsComponent* FindOnMetalStatsComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<ULyraOnMetalStatsComponent>() : nullptr); }

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "Lyra|OnMetalStats")
	void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Lyra|OnMetalStats")
	void UninitializeFromAbilitySystem();

	// Returns the current stamina value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|OnMetalStats")
	float GetStamina() const;

	// Returns the maximum stamina value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|OnMetalStats")
	float GetMaxStamina() const;

	//Returns the current stamina value in range 0-1.
	UFUNCTION(BlueprintCallable, Category = "Lyra|OnMetalStats")
	float GetStaminaNormalized() const;

public:
	// Delegate when Stamina changes, some information may be missing on the client
	UPROPERTY(BlueprintAssignable, Category = "Lyra|OnMetalStats")
	FStamina_AttributeChanged OnStaminaChanged;

	// Delegate when max Stamina changes
	UPROPERTY(BlueprintAssignable, Category = "Lyra|OnMetalStats")
	FStamina_AttributeChanged OnMaxStaminaChanged;

protected:
	virtual void OnUnregister() override;
	virtual void HandleStaminaChanged(const FOnAttributeChangeData& ChangeData);
	virtual void HandleMaxStaminaChanged(const FOnAttributeChangeData& ChangeData);

	// A reference to the ability system component that this component is using.
	UPROPERTY()
	ULyraAbilitySystemComponent* AbilitySystemComponent;

	// A reference to the AttributeSet that this component is using.
	UPROPERTY()
	const UOnMetalAttributeSet* OnMetalAttributeSet;
	
};
