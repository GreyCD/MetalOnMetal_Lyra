// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE
 

#include "ShiftLensCineCameraComponent.h"

#include "Engine.h"

//
UShiftLensCineCameraComponent::UShiftLensCineCameraComponent()
	: Super()
{
}

/*
 * Add consider for perspective collection.
 * Try expressing like optical movement shift lens, to change a value of projection matrix[2][1].
 * Fortunately UE4 already have sources to do it.We just set a value of FMinimalViewInfo::OffCenterProjectionOffset.Y.
 */
void UShiftLensCineCameraComponent::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	Super::GetCameraView(DeltaTime, DesiredView);

	// Update shift lens
	DesiredView.OffCenterProjectionOffset.Y = this->ShiftLens;
	UpdateShiftLens();

	// Keep Horizontaltal
	if (KeepHorizontal) {
		DesiredView.Rotation.Pitch = 0.0f;
	}
}

/** Set shift lens value. */
void UShiftLensCineCameraComponent::SetShiftLens(float InShiftLens)
{
	ShiftLens = InShiftLens;
}

/** Update shift lens value. */
void UShiftLensCineCameraComponent::UpdateShiftLens()
{
	// For editor.
	AActor* Owner = GetOwner();
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	AController* OwningController = OwningPawn ? OwningPawn->GetController() : nullptr;
	if (OwningController && OwningController->IsLocalPlayerController())
	{
		APlayerController* PlayerController = Cast<APlayerController>(OwningController);
		FLocalPlayerContext Context(PlayerController);
		ULocalPlayer* LocalPlayer = Context.GetLocalPlayer();
		if (LocalPlayer)
		{
			//			UShiftLensLocalPlayer* ShiftLensLocalPlayer = Cast<UShiftLensLocalPlayer>(LocalPlayer);
			//			if (ShiftLensLocalPlayer)
			//			{
			//				ShiftLensLocalPlayer->SetShiftLens(this->ShiftLens);
			//			}
		}
	}

	// For playing.
	UWorld* World = Owner->GetWorld();
	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (PlayerController && PlayerController->PlayerCameraManager)
		{
			//AActor* ViewTarget = PlayerController->GetViewTarget();
			//if (ViewTarget == this)
			//{
			ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
			if (LocalPlayer)
			{
				//				UShiftLensLocalPlayer* ShiftLensLocalPlayer = Cast<UShiftLensLocalPlayer>(LocalPlayer);
				//				if (ShiftLensLocalPlayer)
				//				{
				//					ShiftLensLocalPlayer->SetShiftLens(this->ShiftLens);
				//				}
			}
			//}
		}
	}
}
