// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "Subsystems/WorldSubsystem.h"
#include "LPrefabManager.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FLPrefabEditorTickMulticastDelegate, float);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLPrefabEditorManagerOnComponentCreateDelete, bool, UActorComponent*, AActor*);

class ULPrefab;
class ULPrefabHelperObject;

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LPREFAB_API ULPrefabManagerObject :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static ULPrefabManagerObject* Instance;
	ULPrefabManagerObject();
	virtual void BeginDestroy()override;
public:
	//begin TickableEditorObject interface
	virtual void Tick(float DeltaTime)override;
	virtual bool IsTickable() const { return Instance == this; }
	virtual bool IsTickableInEditor()const { return Instance == this; }
	virtual TStatId GetStatId() const override;
	virtual bool IsEditorOnly()const override { return true; }
	//end TickableEditorObject interface
#if WITH_EDITORONLY_DATA
private:
	int32 PrevEditorViewportCount = 0;
	FLPrefabEditorTickMulticastDelegate EditorTick;
	UPROPERTY()UWorld* PreviewWorldForPrefabPackage = nullptr;
	bool bIsBlueprintCompiling = false;
	class FLPrefabObjectCreateDeleteListener* ObjectCreateDeleteListener = nullptr;
private:
	friend class LPrefabEditorTools;
	bool bShouldBroadcastLevelActorListChanged = false;
	bool bIsProcessingDelete = false;
#endif
#if WITH_EDITOR
private:
	TArray<TTuple<int, TFunction<void()>>> OneShotFunctionsToExecuteInTick;
	FLPrefabEditorManagerOnComponentCreateDelete OnComponentCreateDeleteEvent;
public:
	static void AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount = 0);
	static FDelegateHandle RegisterEditorTickFunction(const TFunction<void(float)>& InFunction);
	static void UnregisterEditorTickFunction(const FDelegateHandle& InDelegateHandle);
	static FLPrefabEditorManagerOnComponentCreateDelete& OnComponentCreateDelete() { InitCheck(); return Instance->OnComponentCreateDeleteEvent; }
private:
	static bool InitCheck();
public:
	static ULPrefabManagerObject* GetInstance(bool CreateIfNotValid = false);
	static bool IsSelected(AActor* InObject);
	static bool AnySelectedIsChildOf(AActor* InObject);
	static UWorld* GetPreviewWorldForPrefabPackage();
	static bool GetIsBlueprintCompiling();
	static bool GetIsProcessingDelete();
private:
	FDelegateHandle OnBlueprintPreCompileDelegateHandle;
	FDelegateHandle OnBlueprintCompiledDelegateHandle;
	void OnBlueprintPreCompile(UBlueprint* InBlueprint);
	void OnBlueprintCompiled();
public:
	static void MarkBroadcastLevelActorListChanged();
private:
	FDelegateHandle OnAssetReimportDelegateHandle;
	void OnAssetReimport(UObject* asset);
	FDelegateHandle OnMapOpenedDelegateHandle;
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	FDelegateHandle OnPackageReloadedDelegateHandle;
	void OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event);

public:
	DECLARE_DELEGATE_OneParam(FSerialize_SortChildrenActors, TArray<AActor*>&);
	static FSerialize_SortChildrenActors Serialize_SortChildrenActors;
	DECLARE_DELEGATE_OneParam(FDeserialize_Components, const TArray<UActorComponent*>&);
	static FDeserialize_Components Deserialize_ProcessComponentsBeforeRerunConstructionScript;

	DECLARE_DELEGATE(FPrefabHelperObject_Refresh);
	static FPrefabHelperObject_Refresh PrefabHelperObject_Refresh;
	DECLARE_DELEGATE_ThreeParams(FPrefabHelperObject_ReplaceObjectPropertyForApplyOrRevert, ULPrefabHelperObject*, UObject*, FName&);
	static FPrefabHelperObject_ReplaceObjectPropertyForApplyOrRevert PrefabHelperObject_ReplaceObjectPropertyForApplyOrRevert;
	DECLARE_DELEGATE_ThreeParams(FPrefabHelperObject_AfterObjectPropertyApplyOrRevert, ULPrefabHelperObject*, UObject*, FName);
	static FPrefabHelperObject_AfterObjectPropertyApplyOrRevert PrefabHelperObject_AfterObjectPropertyApplyOrRevert;
	DECLARE_DELEGATE_TwoParams(FPrefabHelperObject_AfterMakePrefabAsSubPrefab, ULPrefabHelperObject*, AActor*);
	static FPrefabHelperObject_AfterMakePrefabAsSubPrefab PrefabHelperObject_AfterMakePrefabAsSubPrefab;
	DECLARE_DELEGATE_ThreeParams(FPrefabHelperObject_AfterCollectPropertyToOverride, ULPrefabHelperObject*, UObject*, FName);
	static FPrefabHelperObject_AfterCollectPropertyToOverride PrefabHelperObject_AfterCollectPropertyToOverride;
	DECLARE_DELEGATE_ThreeParams(FPrefabHelperObject_CopyRootObjectParentAnchorData, ULPrefabHelperObject*, UObject*, UObject*);
	static FPrefabHelperObject_CopyRootObjectParentAnchorData PrefabHelperObject_CopyRootObjectParentAnchorData;
#endif
};

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LPREFAB_API ULPrefabWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection)override {};
	virtual void Deinitialize()override {};

	static ULPrefabWorldSubsystem* GetInstance(UWorld* World);
	DECLARE_EVENT_OneParam(ULPrefabWorldSubsystem, FDeserializeSession, const FGuid&);
	FDeserializeSession OnBeginDeserializeSession;
	FDeserializeSession OnEndDeserializeSession;
private:
	/** Map actor to prefab-deserialize-settion-id */
	UPROPERTY(VisibleAnywhere, Category = "LPrefab")
	TMap<AActor*, FGuid> AllActors_PrefabSystemProcessing;
public:
	void BeginPrefabSystemProcessingActor(const FGuid& InSessionId);
	void EndPrefabSystemProcessingActor(const FGuid& InSessionId);
	void AddActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId);
	void RemoveActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId);
	FGuid GetPrefabSystemSessionIdForActor(AActor* InActor);

	/**
	 * Tell if PrefabSystem is deserializing the actor, can use this function in BeginPlay, if this return true then means properties are not ready yet, then you should use ILPrefabInterface and implement Awake instead of BeginPlay.
	 * PrefabSystem is deserializing actor during LoadPrefab or DuplicateActor.
	 * (This static version function is for Blueprint easily use).
	 */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "LPrefab")
		static bool IsLPrefabSystemProcessingActor(AActor* InActor);
	/**
	 * Tell if PrefabSystem is deserializing the actor, can use this function in BeginPlay, if this return true then means properties are not ready yet, then you should use ILPrefabInterface and implement Awake instead of BeginPlay.
	 * PrefabSystem is deserializing actor during LoadPrefab or DuplicateActor.
	 */
	bool IsPrefabSystemProcessingActor(AActor* InActor);
};
