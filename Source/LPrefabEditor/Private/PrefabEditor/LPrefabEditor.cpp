// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditor.h"
#include "LPrefabEditorModule.h"
#include "LPrefabEditorViewport.h"
#include "LPrefabEditorScene.h"
#include "LPrefabEditorDetails.h"
#include "LPrefabEditorOutliner.h"
#include "LPrefabRawDataViewer.h"
#include "UnrealEdGlobals.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StaticMeshActor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"
#include "LPrefabEditorCommand.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LPrefabEditorTools.h"
#include "Engine/Selection.h"
#include "ToolMenus.h"
#include "LPrefabEditorUtils.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "PrefabAnimation/LPrefabSequenceEditor.h"
#include "PrefabSystem/LPrefabHelperObject.h"
#include "PrefabSystem/LPrefabManager.h"
#include "LPrefabUtils.h"
#include "SceneOutliner/LPrefabNativeSceneOutlinerExtension.h"

#define LOCTEXT_NAMESPACE "LPrefabEditor"

UE_DISABLE_OPTIMIZATION

const FName PrefabEditorAppName = FName(TEXT("LPrefabEditorApp"));

TArray<FLPrefabEditor*> FLPrefabEditor::LPrefabEditorInstanceCollection;

struct FLPrefabEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName OutlinerID;
	static const FName PrefabRawDataViewerID;
};

const FName FLPrefabEditorTabs::DetailsID(TEXT("Details"));
const FName FLPrefabEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLPrefabEditorTabs::OutlinerID(TEXT("Outliner"));
const FName FLPrefabEditorTabs::PrefabRawDataViewerID(TEXT("PrefabRawDataViewer"));

FName GetPrefabWorldName()
{
	static uint32 NameSuffix = 0;
	return FName(*FString::Printf(TEXT("PrefabEditorWorld_%d"), NameSuffix++));
}
FLPrefabEditor::FLPrefabEditor()
	:PreviewScene(FLPrefabEditorScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(false).SetEditor(true).SetName(GetPrefabWorldName()))
{
	PrefabHelperObject = NewObject<ULPrefabHelperObject>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Transactional);
	LPrefabEditorInstanceCollection.Add(this);
}
FLPrefabEditor::~FLPrefabEditor()
{
	PrefabHelperObject->ConditionalBeginDestroy();
	PrefabHelperObject = nullptr;

	LPrefabEditorInstanceCollection.Remove(this);

	GEditor->SelectNone(true, true);

	ULPrefabManagerObject::MarkBroadcastLevelActorListChanged();
	FLPrefabEditorModule::Get().GetNativeSceneOutlinerExtension()->Restore();
}

FLPrefabEditor* FLPrefabEditor::GetEditorForPrefabIfValid(ULPrefab* InPrefab)
{
	for (auto Instance : LPrefabEditorInstanceCollection)
	{
		if (Instance->PrefabBeingEdited == InPrefab)
		{
			return Instance;
		}
	}
	return nullptr;
}

ULPrefabHelperObject* FLPrefabEditor::GetEditorPrefabHelperObjectForActor(AActor* InActor)
{
	for (auto Instance : LPrefabEditorInstanceCollection)
	{
		if (InActor->GetWorld() == Instance->GetWorld())
		{
			return Instance->PrefabHelperObject;
		}
	}
	return nullptr;
}

bool FLPrefabEditor::WorldIsPrefabEditor(UWorld* InWorld)
{
	for (auto Instance : LPrefabEditorInstanceCollection)
	{
		if (Instance->GetWorld() == InWorld)
		{
			return true;
		}
	}
	return false;
}

bool FLPrefabEditor::ActorIsRootAgent(AActor* InActor)
{
	for (auto Instance : LPrefabEditorInstanceCollection)
	{
		if (InActor == Instance->GetPreviewScene().GetRootAgentActor())
		{
			return true;
		}
	}
	return false;
}

