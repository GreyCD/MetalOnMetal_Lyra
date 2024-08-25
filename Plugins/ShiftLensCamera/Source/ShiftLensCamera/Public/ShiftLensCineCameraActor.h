// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE

#pragma once

#include "CoreMinimal.h"
#include "CineCameraActor.h"
#include "ShiftLensCineCameraActor.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Common, hideCategories = (Input, Rendering, AutoPlayerActivation), showcategories = ("Input|MouseInput", "Input|TouchInput"), Blueprintable)
class SHIFTLENSCAMERA_API AShiftLensCineCameraActor : public ACineCameraActor
{
	GENERATED_BODY()

public:
	AShiftLensCineCameraActor(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/** Get shift lens value from shift lens camera component. */
	UFUNCTION(BlueprintCallable, Category = "Cine Camera")
		float GetShiftLens();

	/** Update shift lens value. */
	UFUNCTION(BlueprintCallable, Category = "Cine Camera")
		void UpdateShiftLens();

	/** Returns the CineCameraComponent of this CineCamera. */
	UFUNCTION(BlueprintCallable, Category = "Cine Camera")
		UShiftLensCineCameraComponent* GetShiftLensCineCameraComponent() const { return ShiftLensCineCameraComponent; }

private:
	/** Returns CineCameraComponent subobject. */
	class UShiftLensCineCameraComponent* ShiftLensCineCameraComponent;

};
