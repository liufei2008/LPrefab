// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LPrefabEditorScene.h"
#pragma once

class ULPrefab;
class SLPrefabEditorViewport;
class SLPrefabEditorDetails;
class FLPrefabEditorOutliner;
class SLPrefabOverrideParameterEditor;
class SLPrefabRawDataViewer;
class AActor;
class FLPrefabEditorScene;
class ULPrefabHelperObject;
class ULPrefabOverrideParameterHelperObject;
class ULPrefabOverrideHelperObject;
struct FLSubPrefabData;

/**
 * 
 */
class LPREFABEDITOR_API FLPrefabEditor : public FAssetEditorToolkit
	, public FGCObject
{
public:
	FLPrefabEditor();
	~FLPrefabEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
public:
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void OnToolkitHostingStarted(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void SaveAsset_Execute()override;
private:
	virtual bool OnRequestClose()override;
	// End of FAssetEditorToolkit
public:
	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName()const { return TEXT("LPrefabEditor"); }

	bool CheckBeforeSaveAsset();

	void InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, ULPrefab* InPrefab);
	TArray<AActor*> GetAllActors();

	/** Try to handle a drag-drop operation */
	FReply TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent);

	FLPrefabEditorScene& GetPreviewScene();
	UWorld* GetWorld();
	ULPrefab* GetPrefabBeingEdited()const { return PrefabBeingEdited; }

	void DeleteActors(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray);

	static FLPrefabEditor* GetEditorForPrefabIfValid(ULPrefab* InPrefab);
	static ULPrefabHelperObject* GetEditorPrefabHelperObjectForActor(AActor* InActor);
	static bool WorldIsPrefabEditor(UWorld* InWorld);
	static bool ActorIsRootAgent(AActor* InActor);
	static void IterateAllPrefabEditor(const TFunction<void(FLPrefabEditor*)>& InFunction);
	bool RefreshOnSubPrefabDirty(ULPrefab* InSubPrefab);

	bool GetSelectedObjectsBounds(FBoxSphereBounds& OutResult);
	FBoxSphereBounds GetAllObjectsBounds();
	bool ActorBelongsToSubPrefab(AActor* InSubPrefabActor);
	bool ActorIsSubPrefabRoot(AActor* InSubPrefabRootActor);
	FLSubPrefabData GetSubPrefabDataForActor(AActor* InSubPrefabActor);
	void GetInitialViewLocationAndRotation(FVector& OutLocation, FRotator& OutRotation, FVector& OutOrbitLocation);

	void OpenSubPrefab(AActor* InSubPrefabActor);
	void SelectSubPrefab(AActor* InSubPrefabActor);
	bool GetAnythingDirty()const;
	void CloseWithoutCheckDataDirty();

	ULPrefabHelperObject* GetPrefabManagerObject()const { return PrefabHelperObject; }
	void ApplyPrefab();
private:
	TObjectPtr<ULPrefab> PrefabBeingEdited = nullptr;
	TObjectPtr<ULPrefabHelperObject> PrefabHelperObject = nullptr;
	static TArray<FLPrefabEditor*> LPrefabEditorInstanceCollection;

	TSharedPtr<SLPrefabEditorViewport> ViewportPtr;
	TSharedPtr<SLPrefabEditorDetails> DetailsPtr;
	TSharedPtr<FLPrefabEditorOutliner> OutlinerPtr;
	TSharedPtr<SLPrefabRawDataViewer> PrefabRawDataViewer;

	TWeakObjectPtr<AActor> CurrentSelectedActor;

	FLPrefabEditorScene PreviewScene;
private:

	void BindCommands();
	//void ExtendMenu();
	void ExtendToolbar();

	FText GetApplyButtonStatusTooltip()const;
	FSlateIcon GetApplyButtonStatusImage()const;

	void OnApply();
	void OnOpenRawDataViewerPanel();
	void OnOpenPrefabHelperObjectDetailsPanel();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Outliner(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args);

	bool IsFilteredActor(const AActor* Actor);
	void OnOutlinerPickedChanged(AActor* Actor);
	void OnOutlinerActorDoubleClick(AActor* Actor);
};
