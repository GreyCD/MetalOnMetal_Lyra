// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "LyraLogChannels.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCharacterMovementComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_MovementStopped, "Gameplay.MovementStopped");

namespace LyraCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("LyraCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};

ULyraCharacterMovementComponent::ULyraCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Safe_bWantsToSprint = false;
	bWantsToSprint = false;
	MaxWalkSpeed = 400.0f;
	DefaultMaxWalkSpeed = 400.0f;
	MaxWalkSpeedCrouched = 200.0f;
}

void ULyraCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

bool ULyraCharacterMovementComponent::CanAttemptJump() const
{
	// Same as UCharacterMovementComponent's implementation but without the crouch check
	return IsJumpAllowed() &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}


FSavedMovePtr ULyraCharacterMovementComponent::FNetworkPredictionData_Client_Lyra::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Lyra);
}

void ULyraCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToSprint = (Flags&FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* ULyraCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		ULyraCharacterMovementComponent* MutableThis = const_cast<ULyraCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Lyra(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f; // 140 is the default value in UCharacterMovementComponent
	}
	return ClientPredictionData;
}

void ULyraCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	//UKismetSystemLibrary::PrintString(GetWorld(), TEXT("OnMovementUpdated"));


		// if(Safe_bWantsToSprint)
	 // 	{
		// 	UE_LOG(LogLyra, Error, TEXT("BlahBlah Blah Sprint true"));
	 // 		//MaxWalkSpeed = SprintSpeed;
	 // 	}
	 // 	else
	 // 	{
		// 	//MaxWalkSpeed = DefaultMaxWalkSpeed;
 	// }
 }



void ULyraCharacterMovementComponent::SetSprintingState(bool bNewWantsToSprint)
{
	// if (bNewWantsToSprint != Safe_bWantsToSprint)
	// {
	// 	Safe_bWantsToSprint = bNewWantsToSprint;
	//
	// 	if (MovementMode == MOVE_Walking)
	// 	{
	// 		MaxWalkSpeed = bNewWantsToSprint ? SprintSpeed : DefaultMaxWalkSpeed;
	// 	}
	// }

	// Safe_bWantsToSprint = bNewWantsToSprint;
	//
	// if (Safe_bWantsToSprint == true)
	// 	{
	// 		MaxWalkSpeed = SprintSpeed;
	// 	}
	// else if (Safe_bWantsToSprint == false)
	//  	{
	//  		MaxWalkSpeed = DefaultMaxWalkSpeed;
	//  	}

	 if (bNewWantsToSprint != Safe_bWantsToSprint)
		{
			Safe_bWantsToSprint = bNewWantsToSprint;
	 		MaxWalkSpeed = bNewWantsToSprint ? SprintSpeed : DefaultMaxWalkSpeed;
	 	}
}




void ULyraCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}


const FLyraCharacterGroundInfo& ULyraCharacterMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - LyraCharacter::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = LyraCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}

void ULyraCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{
	bHasReplicatedAcceleration = true;
	Acceleration = InAcceleration;
}

FRotator ULyraCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Gameplay_MovementStopped))
		{
			return FRotator(0,0,0);
		}
	}

	return Super::GetDeltaRotation(DeltaTime);
}

float ULyraCharacterMovementComponent::GetMaxSpeed() const
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		if (ASC->HasMatchingGameplayTag(TAG_Gameplay_MovementStopped))
		{
			return 0;
		}
	}

	return Super::GetMaxSpeed();
}

//Sprint Logic Functions
void ULyraCharacterMovementComponent::FSavedMove_Lyra::Clear()
{
	FSavedMove_Character::Clear();
	bSavedWantsToSprint = 0;
}

uint8 ULyraCharacterMovementComponent::FSavedMove_Lyra::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

bool ULyraCharacterMovementComponent::FSavedMove_Lyra::CanCombineWith(const FSavedMovePtr& NewMove,
	ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_Lyra* NewMove_Lyra = static_cast<FSavedMove_Lyra*>(NewMove.Get());
	
	if (bSavedWantsToSprint != NewMove_Lyra->bSavedWantsToSprint)
	{
		return false;
	}
	return FSavedMove_Character::CanCombineWith(NewMove, Character, MaxDelta);
}

void ULyraCharacterMovementComponent::FSavedMove_Lyra::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const ULyraCharacterMovementComponent* CustomMovementComponent = Cast<ULyraCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bSavedWantsToSprint = CustomMovementComponent->bWantsToSprint;
	}
}

void ULyraCharacterMovementComponent::FSavedMove_Lyra::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);
	if (ULyraCharacterMovementComponent* CustomMovementComponent = Cast<ULyraCharacterMovementComponent>(Character->GetCharacterMovement()))
	{
		CustomMovementComponent->bWantsToSprint = bSavedWantsToSprint;
	}
}