void FLPrefabEditor::IterateAllPrefabEditor(const TFunction<void(FLPrefabEditor*)>& InFunction)
{
	for (auto Instance : LPrefabEditorInstanceCollection)
	{
		InFunction(Instance);
	}
}

bool FLPrefabEditor::RefreshOnSubPrefabDirty(ULPrefab* InSubPrefab)
{
	return PrefabHelperObject->RefreshOnSubPrefabDirty(InSubPrefab);
}

bool FLPrefabEditor::GetSelectedObjectsBounds(FBoxSphereBounds& OutResult)
{
	USelection* Selection = GEditor->GetSelectedActors();
	TArray<AActor*> SelectedActors;
	for (int i = 0; i < Selection->Num(); i++)
	{
		if (auto Actor = Cast<AActor>(Selection->GetSelectedObject(i)))
		{
			if (Actor->GetWorld() == this->GetWorld())//only concern actors belongs to this prefab
			{
				SelectedActors.Add(Actor);
			}
		}
	}

	FBoxSphereBounds Bounds = FBoxSphereBounds(EForceInit::ForceInitToZero);
	bool IsFirstBounds = true;
	for (auto& Actor : SelectedActors)
	{
		auto Box = Actor->GetComponentsBoundingBox();
		if (IsFirstBounds)
		{
			IsFirstBounds = false;
			Bounds = Box;
		}
		else
		{
			Bounds = Bounds + Box;
		}
	}
	OutResult = Bounds;
	return IsFirstBounds == false;
}

FBoxSphereBounds FLPrefabEditor::GetAllObjectsBounds()
{
	FBoxSphereBounds Bounds;
	bool IsFirstBounds = true;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		FBox Box; bool bIsValidBox = false;
		if (auto SceneComp = Cast<USceneComponent>(KeyValue.Value))
		{
			if (SceneComp->IsRegistered())
			{
				if (!SceneComp->GetOwner()->IsHiddenEd() && SceneComp->IsVisible())
				{
					Box = SceneComp->Bounds.GetBox();
					bIsValidBox = true;
				}
			}
		}
		if (bIsValidBox)
		{
			if (IsFirstBounds)
			{
				IsFirstBounds = false;
				Bounds = Box;
			}
			else
			{
				Bounds = Bounds + Box;
			}
		}
	}
	return Bounds;
}

bool FLPrefabEditor::ActorBelongsToSubPrefab(AActor* InActor)
{
	return PrefabHelperObject->IsActorBelongsToSubPrefab(InActor);
}

bool FLPrefabEditor::ActorIsSubPrefabRoot(AActor* InSubPrefabRootActor)
{
	return PrefabHelperObject->SubPrefabMap.Contains(InSubPrefabRootActor);
}

FLSubPrefabData FLPrefabEditor::GetSubPrefabDataForActor(AActor* InSubPrefabActor)
{
	return PrefabHelperObject->GetSubPrefabData(InSubPrefabActor);
}

void FLPrefabEditor::OpenSubPrefab(AActor* InSubPrefabActor)
{
	if (auto SubPrefabAsset = PrefabHelperObject->GetSubPrefabAsset(InSubPrefabActor))
	{
		auto PrefabEditor = FLPrefabEditor::GetEditorForPrefabIfValid(SubPrefabAsset);
		if (!PrefabEditor)
		{
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditorSubsystem->OpenEditorForAsset(SubPrefabAsset);
		}
	}
}
void FLPrefabEditor::SelectSubPrefab(AActor* InSubPrefabActor)
{
	if (auto SubPrefabAsset = PrefabHelperObject->GetSubPrefabAsset(InSubPrefabActor))
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(SubPrefabAsset);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
}

bool FLPrefabEditor::GetAnythingDirty()const 
{ 
	return PrefabHelperObject->GetAnythingDirty();
}

void FLPrefabEditor::CloseWithoutCheckDataDirty()
{
	PrefabHelperObject->SetNothingDirty();
	this->CloseWindow();
}

