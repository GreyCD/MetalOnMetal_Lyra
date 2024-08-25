// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/LyraCameraMode.h"
#include "Curves/CurveFloat.h"
#include "OnMetalFPSCameraMode.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class METALONMETALRUNTIME_API UOnMetalFPSCameraMode : public ULyraCameraMode
{
	GENERATED_BODY()
	
public:
	UOnMetalFPSCameraMode();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Person Camera")
	//float FieldOfView;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Person Camera")
	float TargetFOV;

	UPROPERTY(EditDefaultsOnly, Category = "First Person Camera")
	UCurveFloat* TargetFOVCurve;

	UPROPERTY(EditDefaultsOnly, Category = "First Person Camera")
	float FOVBlendTime;

	UFUNCTION(BlueprintCallable, Category = "First Person Camera")
	void SetTargetFOV(float NewTargetFOV);

protected:
	virtual void UpdateView(float DeltaTime) override;
	virtual FVector GetPivotLocation() const override;

private:
	void UpdateFOVBlend(float DeltaTime);

	float FOVBlendWeight;
	float FOVBlendStartTime;
	
};
