// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"

class FToolBarBuilder;
class FMenuBuilder;
class FLPrefabNativeSceneOutlinerExtension;
DECLARE_LOG_CATEGORY_EXTERN(LPrefabEditor, Log, All);

class LPREFABEDITOR_API FLPrefabEditorModule : public IModuleInterface, public FGCObject
{
public:

	static const FName LPrefabSequenceTabName;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FLPrefabEditorModule& Get();

	TSharedRef<SWidget> MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool PreviewInViewport, bool EditorCameraControl, bool Others);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;
	void OnOutlinerSelectionChange();
	FLPrefabNativeSceneOutlinerExtension* GetNativeSceneOutlinerExtension()const;

	DECLARE_DELEGATE_TwoParams(FLPrefabEditor_OnExtendActorMenu, FMenuBuilder&, bool);
	static FLPrefabEditor_OnExtendActorMenu PrefabEditor_OnExtendActorMenu;
	DECLARE_DELEGATE_TwoParams(FLPrefabEditor_OnExtendEditorCameraControlMenu, FMenuBuilder&, bool);
	static FLPrefabEditor_OnExtendEditorCameraControlMenu PrefabEditor_OnExtendEditorCameraControlMenu;
	DECLARE_DELEGATE_OneParam(FLPrefabEditor_OnExtendOthersMenu, FMenuBuilder&);
	static FLPrefabEditor_OnExtendOthersMenu PrefabEditor_OnExtendOthersMenu;
	DECLARE_DELEGATE_OneParam(FLPrefabEditor_OnExtendActorActionMenu, FMenuBuilder&);
	static FLPrefabEditor_OnExtendActorActionMenu PrefabEditor_OnExtendActorActionMenu;
	DECLARE_DELEGATE_ThreeParams(FLPrefabEditor_OnExtendSequencerAddTrackMenu, FMenuBuilder&, UObject*, TSharedPtr<class ISequencer>);
	static FLPrefabEditor_OnExtendSequencerAddTrackMenu PrefabEditor_OnExtendSequencerAddTrackMenu;

	DECLARE_DELEGATE(FPrefabEditor_EndDuplicateActors);
	static FPrefabEditor_EndDuplicateActors PrefabEditor_EndDuplicateActors;
	DECLARE_DELEGATE(FPrefabEditor_EndPasteActors);
	static FPrefabEditor_EndPasteActors PrefabEditor_EndPasteActors;
	DECLARE_DELEGATE(FPrefabEditor_EndPasteComponentValues);
	static FPrefabEditor_EndPasteComponentValues PrefabEditor_EndPasteComponentValues;
	DECLARE_DELEGATE_TwoParams(FPrefabEditor_ReplaceActorByClass, AActor*, TFunction<void(AActor*)>&);
	static FPrefabEditor_ReplaceActorByClass PrefabEditor_ReplaceActorByClass;
	DECLARE_DELEGATE_RetVal_OneParam(bool, FPrefabEditor_ActorSupportRestoreTemporarilyHidden, AActor*);
	static FPrefabEditor_ActorSupportRestoreTemporarilyHidden PrefabEditor_ActorSupportRestoreTemporarilyHidden;
	DECLARE_DELEGATE_RetVal_ThreeParams(bool, FPrefabEditor_SortActorOnLGUIInfoColumn, AActor*, AActor*, const TFunction<bool()>&);
	static FPrefabEditor_SortActorOnLGUIInfoColumn PrefabEditor_SortActorOnLGUIInfoColumn;
	DECLARE_DELEGATE_RetVal_OneParam(bool, FPrefabEditor_IsCanvasActor, AActor*);
	static FPrefabEditor_IsCanvasActor PrefabEditor_IsCanvasActor;
	DECLARE_DELEGATE_RetVal_OneParam(int, FPrefabEditor_GetCanvasActorDrawcallCount, AActor*);
	static FPrefabEditor_GetCanvasActorDrawcallCount PrefabEditor_GetCanvasActorDrawcallCount;
	DECLARE_DELEGATE_RetVal_TwoParams(bool, FPrefabEditor_GenerateAgentActorForPrefabThumbnail, AActor*, ULPrefab*);
	static FPrefabEditor_GenerateAgentActorForPrefabThumbnail PrefabEditor_GenerateAgentActorForPrefabThumbnail;

	DECLARE_DELEGATE_FourParams(FPrefabEditorViewport_MouseClick, UWorld*, const FVector&, const FVector&, AActor*&);
	static FPrefabEditorViewport_MouseClick PrefabEditorViewport_MouseClick;
	DECLARE_DELEGATE_OneParam(FPrefabEditorViewport_MouseMove, UWorld*);
	static FPrefabEditorViewport_MouseMove PrefabEditorViewport_MouseMove;
	DECLARE_DELEGATE_FourParams(FPrefabEditor_CreateRootAgent, UWorld*, UClass*, ULPrefab*, AActor*&);
	static FPrefabEditor_CreateRootAgent PrefabEditor_CreateRootAgent;
	DECLARE_DELEGATE_TwoParams(FPrefabEditor_SavePrefab, AActor*, ULPrefab*);
	static FPrefabEditor_SavePrefab PrefabEditor_SavePrefab;
public:
	static bool CanCreateActor();
private:

	void CreateCommonActorSubMenu(FMenuBuilder& MenuBuilder);
	bool CanUnpackActorForPrefab();
	bool CanBrowsePrefab();
	bool CanUpdateLevelPrefab();
	ECheckBoxState GetAutoUpdateLevelPrefab()const;
	bool CanCreatePrefab();
	bool CanCheckPrefabOverrideParameter()const;
	bool CanReplaceActor();

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);

	void ToggleLGUIColumnInfo();
	bool IsLGUIColumnInfoChecked();

	void ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner);
	TWeakObjectPtr<class ULPrefabHelperObject> CurrentPrefabHelperObject;
private:
	TSharedRef<SDockTab> HandleSpawnLPrefabSequenceTab(const FSpawnTabArgs& SpawnTabArgs);
	bool bActiveViewportAsPreview = false;
	FLPrefabNativeSceneOutlinerExtension* NativeSceneOutlinerExtension = nullptr;
	TSharedPtr<class SLPrefabOverrideDataViewer> PrefabOverrideDataViewer = nullptr;
	void CheckPrefabOverrideDataViewerEntry();

	
	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	FName LPrefabSequenceComponentName;
	static void OnInitializeSequence(class ULPrefabSequence* Sequence);
	TObjectPtr<class USequencerSettings> LPrefabSequencerSettings = nullptr;

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
};