bool FLPrefabEditor::OnRequestClose()
{
	if (GetAnythingDirty())
	{
		auto WarningMsg = LOCTEXT("LoseDataOnCloseEditor", "Are you sure you want to close prefab editor window? Property will lose if not hit Apply!");
		auto Result = FMessageDialog::Open(EAppMsgType::YesNo, WarningMsg);
		if (Result == EAppReturnType::Yes)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void FLPrefabEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_LPrefabEditor", "LPrefab Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FLPrefabEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FLPrefabEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FLPrefabEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FLPrefabEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FLPrefabEditorTabs::OutlinerID, FOnSpawnTab::CreateSP(this, &FLPrefabEditor::SpawnTab_Outliner))
		.SetDisplayName(LOCTEXT("OutlinerTabLabel", "Outliner"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Outliner"));

	InTabManager->RegisterTabSpawner(FLPrefabEditorTabs::PrefabRawDataViewerID, FOnSpawnTab::CreateSP(this, &FLPrefabEditor::SpawnTab_PrefabRawDataViewer))
		.SetDisplayName(LOCTEXT("PrefabRawDataViewerTabLabel", "PrefabRawDataViewer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		;
}
void FLPrefabEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FLPrefabEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FLPrefabEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FLPrefabEditorTabs::OutlinerID);
	InTabManager->UnregisterTabSpawner(FLPrefabEditorTabs::PrefabRawDataViewerID);
}

void FLPrefabEditor::InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost >& InitToolkitHost, ULPrefab* InPrefab)
{
	PrefabBeingEdited = InPrefab;
	PrefabHelperObject->PrefabAsset = PrefabBeingEdited;
	if (PrefabBeingEdited->ReferenceClassList.Contains(nullptr))
	{
		auto MsgText = LOCTEXT("Error_PrefabMissingReferenceClass", "Prefab missing some class reference!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}
	if (PrefabBeingEdited->ReferenceAssetList.Contains(nullptr))
	{
		auto MsgText = LOCTEXT("Error_PrefabMissingReferenceAsset", "Prefab missing some asset reference!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}

	FLPrefabEditorCommand::Register();

	PrefabHelperObject->LoadPrefab(GetPreviewScene().GetWorld(), GetPreviewScene().GetParentComponentForPrefab(PrefabBeingEdited));
	if (!IsValid(PrefabHelperObject->LoadedRootActor))
	{
		auto MsgText = LOCTEXT("Error_LoadPrefabFail", "Load prefab fail! Nothing loaded!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}
	PrefabHelperObject->RootAgentActorForPrefabEditor = GetPreviewScene().GetRootAgentActor();
	PrefabHelperObject->MarkAsManagerObject();

	TSharedPtr<FLPrefabEditor> PrefabEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SLPrefabEditorViewport, PrefabEditorPtr, PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode);
	
	DetailsPtr = SNew(SLPrefabEditorDetails, PrefabEditorPtr);

	PrefabRawDataViewer = SNew(SLPrefabRawDataViewer, PrefabEditorPtr, PrefabBeingEdited);

	auto UnexpendActorGuidSet = PrefabBeingEdited->PrefabDataForPrefabEditor.UnexpendActorSet;
	TSet<AActor*> UnexpendActorSet;
	for (auto& ItemActorGuid : UnexpendActorGuidSet)
	{
		if (auto ObjectPtr = PrefabHelperObject->MapGuidToObject.Find(ItemActorGuid))
		{
			if (auto Actor = Cast<AActor>(*ObjectPtr))
			{
				UnexpendActorSet.Add(Actor);
			}
		}
	}
	OutlinerPtr = MakeShared<FLPrefabEditorOutliner>();
	OutlinerPtr->ActorFilter = FOnShouldFilterActor::CreateRaw(this, &FLPrefabEditor::IsFilteredActor);
	OutlinerPtr->OnActorPickedDelegate = FOnActorPicked::CreateRaw(this, &FLPrefabEditor::OnOutlinerPickedChanged);
	OutlinerPtr->OnActorDoubleClickDelegate = FOnActorPicked::CreateRaw(this, &FLPrefabEditor::OnOutlinerActorDoubleClick);
	OutlinerPtr->InitOutliner(GetPreviewScene().GetWorld(), PrefabEditorPtr, UnexpendActorSet);

	BindCommands();
	ExtendToolbar();

	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_LPrefabEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FLPrefabEditorTabs::OutlinerID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(FLPrefabEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FLPrefabEditorTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, PrefabEditorAppName, StandaloneDefaultLayout, true, true, PrefabBeingEdited);

	// After opening a prefab, broadcast event to LPrefabSequencerEditor
	LPrefabEditorTools::OnEditingPrefabChanged.Broadcast(GetPreviewScene().GetRootAgentActor());
}

TArray<AActor*> FLPrefabEditor::GetAllActors()
{
	TArray<AActor*> AllActors;
	if (PrefabHelperObject->LoadedRootActor != nullptr)
	{
		LPrefabUtils::CollectChildrenActors(PrefabHelperObject->LoadedRootActor, AllActors, true);
	}
	return AllActors;
}

void FLPrefabEditor::GetInitialViewLocationAndRotation(FVector& OutLocation, FRotator& OutRotation, FVector& OutOrbitLocation)
{
	auto& PrefabEditorData = PrefabBeingEdited->PrefabDataForPrefabEditor;
	auto SceneBounds = this->GetAllObjectsBounds();
	if (PrefabEditorData.ViewLocation == FVector::ZeroVector && PrefabEditorData.ViewRotation == FRotator::ZeroRotator)
	{
		OutLocation = FVector(-SceneBounds.SphereRadius * 1.2f, SceneBounds.Origin.Y, SceneBounds.Origin.Z);
		OutRotation = FRotator::ZeroRotator;
	}
	else
	{
		OutLocation = PrefabEditorData.ViewLocation;
		OutRotation = PrefabEditorData.ViewRotation;
	}
	if (PrefabEditorData.ViewOrbitLocation == FVector::ZeroVector)
	{
		OutOrbitLocation = SceneBounds.Origin;
	}
	else
	{
		OutOrbitLocation = PrefabEditorData.ViewOrbitLocation;
	}
}

void FLPrefabEditor::DeleteActors(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray)
{
	for (auto Item : InSelectedActorArray)
	{
		if (Item == PrefabHelperObject->LoadedRootActor)
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
		if (Item == GetPreviewScene().GetRootAgentActor())
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root agent actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(Item.Get()) && !PrefabHelperObject->SubPrefabMap.Contains(Item.Get()))
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy sub prefab's actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
	}

	TArray<AActor*> SelectedActorArray;
	for (auto Item : InSelectedActorArray)
	{
		if (Item.IsValid())
		{
			SelectedActorArray.Add(Item.Get());
		}
	}
	LPrefabEditorTools::DeleteActors_Impl(SelectedActorArray);
}

void FLPrefabEditor::ApplyPrefab()
{
	OnApply();
}

void FLPrefabEditor::SaveAsset_Execute()
{
	if (CheckBeforeSaveAsset())
	{
		if (GetAnythingDirty())
		{
			OnApply();//apply change
		}
		FAssetEditorToolkit::SaveAsset_Execute();//save asset
	}
}
void FLPrefabEditor::OnApply()
{
	if (CheckBeforeSaveAsset())
	{
		//save view location and rotation
		auto ViewTransform = ViewportPtr->GetViewportClient()->GetViewTransform();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewLocation = ViewTransform.GetLocation();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewRotation = ViewTransform.GetRotation();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewOrbitLocation = ViewTransform.GetLookAt();
		if (auto RootAgentActor = GetPreviewScene().GetRootAgentActor())
		{
			if (!FLPrefabEditorModule::PrefabEditor_SavePrefab.ExecuteIfBound(RootAgentActor, PrefabBeingEdited))
			{
				PrefabBeingEdited->PrefabDataForPrefabEditor.bNeedCanvas = false;
			}
		}
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode = ViewportPtr->GetViewportClient()->GetViewMode();
		TSet<FGuid> UnexpendActorGuidArray;
		TArray<AActor*> UnexpendActorArray;
		LPrefabUtils::CollectChildrenActors(PrefabHelperObject->LoadedRootActor, UnexpendActorArray, true);
		OutlinerPtr->GetUnexpendActor(UnexpendActorArray);
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (UnexpendActorArray.Contains(KeyValue.Value))
			{
				UnexpendActorGuidArray.Add(KeyValue.Key);
			}
		}
		PrefabBeingEdited->PrefabDataForPrefabEditor.UnexpendActorSet = UnexpendActorGuidArray;

		//refresh parameter, remove invalid
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			KeyValue.Value.CheckParameters();
		}

		LPrefabEditorTools::OnBeforeApplyPrefab.Broadcast(PrefabHelperObject);
		PrefabHelperObject->SavePrefab();
		LPrefabEditorTools::RefreshLevelLoadedPrefab(PrefabHelperObject->PrefabAsset);
		LPrefabEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
	}
}

void FLPrefabEditor::OnOpenRawDataViewerPanel()
{
	this->InvokeTab(FLPrefabEditorTabs::PrefabRawDataViewerID);
}
void FLPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(PrefabHelperObject);
}

void FLPrefabEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrefabBeingEdited);
	Collector.AddReferencedObject(PrefabHelperObject);
}

