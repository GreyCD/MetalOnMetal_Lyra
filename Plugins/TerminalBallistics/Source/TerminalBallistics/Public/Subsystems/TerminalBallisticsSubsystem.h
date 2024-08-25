// Copyright Erik Hedberg. All Rights Reserved.

#pragma once

#include "Async/ParallelFor.h"
#include "Containers/Queue.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Misc/Optional.h"
#include "Misc/ScopeRWLock.h"
#include "Misc/TBConfiguration.h"
#include "Misc/TBMacrosAndFunctions.h"
#include "Misc/TVariant.h"
#include "Subsystems/SubsystemCollection.h"
#include "Subsystems/WorldSubsystem.h"
#include "Threading/TBProjectileTaskResult.h"
#include "Types/TBEnums.h"
#include "Types/TBImpactParams.h"
#include "Types/TBLaunchTypes.h"
#include "Types/TBProjectile.h"
#include "Types/TBProjectileFlightData.h"
#include "Types/TBProjectileId.h"
#include "Types/TBProjectileInjury.h"
#include "Types/TBSimData.h"
#include "Types/TBTaskCallbacks.h"
#include "Types/TBTraits.h"
#include <atomic>
#include <type_traits>

#include "TerminalBallisticsSubsystem.generated.h"


class UBulletDataAsset;
class UEnvironmentSubsystem;
class FTBProjectileThread;
class ITerminalBallisticsGameModeBaseInterface;

typedef TSharedPtr<struct FTBBullet> BulletPointer;


DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnSimTaskExit, const int, ExitCode, const FTBProjectileId&, Id, const TArray<FPredictProjectilePathPointData>&, Results);


UCLASS(MinimalApi)
class UTBProjectileThreadQueue : public UObject
{
	GENERATED_BODY()
public:
	UTBProjectileThreadQueue(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}

	UTBProjectileThreadQueue(FVTableHelper& Helper)
		: Super(Helper)
	{}

	~UTBProjectileThreadQueue();

	inline void Empty()
	{
		BulletInputQueue.Empty();
		BulletOutputQueue.Empty();
		ProjectileInputQueue.Empty();
		ProjectileOutputQueue.Empty();
	}

	inline bool HasInputData() const
	{
		return !BulletInputQueue.IsEmpty()
			|| !ProjectileInputQueue.IsEmpty();
	}

	inline bool HasOutputData() const
	{
		return !BulletOutputQueue.IsEmpty()
			|| !ProjectileOutputQueue.IsEmpty();
	}

	inline bool Enqueue(FTBBulletSimData&& BulletSimData)
	{
		//UnpauseThread();
		return BulletInputQueue.Enqueue(std::forward<FTBBulletSimData>(BulletSimData));
	}

	inline bool Enqueue(FTBProjectileSimData&& ProjectileSimData)
	{
		//UnpauseThread();
		return ProjectileInputQueue.Enqueue(std::forward<FTBProjectileSimData>(ProjectileSimData));
	}

	inline bool Enqueue(FBulletTaskResult&& TaskResult)
	{
		UnpauseThread();
		return BulletOutputQueue.Enqueue(std::forward<FBulletTaskResult>(TaskResult));
	}

	inline bool Enqueue(FProjectileTaskResult&& TaskResult)
	{
		UnpauseThread();
		return ProjectileOutputQueue.Enqueue(std::forward<FProjectileTaskResult>(TaskResult));
	}

	inline bool Dequeue(FBulletTaskResult& TaskResult)
	{
		return BulletOutputQueue.Dequeue(TaskResult);
	}

	inline bool Dequeue(FProjectileTaskResult& TaskResult)
	{
		return ProjectileOutputQueue.Dequeue(TaskResult);
	}

	inline bool Dequeue(FTBBulletSimData& BulletSimData)
	{
		return BulletInputQueue.Dequeue(BulletSimData);
	}

	inline bool Dequeue(FTBProjectileSimData& ProjectileSimData)
	{
		return ProjectileInputQueue.Dequeue(ProjectileSimData);
	}

	inline void SetUnpauseFunction(TUniqueFunction<void()> Function)
	{
		UnpauseThreadFunc = MoveTemp(Function);
	}

	inline void UnpauseThread()
	{
		TB_RET_COND(!IsThreadPaused);

		if (UnpauseThreadFunc) {
			UnpauseThreadFunc();
		}
	}

	std::atomic_bool IsThreadPaused;

protected:
	TUniqueFunction<void()> UnpauseThreadFunc;

	TQueue<FTBBulletSimData, EQueueMode::Mpsc> BulletInputQueue;
	TQueue<FTBProjectileSimData, EQueueMode::Mpsc> ProjectileInputQueue;

