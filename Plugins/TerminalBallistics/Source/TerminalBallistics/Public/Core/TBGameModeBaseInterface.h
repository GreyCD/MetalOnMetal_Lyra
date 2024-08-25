// Copyright Erik Hedberg. All Rights Reserved.

#pragma once

#include "Types/TBImpactParams.h"
#include "Types/TBProjectileInjury.h"
#include "UObject/Interface.h"

#include "TBGameModeBaseInterface.generated.h"



UINTERFACE(MinimalAPI)
class UTerminalBallisticsGameModeBaseInterface : public UInterface
{
	GENERATED_BODY()
};

class TERMINALBALLISTICS_API ITerminalBallisticsGameModeBaseInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void ProjectileImpactEvent(const FTBImpactParamsBasic& impactParams);
	virtual void ProjectileImpactEvent_Implementation(const FTBImpactParamsBasic& impactParams) {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void ProjectileExitEvent(const FTBImpactParamsBasic& impactParams);
	virtual void ProjectileExitEvent_Implementation(const FTBImpactParamsBasic& impactParams) {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void ProjectileInjureEvent(const FTBImpactParamsBasic& impactParams, const FTBProjectileInjuryParams& injuryParams);
	virtual void ProjectileInjureEvent_Implementation(const FTBImpactParamsBasic& impactParams, const FTBProjectileInjuryParams& injuryParams) {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void BulletImpactEvent(const FTBImpactParams& impactParams);
	virtual void BulletImpactEvent_Implementation(const FTBImpactParams& impactParams) {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void BulletExitEvent(const FTBImpactParams& impactParams);
	virtual void BulletExitEvent_Implementation(const FTBImpactParams& impactParams) {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Terminal Ballistics|Game Mode")
	void BulletInjureEvent(const FTBImpactParams& impactParams, const FTBProjectileInjuryParams& injuryParams);
	virtual void BulletInjureEvent_Implementation(const FTBImpactParams& impactParams, const FTBProjectileInjuryParams& injuryParams) {}
};