bool FLPrefabEditor::CheckBeforeSaveAsset()
{
	auto RootUIAgentActor = GetPreviewScene().GetRootAgentActor();
	//All actor should attach to prefab's root actor
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (AActor* ItemActor = *ActorItr)
		{
			if (ItemActor == PrefabHelperObject->LoadedRootActor)continue;
			if (ItemActor == RootUIAgentActor)continue;
			if (GetPreviewScene().IsWorldDefaultActor(ItemActor))continue;
			if (!ItemActor->IsAttachedTo(PrefabHelperObject->LoadedRootActor))
			{
				auto MsgText = LOCTEXT("Error_AllActor", "All prefab's actors must attach to prefab's root actor!");
				FMessageDialog::Open(EAppMsgType::Ok, MsgText);
				return false;
			}
		}
	}

	return true;
}

FLPrefabEditorScene& FLPrefabEditor::GetPreviewScene()
{ 
	return PreviewScene;
}

UWorld* FLPrefabEditor::GetWorld()
{
	return PreviewScene.GetWorld();
}

void FLPrefabEditor::BindCommands()
{
	const FLPrefabEditorCommand& PrefabEditorCommands = FLPrefabEditorCommand::Get();
	ToolkitCommands->MapAction(
		PrefabEditorCommands.Apply,
		FExecuteAction::CreateSP(this, &FLPrefabEditor::OnApply),
		FCanExecuteAction(),
		FIsActionChecked()
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.RawDataViewer,
		FExecuteAction::CreateSP(this, &FLPrefabEditor::OnOpenRawDataViewerPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.OpenPrefabHelperObject,
		FExecuteAction::CreateSP(this, &FLPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);

	ToolkitCommands->MapAction(
		PrefabEditorCommands.CopyActor,
		FExecuteAction::CreateStatic(&LPrefabEditorTools::CopySelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanCopyActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanCopyActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.PasteActor,
		FExecuteAction::CreateStatic(&LPrefabEditorTools::CutSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanCutActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanCutActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.PasteActor,
		FExecuteAction::CreateStatic(&LPrefabEditorTools::PasteSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanPasteActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanPasteActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.DuplicateActor,
		FExecuteAction::CreateStatic(&LPrefabEditorTools::DuplicateSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanDuplicateActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanDuplicateActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.DestroyActor,
		FExecuteAction::CreateStatic(&LPrefabEditorTools::DeleteSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanDeleteActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanDeleteActor)
	);
}
void FLPrefabEditor::ExtendToolbar()
{
	const FName MenuName = GetToolMenuToolbarName();
	if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
	{
		UToolMenus::Get()->RegisterMenu(MenuName, "AssetEditor.DefaultToolBar", EMultiBoxType::ToolBar);
	}

	UToolMenu* ToolBar = UToolMenus::Get()->FindMenu(MenuName);

	FToolMenuInsert InsertAfterAssetSection("Asset", EToolMenuInsertType::After);
	{
		auto ApplyButtonMenuEntry = FToolMenuEntry::InitToolBarButton(FLPrefabEditorCommand::Get().Apply
			, LOCTEXT("Apply", "Apply")
			, TAttribute<FText>(this, &FLPrefabEditor::GetApplyButtonStatusTooltip)
			, TAttribute<FSlateIcon>(this, &FLPrefabEditor::GetApplyButtonStatusImage));

		FToolMenuSection& Section = ToolBar->AddSection("LPrefabCommands", TAttribute<FText>(), InsertAfterAssetSection);
		Section.AddEntry(ApplyButtonMenuEntry);
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLPrefabEditorCommand::Get().RawDataViewer));
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLPrefabEditorCommand::Get().OpenPrefabHelperObject));
	}
}

FText FLPrefabEditor::GetApplyButtonStatusTooltip()const
{
	return GetAnythingDirty() ? LOCTEXT("Apply_Tooltip", "Dirty, need to apply") : LOCTEXT("Apply_Tooltip", "Good to go");
}
FSlateIcon FLPrefabEditor::GetApplyButtonStatusImage()const
{
	static const FName CompileStatusBackground("Blueprint.CompileStatus.Background");
	static const FName CompileStatusUnknown("Blueprint.CompileStatus.Overlay.Unknown");
	static const FName CompileStatusGood("Blueprint.CompileStatus.Overlay.Good");

	return FSlateIcon(FAppStyle::GetAppStyleSetName(), CompileStatusBackground, NAME_None, GetAnythingDirty() ? CompileStatusUnknown : CompileStatusGood);
}

TSharedRef<SDockTab> FLPrefabEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SOverlay)

			// The sprite editor viewport
			+SOverlay::Slot()
			[
				ViewportPtr.ToSharedRef()
			]

			// Bottom-right corner text indicating the preview nature of the sprite editor
			+SOverlay::Slot()
			.Padding(10)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.TextStyle(FAppStyle::Get(), "Graph.CornerText")
				//.Text(this, &FSpriteEditor::GetCurrentModeCornerText)
			]
		];
}
TSharedRef<SDockTab> FLPrefabEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			DetailsPtr.ToSharedRef()
		];
}
TSharedRef<SDockTab> FLPrefabEditor::SpawnTab_Outliner(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OutlinerTab_Title", "Outliner"))
		[
			OutlinerPtr->GetOutlinerWidget().ToSharedRef()
		];
}

