// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE

#include "ShiftLensCamera.h"

//#include "PlacementMode/Public/IPlacementModeModule.h"		// yanai add
//#include "ShiftLensCineCameraActor.h"	// yanai add

#define LOCTEXT_NAMESPACE "FShiftLensCameraModule"

void FShiftLensCameraModule::StartupModule()
{
	/*
	if (!IPlacementModeModule::IsAvailable())
	{
		return;
	}

	IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

	//const FPlacementCategoryInfo* category = 0;// IPlacementModeModule::Get().GetRegisteredPlacementCategory("Cinematic");
	FPlacementCategoryInfo Info(
		//LOCTEXT("MyCategory", "MyCategory"),
		FText::FromString(TEXT("MyCategory")),
		"MyCategory",
		TEXT("PMMyCategory"),
		45
	);
	PlacementModeModule.RegisterPlacementCategory(Info);
	PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, 
		MakeShareable(new FPlaceableItem(nullptr, FAssetData(AShiftLensCineCameraActor::StaticClass()))));
	*/
	/*
	FPlacementCategoryInfo Info(
		LOCTEXT("CinematicCategoryName", "Cinematic"),
		"Cinematic",
		TEXT("PMCinematic"),
		25
	);

	IPlacementModeModule::Get().RegisterPlacementCategory(Info);
	IPlacementModeModule::Get().RegisterPlaceableItem(Info.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(AShiftLensCineCameraActor::StaticClass()))));
	//IPlacementModeModule::Get().RegisterPlaceableItem(Info.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(ACineCameraActor::StaticClass()))));
	//IPlacementModeModule::Get().RegisterPlaceableItem(Info.UniqueHandle, MakeShareable(new FPlaceableItem(nullptr, FAssetData(ACameraRig_Crane::StaticClass()))));
	*/
}

void FShiftLensCameraModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	/*
	if (IPlacementModeModule::IsAvailable())
	{
		IPlacementModeModule::Get().UnregisterPlacementCategory("Cinematic");
	}
	*/
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShiftLensCameraModule, ShiftLensCamera)