// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "NativeGameplayTags.h"

#include "LyraCharacterMovementComponent.generated.h"

class UObject;
struct FFrame;

LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);


/**
 * FLyraCharacterGroundInfo
 *
 *	Information about the ground under the character.  It only gets updated as needed.
 */
USTRUCT(BlueprintType)
struct FLyraCharacterGroundInfo
{
	GENERATED_BODY()

	FLyraCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

/**
 * ULyraCharacterMovementComponent
 *
 *	The base character movement component class used by this project.
 */
UCLASS(Config = Game)
class LYRAGAME_API ULyraCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	ULyraCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void SimulateMovement(float DeltaTime) override;

	virtual bool CanAttemptJump() const override;

	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Lyra|CharacterMovement")
	const FLyraCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent interface

	/**
	 *This is the start of the sprint implementation
	 */
	
	//Sprint Variables
	uint8 bWantsToSprint : 1;
	bool Safe_bWantsToSprint;
	
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting")
	float SprintSpeed {275.0f};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting")
	float SuperSprintSpeed {300.0f};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting")
	float MoveDirectionTolerance {0.5f};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting")
	bool bCanOnlySprintForwards {false};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting", meta = (EditCondition = "!bCanOnlySprintForwards", EditConditionHides))
	bool bSprintSideways {true};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Sprinting", meta = (EditCondition = "!bCanOnlySprintForwards && bSprintSideways", EditConditionHides))
	float SprintSidewaysSpeed {250.0f};

	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Walking")
	float MaxWalkSpeedMultiplier {1.0f};
	UPROPERTY(EditAnywhere, Category = "OnMetalMovementComponent|Walking")
	float MinWalkSpeedMultiplier {0.25f};
	
	float DefaultMaxWalkSpeed {1.0f};

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

	UFUNCTION(BlueprintCallable, Category = "Lyra|CharacterMovement")
	void SetSprintingState(bool bNewWantsToSprint);
	
	class FSavedMove_Lyra : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
		virtual void PrepMoveFor(ACharacter* Character) override;



		uint8 bSavedWantsToSprint : 1;
		uint8 bSavedWantsToSuperSprint : 1;
		float SavedRequestedWalkSpeedMultiplier {1.0f};
	};
	
	class FNetworkPredictionData_Client_Lyra : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Lyra(const UCharacterMovementComponent& ClientMovement):Super(ClientMovement){}
		typedef FNetworkPredictionData_Client_Character Super;
		virtual FSavedMovePtr AllocateNewMove() override;
	};
	
	/**
 *This is the end of the sprint implementation
 */

protected:

	virtual void InitializeComponent() override;

protected:

	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FLyraCharacterGroundInfo CachedGroundInfo;

	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;
};

