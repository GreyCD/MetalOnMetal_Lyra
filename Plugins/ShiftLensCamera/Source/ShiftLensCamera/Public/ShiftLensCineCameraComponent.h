// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE

#pragma once

#include "CoreMinimal.h"
#include "CineCameraComponent.h"
#include "ShiftLensCineCameraComponent.generated.h"

/**
 * 
 */
UCLASS(HideCategories = (Mobility, Rendering, LOD), Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class SHIFTLENSCAMERA_API UShiftLensCineCameraComponent : public UCineCameraComponent
{
	GENERATED_BODY()

public:
	/* Add consider for perspective collection.
	 * Try expressing like optical movement shift lens, to change a value of projection matrix[2][1].
	 * Fortunately UE4 already have sources to do it.We just set a value of FMinimalViewInfo::OffCenterProjectionOffset.Y.
	 */
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	UShiftLensCineCameraComponent();

	/* Shift lens value.
	 * You should set range at minimum -1.0, and at maximum 1.0.
	 * It doesn't matter over 1.0. But rendering result is not realistic camera.
	 */
	UPROPERTY(Interp, BlueprintSetter = SetShiftLens, EditAnywhere, BlueprintReadWrite, Category = "Current Camera Settings")
		float ShiftLens;
	UFUNCTION(BlueprintCallable, Category = "Current Camera Settings")
		virtual void SetShiftLens(float InShiftLens);

	/** If it is true, camera keep horizontal. */
	UPROPERTY(Interp, EditAnywhere, BlueprintReadWrite, Category = "Current Camera Settings")
		uint8 KeepHorizontal : 1;

	/** Update shift lens value. */
	UFUNCTION(BlueprintCallable, Category = "Current Camera Settings")
		void UpdateShiftLens();

//private:
	//float LocationZ;
};