	TQueue<FBulletTaskResult, EQueueMode::Mpsc> BulletOutputQueue;
	TQueue<FProjectileTaskResult, EQueueMode::Mpsc> ProjectileOutputQueue;
};

/**
* 
* Simplified Projectile Lifetime:
* 
*	Tick:
*		Pending removals are processed.		(RemovalQueue)
*		Pending additions are processed.	(ToAdd)
*		Any inactive items that are requesting to be made active are added to the Active array.
*		Pending launches are processed.		(ToLaunch)
*		Results are taken from the ProjectileThread and processed. (GetResultsFromProjectileThread)
*	
*	Add:
*		Data is converted to FTBSimData and assigned an Id. 
*		If projectiles can be added:
			SimData is added to the Inactive array, ready to be launched.
*		Otherwise:
			SimData is added to the ToAdd queue, and will be added next tick.
* 
*	Remove:
*		Appropriate arrays are locked. (Inactive and Active)
*		Data is removed	from active/inactive arrays and destructed.
*		Arrays are unlocked.
* 
*	Fire:
*		SimData is retrieved from the Inactive array.
*		If launching is not allowed:
*			launch data is added to the ToLaunch queue, and will be launched next tick.
*		Otherwise:
*			SimData is configured with the given launch data. (FTBLaunchParams, DebugType, etc...)
*			Projectile is launched.
*			SimData moved from Inactive to Active.
* 
*	OnProjectileComplete:
*		Projectile is added to the removal queue.
* 
* Simplified Projectile Lifetime on the ProjectileThread:
* 
*		Projectiles are added to the thread using the thread's DataQueue input. (DataQueue->Enqueue)
*		From there, the Projectile is picked up by the ProjectileThread and it begins simulation.
*		Delegates bound to the Projectile are executed on the game thread using the task graph system.
* 
*		Once complete, the Projectile is removed from the ProjectileThread and it's TaskResult is added to the DataQueue's output.
*		Next tick (or potentially the current tick), the DataQueue's output is checked by this subsystem, where TaskResults are retrieved from the DataQueue (DataQueue->Dequeue) and processed.
*		Then they are removed and the associated Projectile is added to the removal queue.
* 
*/
UCLASS(meta = (ShortToolTip="Subsystem that handles ballistics."))
class TERMINALBALLISTICS_API UTerminalBallisticsSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	UTerminalBallisticsSubsystem();

	// FTickableGameObject implementation Begin
	virtual ~UTerminalBallisticsSubsystem() override
	{
		ShutdownProjectileThread();
		ActiveBullets.Empty();
		InactiveBullets.Empty();
		ActiveProjectiles.Empty();
		InactiveProjectiles.Empty();
	}
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UTerminalBallisticsSubsystem, STATGROUP_Tickables); }
	// FTickableGameObject implementation End

	// USubsystem implementation Begin
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	// USubsystem implementation End


	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

