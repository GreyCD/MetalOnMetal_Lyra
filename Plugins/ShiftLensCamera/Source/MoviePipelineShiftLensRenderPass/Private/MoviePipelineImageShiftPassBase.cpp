// Created by christinayan01 by Takahiro Yanai. 2023.9.8

// NOTE: New version released Unreal Engine, 
// it is necessary to sync with MoviePipeLineImagePassBase.cpp.

#include "MoviePipelineImageShiftPassBase.h"

// For Cine Camera Variables in Metadata
#include "MoviePipelineViewFamilySetting.h"
#include "LegacyScreenPercentageDriver.h"
#include "MoviePipelineGameOverrideSetting.h"
#include "Engine/LocalPlayer.h"

// For Cine Camera Variables in Metadata
//#include "CineCameraActor.h"
#include "CineCameraComponent.h"

// For shift lens
#include "ShiftLensCineCameraActor.h" // yanai add

UMoviePipelineImageShiftPassBase::UMoviePipelineImageShiftPassBase()
	: UMoviePipelineImagePassBase()
{
	PassIdentifier = FMoviePipelinePassIdentifier("ImagePassBase");
}

TSharedPtr<FSceneViewFamilyContext> UMoviePipelineImageShiftPassBase::CalculateViewFamily(FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload)
{
	const FMoviePipelineFrameOutputState::FTimeData& TimeData = InOutSampleState.OutputState.TimeData;

	FEngineShowFlags ShowFlags = FEngineShowFlags(EShowFlagInitMode::ESFIM_Game);
	EViewModeIndex  ViewModeIndex;
	GetViewShowFlags(ShowFlags, ViewModeIndex);
	MoviePipelineRenderShowFlagOverride(ShowFlags);
	TWeakObjectPtr<UTextureRenderTarget2D> ViewRenderTarget = GetOrCreateViewRenderTarget(InOutSampleState.BackbufferSize, OptPayload);
	check(ViewRenderTarget.IsValid());

	FRenderTarget* RenderTarget = ViewRenderTarget->GameThread_GetRenderTargetResource();

	TSharedPtr<FSceneViewFamilyContext> OutViewFamily = MakeShared<FSceneViewFamilyContext>(FSceneViewFamily::ConstructionValues(
		RenderTarget,
		GetPipeline()->GetWorld()->Scene,
		ShowFlags)
		.SetTime(FGameTime::CreateUndilated(TimeData.WorldSeconds, TimeData.FrameDeltaTime))
		.SetRealtimeUpdate(true));

	OutViewFamily->SceneCaptureSource = InOutSampleState.SceneCaptureSource;
	OutViewFamily->bWorldIsPaused = InOutSampleState.bWorldIsPaused;
	OutViewFamily->ViewMode = ViewModeIndex;
	OutViewFamily->bOverrideVirtualTextureThrottle = true;

	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	OutViewFamily->OverrideFrameCounter = UE::MovieRenderPipeline::GetRendererFrameCount();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
	
	// Kept as an if/else statement to avoid the confusion with setting all of these values to some permutation of !/!!bHasRenderedFirstViewThisFrame.
	if (!GetPipeline()->bHasRenderedFirstViewThisFrame)
	{
		GetPipeline()->bHasRenderedFirstViewThisFrame = true;
		
		OutViewFamily->bIsFirstViewInMultipleViewFamily = true;
		OutViewFamily->bIsMultipleViewFamily = true;
		OutViewFamily->bAdditionalViewFamily = false;
	}
	else
	{
		OutViewFamily->bIsFirstViewInMultipleViewFamily = false;
		OutViewFamily->bAdditionalViewFamily = true;
		OutViewFamily->bIsMultipleViewFamily = true;
	}

	const bool bIsPerspective = true;
	ApplyViewMode(OutViewFamily->ViewMode, bIsPerspective, OutViewFamily->EngineShowFlags);

	EngineShowFlagOverride(ESFIM_Game, OutViewFamily->ViewMode, OutViewFamily->EngineShowFlags, false);
	
	const UMoviePipelineExecutorShot* Shot = GetPipeline()->GetActiveShotList()[InOutSampleState.OutputState.ShotIndex];
	
	for (UMoviePipelineGameOverrideSetting* OverrideSetting : GetPipeline()->FindSettingsForShot<UMoviePipelineGameOverrideSetting>(Shot))
	{
		if (OverrideSetting->bOverrideVirtualTextureFeedbackFactor)
		{
			OutViewFamily->VirtualTextureFeedbackFactor = OverrideSetting->VirtualTextureFeedbackFactor;
		}
	}

	// No need to do anything if screen percentage is not supported. 
	if (IsScreenPercentageSupported())
	{
		// Allows all Output Settings to have an access to View Family. This allows to modify rendering output settings.
		for (UMoviePipelineViewFamilySetting* Setting : GetPipeline()->FindSettingsForShot<UMoviePipelineViewFamilySetting>(Shot))
		{
			Setting->SetupViewFamily(*OutViewFamily);
		}
	}

	// If UMoviePipelineViewFamilySetting never set a Screen percentage interface we fallback to default.
	if (OutViewFamily->GetScreenPercentageInterface() == nullptr)
	{
		OutViewFamily->SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(*OutViewFamily, IsScreenPercentageSupported() ? InOutSampleState.GlobalScreenPercentageFraction : 1.f));
	}


	// View is added as a child of the OutViewFamily-> 
	FSceneView* View = GetSceneViewForSampleState(OutViewFamily.Get(), /*InOut*/ InOutSampleState, OptPayload);
	
	SetupViewForViewModeOverride(View);

	// Override the view's FrameIndex to be based on our progress through the sequence. This greatly increases
	// determinism with things like TAA.
	View->OverrideFrameIndexValue = InOutSampleState.FrameIndex;
	View->bCameraCut = InOutSampleState.bCameraCut;
	View->bIsOfflineRender = true;
	View->AntiAliasingMethod = IsAntiAliasingSupported() ? InOutSampleState.AntiAliasingMethod : EAntiAliasingMethod::AAM_None;

	// Override the Motion Blur settings since these are controlled by the movie pipeline.
	{
		FFrameRate OutputFrameRate = GetPipeline()->GetPipelinePrimaryConfig()->GetEffectiveFrameRate(GetPipeline()->GetTargetSequence());

		// We need to inversly scale the target FPS by time dilation to counteract slowmo. If scaling isn't applied then motion blur length
		// stays the same length despite the smaller delta time and the blur ends up too long.
		View->FinalPostProcessSettings.MotionBlurTargetFPS = FMath::RoundToInt(OutputFrameRate.AsDecimal() / FMath::Max(SMALL_NUMBER, InOutSampleState.OutputState.TimeData.TimeDilation));
		View->FinalPostProcessSettings.MotionBlurAmount = InOutSampleState.OutputState.TimeData.MotionBlurFraction;
		View->FinalPostProcessSettings.MotionBlurMax = 100.f;
		View->FinalPostProcessSettings.bOverride_MotionBlurAmount = true;
		View->FinalPostProcessSettings.bOverride_MotionBlurTargetFPS = true;
		View->FinalPostProcessSettings.bOverride_MotionBlurMax = true;

		// Skip the whole pass if they don't want motion blur.
		if (FMath::IsNearlyZero(InOutSampleState.OutputState.TimeData.MotionBlurFraction))
		{
			OutViewFamily->EngineShowFlags.SetMotionBlur(false);
		}
	}

	// Locked Exposure
	const bool bAutoExposureAllowed = IsAutoExposureAllowed(InOutSampleState);
	{
		// If the rendering pass doesn't allow autoexposure and they dont' have manual exposure set up, warn.
		if (!bAutoExposureAllowed && (View->FinalPostProcessSettings.AutoExposureMethod != EAutoExposureMethod::AEM_Manual))
		{
			// Skip warning if the project setting is disabled though, as exposure will be forced off in the renderer anyways.
			const URendererSettings* RenderSettings = GetDefault<URendererSettings>();
			if (RenderSettings->bDefaultFeatureAutoExposure != false)
			{
				UE_LOG(LogMovieRenderPipeline, Warning, TEXT("Camera Auto Exposure Method not supported by one or more render passes. Change the Auto Exposure Method to Manual!"));
				View->FinalPostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
			}
		}
	}

	// Orthographic cameras don't support anti-aliasing outside the path tracer (other than FXAA)
	const bool bIsOrthographicCamera = !View->IsPerspectiveProjection();
	if (bIsOrthographicCamera)
	{
		bool bIsSupportedAAMethod = View->AntiAliasingMethod == EAntiAliasingMethod::AAM_FXAA;
		bool bIsPathTracer = OutViewFamily->EngineShowFlags.PathTracing;
		bool bWarnJitters = InOutSampleState.ProjectionMatrixJitterAmount.SquaredLength() > SMALL_NUMBER;
		if ((!bIsPathTracer && !bIsSupportedAAMethod) || bWarnJitters)
		{
			UE_LOG(LogMovieRenderPipeline, Warning, TEXT("Orthographic Cameras are only supported with PathTracer or Deferred with FXAA Anti-Aliasing"));
		}
	}

	OutViewFamily->ViewExtensions.Append(GEngine->ViewExtensions->GatherActiveExtensions(FSceneViewExtensionContext(GetWorld()->Scene)));

	AddViewExtensions(*OutViewFamily, InOutSampleState);

	for (auto ViewExt : OutViewFamily->ViewExtensions)
	{
		ViewExt->SetupViewFamily(*OutViewFamily.Get());
	}

	for (int ViewExt = 0; ViewExt < OutViewFamily->ViewExtensions.Num(); ViewExt++)
	{
		OutViewFamily->ViewExtensions[ViewExt]->SetupView(*OutViewFamily.Get(), *View);
	}

	// The requested configuration may not be supported, warn user and fall back. We can't call
	// FSceneView::SetupAntiAliasingMethod because it reads the value from the cvar which would
	// cause the value set by the MoviePipeline UI to be ignored.
	{
		bool bMethodWasUnsupported = false;
		if (View->AntiAliasingMethod == AAM_TemporalAA && !SupportsGen4TAA(View->GetShaderPlatform()))
		{
			UE_LOG(LogMovieRenderPipeline, Error, TEXT("TAA was requested but this hardware does not support it."));
			bMethodWasUnsupported = true;
		}
		else if (View->AntiAliasingMethod == AAM_TSR && !SupportsTSR(View->GetShaderPlatform()))
		{
			UE_LOG(LogMovieRenderPipeline, Error, TEXT("TSR was requested but this hardware does not support it."));
			bMethodWasUnsupported = true;
		}

		if (bMethodWasUnsupported)
		{
			View->AntiAliasingMethod = AAM_None;
		}
	}

	// Anti Aliasing
	{
		// If we're not using Temporal Anti-Aliasing or Path Tracing we will apply the View Matrix projection jitter. Normally TAA sets this
		// inside FSceneRenderer::PreVisibilityFrameSetup. Path Tracing does its own anti-aliasing internally.
		bool bApplyProjectionJitter = !bIsOrthographicCamera
									&& !OutViewFamily->EngineShowFlags.PathTracing 
									&& !IsTemporalAccumulationBasedMethod(View->AntiAliasingMethod);
		if (bApplyProjectionJitter)
		{
			View->ViewMatrices.HackAddTemporalAAProjectionJitter(InOutSampleState.ProjectionMatrixJitterAmount);
		}
	}

	// Path Tracer Sampling
	if (OutViewFamily->EngineShowFlags.PathTracing)
	{
		// override whatever settings came from PostProcessVolume or Camera

		// If motion blur is enabled:
		//    blend all spatial samples together while leaving the handling of temporal samples up to MRQ
		//    each temporal sample will include denoising and post-process effects
		// If motion blur is NOT enabled:
		//    blend all temporal+spatial samples within the path tracer and only apply denoising on the last temporal sample
		//    this way we minimize denoising cost and also allow a much higher number of temporal samples to be used which
		//    can help reduce strobing

		// NOTE: Tiling is not compatible with the reference motion blur mode because it changes the order of the loops over the image.
		const bool bAccumulateSpatialSamplesOnly = OutViewFamily->EngineShowFlags.MotionBlur || InOutSampleState.GetTileCount() > 1;

		const int32 SampleCount = bAccumulateSpatialSamplesOnly ? InOutSampleState.SpatialSampleCount : InOutSampleState.TemporalSampleCount * InOutSampleState.SpatialSampleCount;
		const int32 SampleIndex = bAccumulateSpatialSamplesOnly ? InOutSampleState.SpatialSampleIndex : InOutSampleState.TemporalSampleIndex * InOutSampleState.SpatialSampleCount + InOutSampleState.SpatialSampleIndex;

		// TODO: pass along FrameIndex (which includes SampleIndex) to make sure sampling is fully deterministic

		// Overwrite whatever sampling count came from the PostProcessVolume
		View->FinalPostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
		View->FinalPostProcessSettings.PathTracingSamplesPerPixel = SampleCount;

		// reset path tracer's accumulation at the start of each sample
		View->bForcePathTracerReset = SampleIndex == 0;

		// discard the result, unless its the last sample
		InOutSampleState.bDiscardResult |= !(SampleIndex == SampleCount - 1);
	}

	// Object Occlusion/Histories
	{
		// If we're using tiling, we force the reset of histories each frame so that we don't use the previous tile's
		// object occlusion queries, as that causes things to disappear from some views.
		if (InOutSampleState.GetTileCount() > 1)
		{
			View->bForceCameraVisibilityReset = true;
		}
	}

	// Bias all mip-mapping to pretend to be working at our target resolution and not our tile resolution
	// so that the images don't end up soft.
	{
		float EffectivePrimaryResolutionFraction = 1.f / InOutSampleState.TileCounts.X;
		View->MaterialTextureMipBias = FMath::Log2(EffectivePrimaryResolutionFraction);

		// Add an additional bias per user settings. This allows them to choose to make the textures sharper if it
		// looks better with their particular settings.
		View->MaterialTextureMipBias += InOutSampleState.TextureSharpnessBias;
	}

	return OutViewFamily;
}

