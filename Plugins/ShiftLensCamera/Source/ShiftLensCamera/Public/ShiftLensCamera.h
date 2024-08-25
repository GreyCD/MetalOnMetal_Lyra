// Created by christinayan01 by Takahiro Yanai. 2020.10.4
// Last Update: 2020.11.1
// https://christinayan01.jp/architecture/ | ARCHITECTURE GRAVURE

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FShiftLensCameraModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