#pragma region Getters
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetNumActiveBullets() const { return ActiveBullets.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetNumActiveProjectiles() const { return ActiveProjectiles.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline bool HasActiveBullets() const { return !ActiveBullets.IsEmpty(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline bool HasActiveProjectiles() const { return !ActiveProjectiles.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetNumInactiveBullets() const { return InactiveBullets.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetNumInactiveProjectiles() const { return InactiveProjectiles.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline bool HasInactiveBullets() const { return !InactiveBullets.IsEmpty(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline bool HasInactiveProjectiles() const { return !InactiveProjectiles.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetTotalBullets() const { return InactiveBullets.Num() + ActiveBullets.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	inline int GetTotalProjectiles() const { return InactiveProjectiles.Num() + ActiveProjectiles.Num(); }
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	bool HasAnyBullets() const;
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	bool HasAnyProjectiles() const;

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem")
	/**
	* Checks to see if there are any bullets/projectiles on the Projectile thread.
	*/
	bool HasAnyBulletsOrProjectiles() const;

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Bullet Found"))
	/**
	* Checks to see if there is a bullet with the given Id.
	* @param Id		The Id to check for.
	*/
	bool HasBullet(const FTBProjectileId& Id) const;
	bool HasBullet(const FTBBulletSimData Bullet) const;

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Projectile Found"))
	/**
	* Checks to see if there is a projectile with the given Id.
	* @param Id		The Id to check for.
	*/
	bool HasProjectile(const FTBProjectileId& Id) const;
	bool HasProjectile(const FTBProjectileSimData& Projectile) const;

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Bullet Found"))
	/**
	* Checks to see if there is a bullet with the given Id currently inactive.
	* @param Id		The Id to check for.
	*/
	inline bool HasBulletInInactive(const FTBProjectileId& Id) const
	{
		auto Pred = [Id](const FTBBulletSimData& SimData) { return SimData.GetId() == Id; };
		return InactiveBullets.ContainsByPredicate(Pred);
	}
	inline bool HasBulletInInactive(const FTBBulletSimData Bullet) const
	{
		return InactiveBullets.Contains(Bullet);
	}

	inline FTBBulletSimData* GetBulletFromInactive(const FTBProjectileId& Id)
	{
		auto Pred = [Id](const FTBBulletSimData& SimData) { return SimData.GetId() == Id; };
		FReadScopeLock Lock(InactiveBulletsLock);
		return InactiveBullets.FindByPredicate(Pred);
	}


	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Projectile Found"))
	/**
	* Checks to see if there is a projectile with the given Id currently inactive.
	* @param Id		The Id to check for.
	*/
	inline bool HasProjectileInInactive(const FTBProjectileId& Id) const
	{
		auto Pred = [Id](const FTBProjectileSimData& SimData) { return SimData.GetId() == Id; };
		return InactiveProjectiles.ContainsByPredicate(Pred);
	}
	inline bool HasProjectileInInactive(const FTBProjectileSimData& Projectile) const
	{
		return InactiveProjectiles.Contains(Projectile);
	}

	inline FTBProjectileSimData* GetProjectileFromInactive(const FTBProjectileId& Id)
	{
		auto Pred = [Id](const FTBProjectileSimData& SimData) { return SimData.GetId() == Id; };
		FReadScopeLock Lock(InactiveProjectilesLock);
		return InactiveProjectiles.FindByPredicate(Pred);
	}

	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Bullet Found"))
	bool GetBullet(const FTBProjectileId& Id, FTBBulletSimData& Bullet) const;
	UFUNCTION(BlueprintPure, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Projectile Found"))
	bool GetProjectile(const FTBProjectileId& Id, FTBProjectileSimData& Projectile) const;
#pragma endregion

	FORCEINLINE bool CanFire() const
	{
		return ProjectilesLaunchedThisTick < TB::Configuration::MaxLaunchesPerTick
			&& !bShuttingDown;
	}

	double UpdateProjectileDrag(const double V, const double GravityZ, const FVector& Location, const FTBProjectilePhysicalProperties& ProjectileProperties, ETBDragComplexity DragCalculationType = ETBDragComplexity::DEFAULT, double p = -1, const bool bAllowAtmosphericDensityOverride = true);
	FVector CalculateProjectileDrag(const FVector& V, const double GravityZ, const FVector& Location, const FQuat& ProjectileRotation, const FTBProjectilePhysicalProperties& ProjectileProperties, ETBDragComplexity DragCalculationType = ETBDragComplexity::DEFAULT, double p = -1, const bool bAllowAtmosphericDensityOverride = true);

	/* Add/Remove/Get methods for projectiles and bullets are thread-safe */
#pragma region Bullet Functions
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType"))
	FTBBulletSimData CreateBulletSimDataFromDataAsset(const UBulletDataAsset* BulletDataAsset, const FTBProjectileId& Id, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);

	TArray<FTBBulletSimData> CreateBulletSimDataFromDataAssetMultiple(const UBulletDataAsset* BulletDataAsset, const FTBProjectileId Id, const int32 DebugType = 0);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void BindFunctionsToBulletSimData(UPARAM(Ref)FTBBulletSimData& BulletSimData, FBPOnProjectileComplete BulletComplete, FBPOnBulletHit BulletHit, FBPOnBulletExitHit BulletExitHit, FBPOnBulletInjure BulletInjure);

	/**
	 * Adds a bullet to the subsystem and returns a unique Id for that bullet.
	 *
	 * @param ToAdd				The bullet to add.
	 * @return FTBProjectileId	The Id assigned to the added bullet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddBullet(UPARAM(Ref)FTBBulletSimData& ToAdd);
	/**
	 * Adds a bullet to the subsystem and returns a unique Id for that bullet.
	 *
	 * @param Bullet			The bullet to add.
	 * @param DesiredId			Optional Id to be used with this bullet. If not provided, one will be generated.
	 * @return FTBProjectileId	The Id assigned to the added bullet, if one was not provided.
	 */
	FTBProjectileId AddBullet(
		BulletPointer Bullet,
		const FTBProjectileId& DesiredId = FTBProjectileId::None
	);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (DisplayName = "Add Bullets", ReturnDisplayName = "Ids"))
	TArray<FTBProjectileId> BP_AddBullets(UPARAM(Ref)TArray<FTBBulletSimData>& ToAdd)
	{
		return AddBullets(ToAdd);
	}

	/**
	 * Adds a bullet from a data asset to the subsystem for simulation and returns the Id of that bullet.
	 *
	 * @param BulletDataAsset	A data asset containing the bullet data.
	 * @param DesiredId			Optional Id to be used with this bullet. If not provided, one will be generated.
	 * @param DebugType			Debug type for the bullet.
	 * @return Id	The Id assigned to the added bullet, if one was not provided.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType, DesiredId", AutoCreateRefTerm = "DesiredId", ReturnDisplayName = "Id"))
	FTBProjectileId AddBulletFromDataAsset(const UBulletDataAsset* BulletDataAsset, const FTBProjectileId& DesiredId, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);

	/**
	 * Adds a bullet with various callback functions and returns a unique Id.
	 *
	 * @param ToAdd				The bullet to add.
	 * @param OnBulletComplete	Callback function for bullet completion.
	 * @param OnBulletHit		Callback function for bullet hitting an object.
	 * @param OnBulletExitHit	Callback function for bullet exiting after hitting an object.
	 * @param OnBulletInjure	Callback function for bullet causing injury to a player/pawn.
	 * @return FTBProjectileId	The Id assigned to the added bullet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddBulletWithCallbacks(
		UPARAM(Ref)FTBBulletSimData& ToAdd,
		FBPOnProjectileComplete OnBulletComplete,
		FBPOnBulletHit OnBulletHit,
		FBPOnBulletExitHit OnBulletExitHit,
		FBPOnBulletInjure OnBulletInjure
	);
	FTBProjectileId AddBulletWithCallbacks(
		FTBBulletSimData& ToAdd,
		FTBBulletTaskCallbacks Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None
	);
	FTBProjectileId AddBulletWithCallbacks(
		BulletPointer ToAdd,
		FTBBulletTaskCallbacks Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None
	);

	/**
	 * Adds a bullet with callbacks and a delegate to be called on update. Returns the unique Id assigned to that bullet.
	 *
	 * @param ToAdd				The bullet to add.
	 * @param OnBulletComplete	Callback function for bullet completion.
	 * @param OnBulletHit		Callback function for bullet hitting an object.
	 * @param OnBulletExitHit	Callback function for bullet exiting after hitting an object.
	 * @param OnBulletInjure	Callback function for bullet causing injury to a player/pawn.
	 * @param OnUpdate			Callback function for continuous bullet updates.
	 * @return FTBProjectileId	The Id assigned to the added bullet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddBulletWithCallbacksAndUpdate(
		UPARAM(Ref)FTBBulletSimData& ToAdd,
		FBPOnProjectileComplete OnBulletComplete,
		FBPOnBulletHit OnBulletHit,
		FBPOnBulletExitHit OnBulletExitHit,
		FBPOnBulletInjure OnBulletInjure,
		FBPOnProjectileUpdate OnUpdate
	);

	/**
	 * Removes a bullet.
	 * This will stop any ongoing simulation of that bullet and also remove it from the pool of inactive bullets waiting to be fired.
	 *
	 * @param ToRemove The bullet to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveBullet(UPARAM(Ref)FTBBulletSimData& ToRemove);

	/**
	 * Removes a bullet based on its Id.
	 * This will stop any ongoing simulation of that bullet and also remove it from the pool of inactive bullets waiting to be fired.
	 *
	 * @param Id The unique Id of the bullet to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveBulletById(const FTBProjectileId& Id);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType"))
	void FireBullet(const FTBProjectileId& BulletId, const FTBLaunchParams& LaunchParams, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);

	/**
	* Explicitly fires a bullet.
	* Bullet is assumed to have already been added to the subsystem. If not, this can result in side effects.
	* Use with caution.
	*/
	void FireBulletExplicit(FTBBulletSimData* BulletToFire, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType", AutoCreateRefTerm = "Id", ReturnDisplayName = "Id"))
	FTBProjectileId AddAndFireBullet(const UBulletDataAsset* BulletDataAsset, const FTBLaunchParams& LaunchParams, const FTBProjectileId& Id, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);

	FTBProjectileId AddAndFireBullet(BulletPointer Bullet, const FTBLaunchParams& LaunchParams, int32 DebugType = 0, const FTBProjectileId& Id = FTBProjectileId::None);

	FTBProjectileId AddAndFireBullet(FTBBulletSimData& SimData, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);

	TArray<FTBProjectileId> AddAndFireBullets(TArray<FTBBulletSimData>& SimData, const FTBLaunchParams& LaunchParams, int32 DebugType = 0, TOptional<FTBBulletTaskCallbacks> Delegates = TOptional<FTBBulletTaskCallbacks>());
	TArray<FTBProjectileId> AddAndFireBullets(TArray<FTBBulletSimData>& SimData, const TArray<FTBLaunchParams>& LaunchParams, int32 DebugType = 0, TOptional<FTBBulletTaskCallbacks> Delegates = TOptional<FTBBulletTaskCallbacks>());

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType, Id", ReturnDisplayName = "Id", AutoCreateRefTerm = "Id"))
	FTBProjectileId AddAndFireBulletWithCallbacks(
		const UBulletDataAsset* BulletDataAsset,
		const FTBLaunchParams& LaunchParams,
		FBPOnProjectileComplete OnBulletComplete,
		FBPOnBulletHit OnBulletHit,
		FBPOnBulletExitHit OnBulletExitHit,
		FBPOnBulletInjure OnBulletInjure,
		const FTBProjectileId& Id,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0
	);

	FTBProjectileId AddAndFireBulletWithCallbacks(
		const UBulletDataAsset* BulletDataAsset,
		const FTBLaunchParams& LaunchParams,
		TOptional<FTBBulletTaskCallbacks> Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None,
		int32 DebugType = 0
	);
	FTBProjectileId AddAndFireBulletWithCallbacks(
		BulletPointer Bullet,
		const FTBLaunchParams& LaunchParams,
		TOptional<FTBBulletTaskCallbacks> Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None,
		int32 DebugType = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType, Id", ReturnDisplayName = "Id", AutoCreateRefTerm = "Id"))
	FTBProjectileId AddAndFireBulletWithCallbacksAndUpdate(
		const UBulletDataAsset* BulletDataAsset,
		const FTBLaunchParams& LaunchParams,
		FBPOnProjectileComplete OnBulletComplete,
		FBPOnBulletHit OnBulletHit,
		FBPOnBulletExitHit OnBulletExitHit,
		FBPOnBulletInjure OnBulletInjure,
		FBPOnProjectileUpdate OnUpdate,
		const FTBProjectileId& Id,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0
	);
#pragma endregion

#pragma region Projectile Functions
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void BindFunctionsToProjectileSimData(UPARAM(Ref)FTBProjectileSimData& ProjectileSimData, FBPOnProjectileComplete OnProjectileComplete, FBPOnProjectileHit OnProjectileHit, FBPOnProjectileExitHit OnProjectileExitHit, FBPOnProjectileInjure OnProjectileInjure);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddProjectile(UPARAM(Ref)FTBProjectileSimData& ToAdd);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddProjectileWithCallbacks(
		UPARAM(Ref)FTBProjectileSimData& ToAdd,
		FBPOnProjectileComplete OnProjectileComplete,
		FBPOnProjectileHit OnProjectileHit,
		FBPOnProjectileExitHit OnProjectileExitHit,
		FBPOnProjectileInjure OnProjectileInjure
	);

	FTBProjectileId AddProjectileWithCallbacks(
		FTBProjectileSimData& ToAdd,
		FTBProjectileTaskCallbacks Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None
	);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (ReturnDisplayName = "Id"))
	FTBProjectileId AddProjectileWithCallbacksAndUpdate(
		UPARAM(Ref)FTBProjectileSimData& ToAdd,
		FBPOnProjectileComplete OnProjectileComplete,
		FBPOnProjectileHit OnProjectileHit,
		FBPOnProjectileExitHit OnProjectileExitHit,
		FBPOnProjectileInjure OnProjectileInjure,
		FBPOnProjectileUpdate OnUpdate
	);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (DisplayName = "Add Projectiles", ReturnDisplayName = "Ids"))
	TArray<FTBProjectileId> BP_AddProjectiles(UPARAM(Ref)TArray<FTBProjectileSimData>& ToAdd)
	{
		return AddProjectiles(ToAdd);
	}
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveProjectile(const FTBProjectileSimData& ToRemove);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveProjectileById(const FTBProjectileId& Id);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveProjectiles(const TArray<FTBProjectileSimData>& ToRemove);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void RemoveProjectilesById(const TArray<FTBProjectileId>& ToRemove);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType"))
	void FireProjectile(const FTBProjectileId& Id, const FTBLaunchParams& LaunchParams, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);

	/**
	* Explicitly fires a Projectile.
	* Projectile is assumed to have already been added to the subsystem. If not, this can result in side effects.
	* Use with caution.
	*/
	void FireProjectileExplicit(FTBProjectileSimData* ProjectileToFire, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem")
	void FireProjectiles(const TArray<FTBLaunchData> ProjectilesToLaunch);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType", ReturnDisplayName = "Id"))
	FTBProjectileId AddAndFireProjectile(UPARAM(Ref)FTBProjectileSimData& ProjectileSimData, const FTBLaunchParams& LaunchParams, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0);
	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType", ReturnDisplayName = "Id"))
	FTBProjectileId AddAndFireProjectileWithCallbacks(
		UPARAM(Ref)FTBProjectileSimData& ProjectileSimData,
		const FTBLaunchParams& LaunchParams,
		FBPOnProjectileComplete OnProjectileComplete,
		FBPOnProjectileHit OnProjectileHit,
		FBPOnProjectileExitHit OnProjectileExitHit,
		FBPOnProjectileInjure OnProjectileInjure,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0
	);

	UFUNCTION(BlueprintCallable, Category = "Terminal Ballistics Subsystem", meta = (AdvancedDisplay = "DebugType", ReturnDisplayName = "Id"))
	FTBProjectileId AddAndFireProjectileWithCallbacksAndUpdate(
		UPARAM(Ref)FTBProjectileSimData& ProjectileSimData,
		const FTBLaunchParams& LaunchParams,
		FBPOnProjectileComplete OnProjectileComplete,
		FBPOnProjectileHit OnProjectileHit,
		FBPOnProjectileExitHit OnProjectileExitHit,
		FBPOnProjectileInjure OnProjectileInjure,
		FBPOnProjectileUpdate OnUpdate,
		UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/TerminalBallistics.ETBBallisticsDebugType")) int32 DebugType = 0
	);

	FTBProjectileId AddAndFireProjectileWithCallbacks(
		FTBProjectileSimData& ProjectileSimData,
		const FTBLaunchParams& LaunchParams,
		FTBProjectileTaskCallbacks Callbacks,
		const FTBProjectileId& Id = FTBProjectileId::None,
		int32 DebugType = 0
	);
#pragma endregion

	void BindDelegates(FTBBulletSimData& SimData, FTBBulletTaskCallbacks Delegates);
	void BindDelegates(FTBProjectileSimData& SimData, FTBProjectileTaskCallbacks Delegates);
protected:

	TArray<FTBProjectileId> AddBullets(TArray<FTBBulletSimData>& SimData, TOptional<FTBBulletTaskCallbacks> Delegates = TOptional<FTBBulletTaskCallbacks>())
	{
		return AddMultiple(SimData, Delegates);
	}
	TArray<FTBProjectileId> AddProjectiles(TArray<FTBProjectileSimData>& SimData, TOptional<FTBProjectileTaskCallbacks> Delegates = TOptional<FTBProjectileTaskCallbacks>())
	{
		return AddMultiple(SimData, Delegates);
	}

	friend class FTBGetSubsystemHelper;

	UWorld* GetOrUpdateWorld() const;

	static bool SupportsNetMode(const ENetMode NetMode);

	void RemoveBulletByIdIgnoreThread(const FTBProjectileId& Id);
	void RemoveProjectileByIdIgnoreThread(const FTBProjectileId& Id);

	bool TryGetEnvironmentSubsystem();

	static int32 ThreadCount;

	std::atomic<int32> ProjectilesLaunchedThisTick;

	std::atomic_bool bShuttingDown; // If the world cleanup has begun or the world is preparing to shut down, make sure we don't continue launching projectiles and firing delegates.

	UFUNCTION()
	void StartShutdown(UWorld* UnusedWorld, bool UnusedBool1 = false, bool UnusedBool2 = false);

	FDelegateHandle OnWorldBeginTearDownHandle;
	FDelegateHandle OnWorldCleanupHandle;

	UPROPERTY()
	class UEnvironmentSubsystem* EnvironmentSubsystem = nullptr;

	UPROPERTY()
	mutable TObjectPtr<UWorld> World;
	
	bool bHasValidGameMode = false;

	bool bWasPaused = false;


	using ProjectileOrId = TVariant<FTBProjectileSimData, FTBProjectileId>;
	using BulletOrId = TVariant<FTBBulletSimData, FTBProjectileId>;

	TQueue<ProjectileOrId, EQueueMode::Mpsc> ProjectileRemovalQueue;
	TQueue<BulletOrId, EQueueMode::Mpsc> BulletRemovalQueue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Terminal Ballistics Subsystem")
	TArray<FTBBulletSimData> ActiveBullets;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Terminal Ballistics Subsystem")
	TArray<FTBBulletSimData> InactiveBullets;

	TQueue<FTBBulletSimData, EQueueMode::Mpsc> BulletsToMakeActive;
	FTBBulletSimDataArray BulletsToAdd;
	TQueue<FTBLaunchData, EQueueMode::Mpsc> BulletLaunchQueue;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Terminal Ballistics Subsystem")
	TArray<FTBProjectileSimData> ActiveProjectiles;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Terminal Ballistics Subsystem")
	TArray<FTBProjectileSimData> InactiveProjectiles;

	TQueue<FTBProjectileSimData, EQueueMode::Mpsc> ProjectilesToMakeActive;
	FTBProjectileSimDataArray ProjectilesToAdd;
	TQueue<FTBLaunchData, EQueueMode::Mpsc> ProjectileLaunchQueue;

	friend struct FTBSimData;
	friend class FTBProjectileThread;
	UFUNCTION()
	void CallOnBulletComplete(const FTBProjectileId& Id, const TArray<FPredictProjectilePathPointData>& pathResults);
	UFUNCTION()
	void CallOnProjectileComplete(const FTBProjectileId& Id, const TArray<FPredictProjectilePathPointData>& pathResults);

	/* Default script delegates */
	FScriptDelegate onBulletHit;
	FScriptDelegate onBulletExitHit;
	FScriptDelegate onBulletInjure;

	FScriptDelegate onProjectileHit;
	FScriptDelegate onProjectileExitHit;
	FScriptDelegate onProjectileInjure;


	inline void BindDefaultDelegates(FTBBulletSimData& SimData)
	{
		if (GetOrUpdateWorld()->GetNetMode() < ENetMode::NM_Client) // Since the client can't access the game mode anyway, there's no reason to bind those delegates.
		{
			if (!SimData.DefaultDelegatesAreBound)
			{
				SimData.onBulletHit.Add(onBulletHit);
				SimData.onBulletExitHit.Add(onBulletExitHit);
				SimData.onBulletInjure.Add(onBulletInjure);
				SimData.DefaultDelegatesAreBound = true;
			}
		}
	}
	inline void BindDefaultDelegates(FTBBulletSimData* SimData)
	{
		if (GetOrUpdateWorld()->GetNetMode() < ENetMode::NM_Client)
		{
			if (!SimData->AreDefaultDelegatesBound())
			{
				SimData->onBulletHit.Add(onBulletHit);
				SimData->onBulletExitHit.Add(onBulletExitHit);
				SimData->onBulletInjure.Add(onBulletInjure);
				SimData->DefaultDelegatesAreBound = true;
			}
		}
	}
	inline void BindDefaultDelegates(FTBProjectileSimData& SimData)
	{
		if (GetOrUpdateWorld()->GetNetMode() < ENetMode::NM_Client)
		{
			if (!SimData.DefaultDelegatesAreBound)
			{
				SimData.onProjectileHit.Add(onProjectileHit);
				SimData.onProjectileExitHit.Add(onProjectileExitHit);
				SimData.onProjectileInjure.Add(onProjectileInjure);
				SimData.DefaultDelegatesAreBound = true;
			}
		}
	}
	inline void BindDefaultDelegates(FTBProjectileSimData* SimData)
	{
		if (GetOrUpdateWorld()->GetNetMode() < ENetMode::NM_Client)
		{
			if (!SimData->AreDefaultDelegatesBound())
			{
				SimData->onProjectileHit.Add(onProjectileHit);
				SimData->onProjectileExitHit.Add(onProjectileExitHit);
				SimData->onProjectileInjure.Add(onProjectileInjure);
				SimData->DefaultDelegatesAreBound = true;
			}
		}
	}

#pragma region Game Mode Calls
	UFUNCTION()
	void CallGameModeHit(const FTBImpactParams& ImpactParams);
	UFUNCTION()
	void CallGameModeHitBasic(const FTBImpactParamsBasic& ImpactParams);
	UFUNCTION()
	void CallGameModeExitHit(const FTBImpactParams& ImpactParams);
	UFUNCTION()
	void CallGameModeExitHitBasic(const FTBImpactParamsBasic& ImpactParams);
	UFUNCTION()
	void CallGameModeInjure(const FTBImpactParams& ImpactParams, const FTBProjectileInjuryParams& Injury);
	UFUNCTION()
	void CallGameModeInjureBasic(const FTBImpactParamsBasic& ImpactParams, const FTBProjectileInjuryParams& Injury);
#pragma endregion

public:
	FBulletTaskResult FireBulletSynchronous(FTBBulletSimData* SimData, const FTBLaunchParams& LaunchParams, FTBBulletTaskCallbacks Callbacks, int32 DebugType = 0);

private:
	template<typename TypeOfSimData>
	typename TEnableIf<TB::Traits::TIsSimData<TypeOfSimData>::Value, void>::Type _SetupProjectileLaunchData(TypeOfSimData* ProjectileToFire, const FTBLaunchParams& LaunchParams, int32 DebugType);
	void _FireBulletInternal(const FTBProjectileId& BulletId, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);
	void _FireProjectileInternal(const FTBProjectileId& Id, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);
	template<typename TypeOfSimData>
	typename TEnableIf<TB::Traits::TIsSimData<TypeOfSimData>::Value, void>::Type _FireProjectileImpl(TypeOfSimData* ProjectileToFire, const FTBLaunchParams& LaunchParams, int32 DebugType = 0);

	template<typename TypeOfSimData>
	typename TEnableIf<TB::Traits::TIsSimData<TypeOfSimData>::Value, void>::Type _SetupProjectile(TypeOfSimData* ProjectileToFire, const FTBLaunchParams& LaunchParams, int32 DebugType);

	template<typename TypeOfSimData>
	typename TEnableIf<TB::Traits::TIsSimData<std::decay_t<TypeOfSimData>>::Value, FTBProjectileId>::Type AddSimData(TypeOfSimData& ToAdd)
	{
		if constexpr (std::is_same_v<std::decay_t<TypeOfSimData>, FTBBulletSimData>)
		{
			return AddBullet(ToAdd);
		}
		else
		{
			return AddProjectile(ToAdd);
		}
	}
	
	template<typename TypeOfSimData, typename CallbacksType>
	typename TEnableIf<TB::Traits::TIsSimData<std::decay_t<TypeOfSimData>>::Value, TArray<FTBProjectileId>>::Type AddMultiple(TArray<TypeOfSimData>& ToAdd, TOptional<CallbacksType> Delegates)
	{
		TArray<FTBProjectileId> Ids;
		Ids.AddUninitialized(ToAdd.Num());

		auto lambda = [&](int32 i)
		{
			if (Delegates.IsSet())
			{
				BindDelegates(ToAdd[i], Delegates.GetValue());
			}
			Ids[i] = AddSimData(ToAdd[i]);
		};

		ParallelFor(ToAdd.Num(), lambda);
		Ids.Shrink();

		return Ids;
	}

	template<typename TypeOfSimData>
	typename TEnableIf<TB::Traits::TIsSimData<std::decay_t<TypeOfSimData>>::Value, void>::Type FireMultiple(TArray<TypeOfSimData> ProjectilesToFire, const TArray<FTBLaunchData>& LaunchData)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UTerminalBallisticsSubsystem::FireMultiple);
		auto lambda = [&](int32 i)
		{
			TypeOfSimData* ProjectileToFire = &ProjectilesToFire[i];
			const FTBLaunchData CurrentLaunchData = LaunchData[i];
			_FireProjectileImpl(ProjectileToFire, CurrentLaunchData.LaunchParams, CurrentLaunchData.DebugType);
		};

		ParallelFor(FMath::Min(ProjectilesToFire.Num(), LaunchData.Num()), lambda);
	}

protected:
#pragma region Threading
	FTBProjectileThread* ProjectileThread = nullptr;

	UPROPERTY()
	UTBProjectileThreadQueue* ProjectileThreadQueue = nullptr;

	void InitProjectileThread();
	void ShutdownProjectileThread();

private:
	void GetResultsFromProjectileThread();

	/**
	* Function to do any cleanup or processing on a task result before it is destroyed.
	*/
	template<typename ImpactStruct> 
	void ProcessTaskResult(TTBBaseProjectileTaskResult<ImpactStruct> TaskResult); 


	mutable FRWLock ActiveBulletsLock;
	mutable FRWLock InactiveBulletsLock;
	mutable FRWLock ActiveProjectilesLock;
	mutable FRWLock InactiveProjectilesLock;

	mutable FRWLock BulletsToAddLock;
	mutable FRWLock ProjectilesToAddLock;

	mutable FCriticalSection ActiveBulletsAccessLock;
	mutable FCriticalSection InactiveBulletsAccessLock;
	mutable FCriticalSection ActiveProjectilesAccessLock;
	mutable FCriticalSection InactiveProjectilesAccessLock;

	mutable FCriticalSection AddBulletMutex;
	mutable FCriticalSection AddProjectileMutex;
#pragma endregion
};


class TERMINALBALLISTICS_API FTBGetSubsystemHelper : public FNoncopyable
{
public:
	static UTerminalBallisticsSubsystem* GetTBSubsystem(const UObject* WorldContextObject);
private:
	friend UTerminalBallisticsSubsystem;
	static UTerminalBallisticsSubsystem* TBSubsystem;
};
