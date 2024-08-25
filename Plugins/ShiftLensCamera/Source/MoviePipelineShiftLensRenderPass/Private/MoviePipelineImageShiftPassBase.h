// Created by christinayan01 by Takahiro Yanai. 2023.5.15
#pragma once

#include "MoviePipelineImagePassBase.h" // yanai add
#include "SceneView.h"
#include "MoviePipelineImageShiftPassBase.generated.h"

class FSceneViewFamily;
class FSceneView;

UCLASS(BlueprintType, Abstract)
class UMoviePipelineImageShiftPassBase : public UMoviePipelineImagePassBase
{
	GENERATED_BODY()

public:
	UMoviePipelineImageShiftPassBase();

protected:
	void ModifyProjectionMatrixForTilingShift(const FMoviePipelineRenderPassMetrics& InSampleState, const bool bInOrthographic, FMatrix& InOutProjectionMatrix, float& OutDoFSensorScale, float& ShiftLensValue) const;

protected:
	virtual TSharedPtr<FSceneViewFamilyContext> CalculateViewFamily(FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload = nullptr);
	virtual FSceneView* GetSceneViewForSampleState(FSceneViewFamily* ViewFamily, FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload = nullptr);

public:

protected:

};