TSharedRef<SDockTab> FLPrefabEditor::SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OverrideParameterTab_Title", "PrefabRawData"))
		[
			PrefabRawDataViewer.ToSharedRef()
		];
}

bool FLPrefabEditor::IsFilteredActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (!Actor->IsListedInSceneOutliner())
	{
		return false;
	}
	return true;
}

void FLPrefabEditor::OnOutlinerPickedChanged(AActor* Actor)
{
	CurrentSelectedActor = Actor;
}

void FLPrefabEditor::OnOutlinerActorDoubleClick(AActor* Actor)
{
	// Create a bounding volume of all of the selected actors.
	FBox BoundingBox(ForceInit);

	TArray<AActor*> Actors;
	Actors.Add(Actor);

	for (int32 ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++)
	{
		AActor* TempActor = Actors[ActorIdx];

		if (TempActor)
		{
			TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(TempActor);

			for (int32 ComponentIndex = 0; ComponentIndex < PrimitiveComponents.Num(); ++ComponentIndex)
			{
				UPrimitiveComponent* PrimitiveComponent = PrimitiveComponents[ComponentIndex];

				if (PrimitiveComponent->IsRegistered())
				{
					// Some components can have huge bounds but are not visible.  Ignore these components unless it is the only component on the actor 
					const bool bIgnore = PrimitiveComponents.Num() > 1 && PrimitiveComponent->IgnoreBoundsForEditorFocus();

					if (!bIgnore)
					{
						FBox LocalBox(ForceInit);
						if (GLevelEditorModeTools().ComputeBoundingBoxForViewportFocus(TempActor, PrimitiveComponent, LocalBox))
						{
							BoundingBox += LocalBox;
						}
						else
						{
							BoundingBox += PrimitiveComponent->Bounds.GetBox();
						}
					}
				}
			}
		}
	}

	ViewportPtr->GetViewportClient()->FocusViewportOnBox(BoundingBox);
}