FSceneView* UMoviePipelineImageShiftPassBase::GetSceneViewForSampleState(FSceneViewFamily* ViewFamily, FMoviePipelineRenderPassMetrics& InOutSampleState, IViewCalcPayload* OptPayload)
{
	APlayerController* LocalPlayerController = GetPipeline()->GetWorld()->GetFirstPlayerController();

	int32 TileSizeX = InOutSampleState.BackbufferSize.X;
	int32 TileSizeY = InOutSampleState.BackbufferSize.Y;

	UE::MoviePipeline::FImagePassCameraViewData CameraInfo = GetCameraInfo(InOutSampleState, OptPayload);

	const float DestAspectRatio = InOutSampleState.BackbufferSize.X / (float)InOutSampleState.BackbufferSize.Y;
	const float CameraAspectRatio = bAllowCameraAspectRatio ? CameraInfo.ViewInfo.AspectRatio : DestAspectRatio;

	FSceneViewInitOptions ViewInitOptions;
	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.ViewOrigin = CameraInfo.ViewInfo.Location;
	ViewInitOptions.SetViewRectangle(FIntRect(FIntPoint(0, 0), FIntPoint(TileSizeX, TileSizeY)));
	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(CameraInfo.ViewInfo.Rotation);
	ViewInitOptions.ViewActor = CameraInfo.ViewActor;

	// Rotate the view 90 degrees (reason: unknown)
	ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	float ViewFOV = CameraInfo.ViewInfo.FOV;

	// Inflate our FOV to support the overscan 
	ViewFOV = 2.0f * FMath::RadiansToDegrees(FMath::Atan((1.0f + InOutSampleState.OverscanPercentage) * FMath::Tan(FMath::DegreesToRadians( ViewFOV * 0.5f ))));

	float DofSensorScale = 1.0f;

	// Calculate a Projection Matrix. This code unfortunately ends up similar to, but not quite the same as FMinimalViewInfo::CalculateProjectionMatrixGivenView
	FMatrix BaseProjMatrix;

	if (CameraInfo.bUseCustomProjectionMatrix)
	{
		BaseProjMatrix = CameraInfo.CustomProjectionMatrix;

		// Modify the custom matrix to do an off center projection, with overlap for high-res tiling
		const bool bOrthographic = false;
		ModifyProjectionMatrixForTiling(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale);
	}
	else
	{
		if (CameraInfo.ViewInfo.ProjectionMode == ECameraProjectionMode::Orthographic)
		{
			const float YScale = 1.0f / CameraAspectRatio;
			const float OverscanScale = 1.0f + InOutSampleState.OverscanPercentage;

			const float HalfOrthoWidth = (CameraInfo.ViewInfo.OrthoWidth / 2.0f) * OverscanScale;
			const float ScaledOrthoHeight = (CameraInfo.ViewInfo.OrthoWidth / 2.0f) * OverscanScale * YScale;

			const float NearPlane = CameraInfo.ViewInfo.OrthoNearClipPlane;
			const float FarPlane = CameraInfo.ViewInfo.OrthoFarClipPlane;

			const float ZScale = 1.0f / (FarPlane - NearPlane);
			const float ZOffset = -NearPlane;

			BaseProjMatrix = FReversedZOrthoMatrix(
				HalfOrthoWidth,
				ScaledOrthoHeight,
				ZScale,
				ZOffset
			);
			ViewInitOptions.bUseFauxOrthoViewPos = true;
			
			// Modify the projection matrix to do an off center projection, with overlap for high-res tiling
			const bool bOrthographic = true;
			ModifyProjectionMatrixForTiling(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale);
		}
		else
		{
			float XAxisMultiplier;
			float YAxisMultiplier;

			if (CameraInfo.ViewInfo.bConstrainAspectRatio)
			{
				// If the camera's aspect ratio has a thinner width, then stretch the horizontal fov more than usual to 
				// account for the extra with of (before constraining - after constraining)
				if (CameraAspectRatio < DestAspectRatio)
				{
					const float ConstrainedWidth = ViewInitOptions.GetViewRect().Height() * CameraAspectRatio;
					XAxisMultiplier = ConstrainedWidth / (float)ViewInitOptions.GetViewRect().Width();
					YAxisMultiplier = CameraAspectRatio;
				}
				// Simplified some math here but effectively functions similarly to the above, the unsimplified code would look like:
				// const float ConstrainedHeight = ViewInitOptions.GetViewRect().Width() / CameraCache.AspectRatio;
				// YAxisMultiplier = (ConstrainedHeight / ViewInitOptions.GetViewRect.Height()) * CameraCache.AspectRatio;
				else
				{
					XAxisMultiplier = 1.0f;
					YAxisMultiplier = ViewInitOptions.GetViewRect().Width() / (float)ViewInitOptions.GetViewRect().Height();
				}
			}
			else
			{
				const int32 DestSizeX = ViewInitOptions.GetViewRect().Width();
				const int32 DestSizeY = ViewInitOptions.GetViewRect().Height();
				const EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULocalPlayer>()->AspectRatioAxisConstraint;
				if (((DestSizeX > DestSizeY) && (AspectRatioAxisConstraint == AspectRatio_MajorAxisFOV)) || (AspectRatioAxisConstraint == AspectRatio_MaintainXFOV))
				{
					//if the viewport is wider than it is tall
					XAxisMultiplier = 1.0f;
					YAxisMultiplier = ViewInitOptions.GetViewRect().Width() / (float)ViewInitOptions.GetViewRect().Height();
				}
				else
				{
					//if the viewport is taller than it is wide
					XAxisMultiplier = ViewInitOptions.GetViewRect().Height() / (float)ViewInitOptions.GetViewRect().Width();
					YAxisMultiplier = 1.0f;
				}
			}

			const float MinZ = CameraInfo.ViewInfo.GetFinalPerspectiveNearClipPlane();
			const float MaxZ = MinZ;
			// Avoid zero ViewFOV's which cause divide by zero's in projection matrix
			const float MatrixFOV = FMath::Max(0.001f, ViewFOV) * (float)PI / 360.0f;


			if ((bool)ERHIZBuffer::IsInverted)
			{
				BaseProjMatrix = FReversedZPerspectiveMatrix(
					MatrixFOV,
					MatrixFOV,
					XAxisMultiplier,
					YAxisMultiplier,
					MinZ,
					MaxZ
				);
			}
			else
			{
				BaseProjMatrix = FPerspectiveMatrix(
					MatrixFOV,
					MatrixFOV,
					XAxisMultiplier,
					YAxisMultiplier,
					MinZ,
					MaxZ
				);
			}

			// yanai -->
			float ShiftLensValue = 0.f;
			if (GetWorld()->GetFirstPlayerController()->PlayerCameraManager)
			{
				// This only works if you use a Shift Lens Cine Camera (which is almost guranteed with Sequencer) and it's easier (and less human error prone) than re-deriving the information
				AShiftLensCineCameraActor* ShiftLensCineCameraActor = Cast<AShiftLensCineCameraActor>(GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetViewTarget());
				if (ShiftLensCineCameraActor)
				{
					ShiftLensValue = ShiftLensCineCameraActor->GetShiftLens();
					UE_LOG(LogMovieRenderPipeline, Log, TEXT("shift lens value: %f"), ShiftLensValue);
				}
			}
			// yanai <--

			// Modify the perspective matrix to do an off center projection, with overlap for high-res tiling
			const bool bOrthographic = false;
			ModifyProjectionMatrixForTilingShift(InOutSampleState, bOrthographic, /*InOut*/ BaseProjMatrix, DofSensorScale, ShiftLensValue); // yanai
			// ToDo: Does orthographic support tiling in the same way or do I need to modify the values before creating the ortho view.
		}
	}
		// BaseProjMatrix may be perspective or orthographic.
		ViewInitOptions.ProjectionMatrix = BaseProjMatrix;

	ViewInitOptions.SceneViewStateInterface = GetSceneViewStateInterface(OptPayload);
	ViewInitOptions.FOV = ViewFOV;
	ViewInitOptions.DesiredFOV = ViewFOV;

	FSceneView* View = new FSceneView(ViewInitOptions);
	ViewFamily->Views.Add(View);

	
	View->ViewLocation = CameraInfo.ViewInfo.Location;
	View->ViewRotation = CameraInfo.ViewInfo.Rotation;
	// Override previous/current view transforms so that tiled renders don't use the wrong occlusion/motion blur information.
	View->PreviousViewTransform = CameraInfo.ViewInfo.PreviousViewTransform;

	View->StartFinalPostprocessSettings(View->ViewLocation);
	BlendPostProcessSettings(View, InOutSampleState, OptPayload);

	// Scaling sensor size inversely with the the projection matrix [0][0] should physically
	// cause the circle of confusion to be unchanged.
	View->FinalPostProcessSettings.DepthOfFieldSensorWidth *= DofSensorScale;
	// Modify the 'center' of the lens to be offset for high-res tiling, helps some effects (vignette) etc. still work.
	View->LensPrincipalPointOffsetScale = (FVector4f)CalculatePrinciplePointOffsetForTiling(InOutSampleState); // LWC_TODO: precision loss. CalculatePrinciplePointOffsetForTiling() could return float, it's normalized?
	View->EndFinalPostprocessSettings(ViewInitOptions);

	// This metadata is per-file and not per-view, but we need the blended result from the view to actually match what we rendered.
	// To solve this, we'll insert metadata per renderpass, separated by render pass name.
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/fstop"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFstop));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/fov"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(ViewInitOptions.FOV));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/focalDistance"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldFocalDistance));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/sensorWidth"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(View->FinalPostProcessSettings.DepthOfFieldSensorWidth));
	InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/overscanPercent"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(InOutSampleState.OverscanPercentage));
	// yanai shift lens -->
	AShiftLensCineCameraActor* ShiftLensCineCameraActor = Cast<AShiftLensCineCameraActor>(GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetViewTarget());
	if (ShiftLensCineCameraActor)
	{
		InOutSampleState.OutputState.FileMetadata.Add(FString::Printf(TEXT("unreal/%s/%s/shiftLens"), *PassIdentifier.CameraName, *PassIdentifier.Name), FString::SanitizeFloat(ShiftLensCineCameraActor->GetShiftLens()));	// yanai
	}
	// yanai shift lens <--

	InOutSampleState.OutputState.FileMetadata.Append(CameraInfo.FileMetadata);
	return View;
}

