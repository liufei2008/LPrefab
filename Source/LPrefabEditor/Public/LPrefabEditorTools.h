// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#pragma once
class ULPrefabHelperObject;
class ULPrefab;

DECLARE_MULTICAST_DELEGATE_OneParam(FEditingPrefabChangedDelegate, AActor*);
DECLARE_MULTICAST_DELEGATE_OneParam(FBeforeApplyPrefabDelegate, ULPrefabHelperObject*);

class LPREFABEDITOR_API LPrefabEditorTools
{
private:
	static FString PrevSavePrafabFolder;
public:
	static FEditingPrefabChangedDelegate OnEditingPrefabChanged;
	static FBeforeApplyPrefabDelegate OnBeforeApplyPrefab;
	static AActor* GetFirstSelectedActor();
	static TArray<AActor*> GetSelectedActors();
	static FString GetUniqueNumetricName(const FString& InPrefix, const TArray<FString>& InExistNames);
	static TArray<AActor*> GetRootActorListFromSelection(const TArray<AActor*>& selectedActors);
	static void CreateEmptyActor();
	static void ReplaceActorByClass(UClass* ActorClass);
	static void DuplicateSelectedActors_Impl();
	static void CopySelectedActors_Impl();
	static void PasteSelectedActors_Impl();
	static void DeleteSelectedActors_Impl();
	static void CutSelectedActors_Impl();
	static void ToggleSelectedActorsSpatiallyLoaded_Impl();
	static ECheckBoxState GetActorSpatiallyLoadedProperty();
	static void DeleteActors_Impl(const TArray<AActor*>& InActors);
	static bool CanDuplicateActor();
	static bool CanCopyActor();
	static bool CanPasteActor();
	static bool CanCutActor();
	static bool CanDeleteActor();
	static bool CanToggleActorSpatiallyLoaded();
	static void CopyComponentValues_Impl();
	static void PasteComponentValues_Impl();
	static UWorld* GetWorldFromSelection();
	static void CreatePrefabAsset();
	static void RefreshLevelLoadedPrefab(ULPrefab* InPrefab);
	static void RefreshOpenedPrefabEditor(ULPrefab* InPrefab);
	static void RefreshOnSubPrefabChange(ULPrefab* InSubPrefab);
	static TArray<ULPrefab*> GetAllPrefabArray();
	static void UnpackPrefab();
	static void SelectPrefabAsset();
	static void OpenPrefabAsset();
	static void UpdateLevelPrefab();
	static void ToggleLevelPrefabAutoUpdate();
	static void CleanupPrefabsInWorld(UWorld* World);
	static ULPrefabHelperObject* GetPrefabHelperObject_WhichManageThisActor(AActor* InActor);
	static bool IsActorCompatibleWithLGUIToolsMenu(AActor* InActor);

	static TMap<FString, TWeakObjectPtr<class ULPrefab>> CopiedActorPrefabMap;//map ActorLabel to prefab
	static TWeakObjectPtr<class UActorComponent> CopiedComponent;
	static bool HaveValidCopiedActors();
	static bool HaveValidCopiedComponent();

	static void MakeCurrentLevel(AActor* InActor);

	static void ForceGC();
};