FName FLPrefabEditor::GetToolkitFName() const
{
	return FName("LPrefabEditor");
}
FText FLPrefabEditor::GetBaseToolkitName() const
{
	return LOCTEXT("LPrefabEditorAppLabel", "LGUI Prefab Editor");
}
FText FLPrefabEditor::GetToolkitName() const
{
	return FText::FromString(PrefabBeingEdited->GetName());
}
FText FLPrefabEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(PrefabBeingEdited);
}
FLinearColor FLPrefabEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
FString FLPrefabEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LPrefabEditor");
}
FString FLPrefabEditor::GetDocumentationLink() const
{
	return TEXT("");
}
void FLPrefabEditor::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{

}
void FLPrefabEditor::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{

}

FReply FLPrefabEditor::TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid() && Operation->IsOfType<FAssetDragDropOp>())
	{
		TArray< FAssetData > DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(Operation);
		const int32 NumAssets = DroppedAssetData.Num();

		if (NumAssets > 0)
		{
			TArray<ULPrefab*> PrefabsToLoad;
			TArray<UClass*> PotentialActorClassesToLoad;
			TArray<UStaticMesh*> PotentialStaticMeshesToLoad;
			auto IsSupportedActorClass = [](UClass* ActorClass) {
				if (ActorClass->HasAnyClassFlags(EClassFlags::CLASS_NotPlaceable | EClassFlags::CLASS_Abstract))
					return false;
				if (!ActorClass->IsChildOf(AActor::StaticClass()))return false;
				return true;
			};
			for (int32 DroppedAssetIdx = 0; DroppedAssetIdx < NumAssets; ++DroppedAssetIdx)
			{
				const FAssetData& AssetData = DroppedAssetData[DroppedAssetIdx];

				if (!AssetData.IsAssetLoaded())
				{
					GWarn->StatusUpdate(DroppedAssetIdx, NumAssets, FText::Format(LOCTEXT("LoadingAsset", "Loading Asset {0}"), FText::FromName(AssetData.AssetName)));
				}

				UClass* AssetClass = AssetData.GetClass();
				UObject* Asset = AssetData.GetAsset();
				UBlueprint* BPClass = Cast<UBlueprint>(Asset);
				UClass* PotentialActorClass = nullptr;
				if ((BPClass != nullptr) && (BPClass->GeneratedClass != nullptr))
				{
					if (IsSupportedActorClass(BPClass->GeneratedClass))
					{
						PotentialActorClass = BPClass->GeneratedClass;
					}
				}
				else if (AssetClass->IsChildOf(UClass::StaticClass()))
				{
					UClass* AssetAsClass = CastChecked<UClass>(Asset);
					if (IsSupportedActorClass(AssetAsClass))
					{
						PotentialActorClass = AssetAsClass;
					}
				}
				if (auto PrefabAsset = Cast<ULPrefab>(Asset))
				{
					if (PrefabAsset->IsPrefabBelongsToThisSubPrefab(this->PrefabBeingEdited, true))
					{
						auto MsgText = LOCTEXT("Error_EndlessNestedPrefab", "Operation error! Target prefab have this prefab as child prefab, which will result in cyclic nested prefab!");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}
					if (this->PrefabBeingEdited == PrefabAsset)
					{
						auto MsgText = LOCTEXT("Error_SelfPrefabAsSubPrefab", "Operation error! Target prefab is same of this one, self cannot be self's child!");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}
					if (PrefabAsset->PrefabVersion <= (uint16)ELPrefabVersion::OldVersion)
					{
						auto MsgText = LOCTEXT("Error_UnsupportOldPrefabVersion", "Operation error! Target prefab's version is too old! Please make it newer: open the prefab and hit \"Save\" button.");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}

					PrefabsToLoad.Add(PrefabAsset);
				}
				else if (auto StaticMeshAsset = Cast<UStaticMesh>(Asset))
				{
					PotentialStaticMeshesToLoad.Add(StaticMeshAsset);
				}
				else if (PotentialActorClass != nullptr)
				{
					PotentialActorClassesToLoad.Add(PotentialActorClass);
				}
			}

			if (PrefabsToLoad.Num() > 0 || PotentialActorClassesToLoad.Num() > 0 || PotentialStaticMeshesToLoad.Num() > 0)
			{
				if (CurrentSelectedActor == nullptr)
				{
					auto MsgText = LOCTEXT("Error_NeedParentNode", "Please select a actor as parent actor");
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (CurrentSelectedActor == GetPreviewScene().GetRootAgentActor())
				{
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "{0} cannot be parent actor of child prefab, please choose another actor."), FText::FromString(FLPrefabEditorScene::RootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
			}
			else
			{
				return FReply::Unhandled();
			}

			GEditor->BeginTransaction(LOCTEXT("CreateFromAssetDrop_Transaction", "LGUI Create from asset drop"));
			TArray<AActor*> CreatedActorArray;
			if (PrefabsToLoad.Num() > 0)
			{
				PrefabHelperObject->SetCanNotifyAttachment(false);
				for (auto& PrefabAsset : PrefabsToLoad)
				{
					TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
					TMap<TObjectPtr<AActor>, FLSubPrefabData> SubSubPrefabMap;
					auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabWithExistingObjects(GetPreviewScene().GetWorld()
						, CurrentSelectedActor->GetRootComponent()
						, SubPrefabMapGuidToObject, SubSubPrefabMap
					);

					PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedSubPrefabRootActor, SubPrefabMapGuidToObject, {});
					CreatedActorArray.Add(LoadedSubPrefabRootActor);
				}
				OnApply();
				PrefabHelperObject->SetCanNotifyAttachment(true);

				if (OutlinerPtr.IsValid())
				{
					ULPrefabManagerObject::AddOneShotTickFunction([=] {
						for (auto& Actor : CreatedActorArray)
						{
							OutlinerPtr->UnexpandActorForDragDroppedPrefab(Actor);
						}
						OutlinerPtr->FullRefresh();
						}, 1);//delay execute, because the outliner not create actor yet
				}
			}
			if (PotentialActorClassesToLoad.Num() > 0)
			{
				for (auto& ActorClass : PotentialActorClassesToLoad)
				{
					if (auto Actor = this->GetWorld()->SpawnActor<AActor>(ActorClass, FActorSpawnParameters()))
					{
						if (auto RootComp = Actor->GetRootComponent())
						{
							RootComp->AttachToComponent(CurrentSelectedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
							CreatedActorArray.Add(Actor);
						}
						else
						{
							Actor->ConditionalBeginDestroy();
						}
					}
				}
			}
			if (PotentialStaticMeshesToLoad.Num() > 0)
			{
				for (auto& Mesh : PotentialStaticMeshesToLoad)
				{
					auto MeshActor = this->GetWorld()->SpawnActor<AStaticMeshActor>();
					MeshActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
					MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
					MeshActor->GetStaticMeshComponent()->AttachToComponent(CurrentSelectedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
					MeshActor->SetActorLabel(Mesh->GetName());
					CreatedActorArray.Add(MeshActor);
				}
			}
			if (CreatedActorArray.Num() > 0)
			{
				GEditor->SelectNone(true, true);
				for (auto& Actor : CreatedActorArray)
				{
					GEditor->SelectActor(Actor, true, true, false, true);
				}
			}
			GEditor->EndTransaction();
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

UE_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE