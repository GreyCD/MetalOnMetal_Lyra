// Copyright Erik Hedberg. All rights reserved.

#include "AsyncNodes/TBFireBulletAsync.h"

#include "Core/TBBulletDataAsset.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Misc/TBMacrosAndFunctions.h"
#include "Subsystems/TerminalBallisticsSubsystem.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBOptionalDelegate.h"
#include "Types/TBProjectileFlightData.h"
#include "Types/TBProjectileId.h"
#include "Types/TBProjectileInjury.h"
#include "Types/TBTaskCallbacks.h"


void UTBFireBulletAsync::BeginDestroy()
{
	Delegates.Clear();
	Super::BeginDestroy();
}

UTBFireBulletAsync* UTBFireBulletAsync::CreateProxyObject(
	TSoftObjectPtr<UBulletDataAsset> Bullet,
	FTBLaunchParams& LaunchParams,
	const FTBProjectileId& DesiredId,
	FTBProjectileId& ActualId,
	int32 DebugType,
	bool bEnableUpdate
)
{
	UTBFireBulletAsync* Proxy = NewObject<UTBFireBulletAsync>();
	Proxy->SetFlags(RF_StrongRefOnFrame);

	ActualId = Proxy->AddAndFireBulletAsync(Bullet, LaunchParams, DesiredId, DebugType, bEnableUpdate);
	return Proxy;
}

FTBProjectileId UTBFireBulletAsync::AddAndFireBulletAsync(
	TSoftObjectPtr<UBulletDataAsset> Bullet,
	FTBLaunchParams& LaunchParams,
	const FTBProjectileId& DesiredId,
	int32 DebugType,
	bool bEnableUpdate
)
{
	using TB::SimTasks::FBulletTaskDelegates;

	if(bEnableUpdate)
	{
		Delegates = FBulletTaskDelegates(
			this,
			_MakeDelegateParams(&UTBFireBulletAsync::OnComplete),
			_MakeDelegateParams(&UTBFireBulletAsync::OnHit),
			_MakeDelegateParams(&UTBFireBulletAsync::OnExitHit),
			_MakeDelegateParams(&UTBFireBulletAsync::OnInjure),
			_MakeDelegateParams(&UTBFireBulletAsync::OnUpdate)
		);
	}
	else
	{
		Delegates = FBulletTaskDelegates(
			this,
			_MakeDelegateParams(&UTBFireBulletAsync::OnComplete),
			_MakeDelegateParams(&UTBFireBulletAsync::OnHit),
			_MakeDelegateParams(&UTBFireBulletAsync::OnExitHit),
			_MakeDelegateParams(&UTBFireBulletAsync::OnInjure),
			nullptr, NAME_None
		);
	}

	if (LaunchParams.Owner)
	{
		if (UTerminalBallisticsSubsystem* TBSubsystem = FTBGetSubsystemHelper::GetTBSubsystem(LaunchParams.Owner))
		{
			UBulletDataAsset* BulletDataAsset = Bullet.LoadSynchronous();
			if (TB_VALID(BulletDataAsset))
			{
				return TBSubsystem->AddAndFireBulletWithCallbacks(BulletDataAsset, LaunchParams, FTBBulletTaskCallbacks(Delegates), DesiredId, DebugType);
			}
		}
	}
	return FTBProjectileId::None;
}

void UTBFireBulletAsync::OnComplete(const FTBProjectileId& Id, const TArray<struct FPredictProjectilePathPointData>& Results)
{
	onComplete.Broadcast(FTBCompletionResult{ Id, Results });
}

void UTBFireBulletAsync::OnHit(const FTBImpactParams& ImpactParams)
{
	onHit.Broadcast(ImpactParams);
}

void UTBFireBulletAsync::OnExitHit(const FTBImpactParams& ImpactParams)
{
	onExitHit.Broadcast(ImpactParams);
}

void UTBFireBulletAsync::OnInjure(const FTBImpactParams& ImpactParams, const FTBProjectileInjuryParams& InjuryParams)
{
	onInjure.Broadcast(FTBBulletInjuryResult{ ImpactParams, InjuryParams });
}

void UTBFireBulletAsync::OnUpdate(const FTBProjectileFlightData& FlightData)
{
	onUpdate.Broadcast(FlightData);
}