void UMoviePipelineImageShiftPassBase::ModifyProjectionMatrixForTilingShift(const FMoviePipelineRenderPassMetrics& InSampleState, const bool bInOrthographic, FMatrix& InOutProjectionMatrix, float& OutDoFSensorScale, float& ShiftLensValue) const
{
	float PadRatioX = 1.0f;
	float PadRatioY = 1.0f;

	if (InSampleState.OverlappedPad.X > 0 && InSampleState.OverlappedPad.Y > 0)
	{
		PadRatioX = float(InSampleState.OverlappedPad.X * 2 + InSampleState.TileSize.X) / float(InSampleState.TileSize.X);
		PadRatioY = float(InSampleState.OverlappedPad.Y * 2 + InSampleState.TileSize.Y) / float(InSampleState.TileSize.Y);
	}

	float ScaleX = PadRatioX / float(InSampleState.TileCounts.X);
	float ScaleY = PadRatioY / float(InSampleState.TileCounts.Y);

	InOutProjectionMatrix.M[0][0] /= ScaleX;
	InOutProjectionMatrix.M[1][1] /= ScaleY;
	OutDoFSensorScale = ScaleX;

	// this offset would be correct with no pad
	float OffsetX = -((float(InSampleState.TileIndexes.X) + 0.5f - float(InSampleState.TileCounts.X) / 2.0f) * 2.0f);
	float OffsetY = ((float(InSampleState.TileIndexes.Y) + 0.5f - float(InSampleState.TileCounts.Y) / 2.0f) * 2.0f);

	if (bInOrthographic)
	{
		InOutProjectionMatrix.M[3][0] += OffsetX / PadRatioX;
		InOutProjectionMatrix.M[3][1] += OffsetY / PadRatioY;
	}
	else
	{
	InOutProjectionMatrix.M[2][0] += OffsetX / PadRatioX;
		InOutProjectionMatrix.M[2][1] += OffsetY / PadRatioY;
		
		// Apply shift lens
		float OverlapScaleY = 1.0f + float(2 * InSampleState.OverlappedPad.Y) / float(InSampleState.TileSize.Y);//yanai
		InOutProjectionMatrix.M[2][1] -= (ShiftLensValue * (float)InSampleState.TileCounts.Y / OverlapScaleY);
	
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tInOutProjectionMatrix.M[2][1]: %f"), (float)InOutProjectionMatrix.M[2][1]);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tPadRatioY: %f"), (float)PadRatioY);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tScaleY: %f"), (float)ScaleY);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tOverlapScaleY: %f"), OverlapScaleY);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tInSampleState.OverlappedPad.Y: %f"), (float)InSampleState.OverlappedPad.Y);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tInSampleState.TileSize.Y: %f"), (float)InSampleState.TileSize.Y);
		UE_LOG(LogMovieRenderPipeline, Log, TEXT("\tInSampleState.TileCounts.Y: %f"), (float)InSampleState.TileCounts.Y);
	}
}
