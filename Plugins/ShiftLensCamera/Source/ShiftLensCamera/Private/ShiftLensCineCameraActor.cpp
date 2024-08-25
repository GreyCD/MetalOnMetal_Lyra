// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE

#include "ShiftLensCineCameraActor.h"

#include "ShiftLensCineCameraComponent.h"
#include "Engine.h"

//
AShiftLensCineCameraActor::AShiftLensCineCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UShiftLensCineCameraComponent>(TEXT("CameraComponent"))
	)
{
	ShiftLensCineCameraComponent = Cast<UShiftLensCineCameraComponent>(GetCineCameraComponent());

	SetActorTickEnabled(true);
	//this->ShiftLens = 0.0f;
}

//
void AShiftLensCineCameraActor::Tick(float DeltaTime)
{
	UpdateShiftLens();
	Super::Tick(DeltaTime);
}

/** Get shift lens value from shift lens camera component. */
float AShiftLensCineCameraActor::GetShiftLens()
{
	return this->ShiftLensCineCameraComponent->ShiftLens;
}

/** Update shift lens value. */
void AShiftLensCineCameraActor::UpdateShiftLens()
{
	this->ShiftLensCineCameraComponent->UpdateShiftLens();
}