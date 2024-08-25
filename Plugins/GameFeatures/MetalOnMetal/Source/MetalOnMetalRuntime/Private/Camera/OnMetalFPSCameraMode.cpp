// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/OnMetalFPSCameraMode.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"

UOnMetalFPSCameraMode::UOnMetalFPSCameraMode()
{
	FieldOfView = 90.0f;
	TargetFOV = 90.0f;
	TargetFOVCurve = nullptr;
	FOVBlendTime = 0.5f;
	FOVBlendWeight = 1.0f;
	FOVBlendStartTime = 0.0f;
}

void UOnMetalFPSCameraMode::UpdateView(float DeltaTime)
{
	//Super::UpdateView(DeltaTime);
	APawn* TargetPawn = Cast<APawn>(GetTargetActor());
	if (TargetPawn)
	{
		USkeletalMeshComponent* MeshComponent = TargetPawn->FindComponentByClass<USkeletalMeshComponent>();
		if (MeshComponent)
		{
			FVector SocketLocation = MeshComponent->GetSocketLocation(TEXT("S_Camera"));
			FRotator ControlRotation = TargetPawn->GetControlRotation();

			View.Location = SocketLocation;
			View.Rotation = ControlRotation;
			View.ControlRotation = ControlRotation;
            
			UpdateFOVBlend(DeltaTime);
			View.FieldOfView = FMath::Lerp(FieldOfView, TargetFOV, FOVBlendWeight);
		}
	}
}

void UOnMetalFPSCameraMode::SetTargetFOV(float NewTargetFOV)
{
	TargetFOV = NewTargetFOV;
	FOVBlendWeight = 0.0f;
	FOVBlendStartTime = GetWorld()->GetTimeSeconds();
}



FVector UOnMetalFPSCameraMode::GetPivotLocation() const
{
	APawn* TargetPawn = Cast<APawn>(GetTargetActor());
	if (TargetPawn)
	{
		USkeletalMeshComponent* MeshComponent = TargetPawn->FindComponentByClass<USkeletalMeshComponent>();
		if (MeshComponent)
		{
			return MeshComponent->GetSocketLocation(TEXT("S_Camera"));
		}
	}

	return Super::GetPivotLocation();
}

void UOnMetalFPSCameraMode::UpdateFOVBlend(float DeltaTime)
{
	if (FOVBlendWeight < 1.0f)
	{
		float FOVBlendDuration = FMath::Max(FOVBlendTime, KINDA_SMALL_NUMBER);
		float FOVBlendAlpha = (GetWorld()->GetTimeSeconds() - FOVBlendStartTime) / FOVBlendDuration;
		FOVBlendWeight = FMath::Clamp(FOVBlendAlpha, 0.0f, 1.0f);

		if (TargetFOVCurve)
		{
			FOVBlendWeight = TargetFOVCurve->GetFloatValue(FOVBlendWeight);
		}
	}
}
