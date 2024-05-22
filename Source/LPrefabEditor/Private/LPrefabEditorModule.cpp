// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#include "LPrefabHeaders.h"
#include "LPrefabEditorSettings.h"

#include "ISettingsModule.h"
#include "ISettingsSection.h"

#include "SceneOutliner/LPrefabSceneOutlinerInfoColumn.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutliner/LPrefabNativeSceneOutlinerExtension.h"
#include "AssetToolsModule.h"
#include "SceneView.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "Engine/CollisionProfile.h"

#include "LPrefabEditorStyle.h"
#include "LPrefabEditorModuleCommands.h"
#include "LPrefabEditorTools.h"

#include "Thumbnail/LPrefabThumbnailRenderer.h"
#include "ContentBrowserExtensions/LPrefabContentBrowserExtensions.h"
#include "LevelEditorMenuExtensions/LPrefabLevelEditorExtensions.h"

#include "AssetTypeActions/AssetTypeActions_LPrefab.h"

#include "DetailCustomization/LPrefabCustomization.h"

#include "PrefabEditor/LPrefabOverrideDataViewer.h"
#include "Engine/Selection.h"

#include "PrefabAnimation/LPrefabSequenceComponentCustomization.h"
#include "PrefabAnimation/MovieSceneSequenceEditor_LPrefabSequence.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditorTabs.h"
#include "Framework/Docking/LayoutExtender.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "SequencerSettings.h"
#include "ISequencerModule.h"
#include "PrefabAnimation/LPrefabSequenceEditor.h"
#include "MovieSceneToolsProjectSettings.h"
#include "PrefabAnimation/LPrefabSequencerSettings.h"

#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "AssetRegistry/AssetRegistryModule.h"

const FName FLPrefabEditorModule::LPrefabSequenceTabName(TEXT("LPrefabSequenceTabName"));
FLPrefabEditorModule::FLPrefabEditor_OnExtendActorMenu FLPrefabEditorModule::PrefabEditor_OnExtendActorMenu;
FLPrefabEditorModule::FLPrefabEditor_OnExtendEditorCameraControlMenu FLPrefabEditorModule::PrefabEditor_OnExtendEditorCameraControlMenu;
FLPrefabEditorModule::FLPrefabEditor_OnExtendOthersMenu FLPrefabEditorModule::PrefabEditor_OnExtendOthersMenu;
FLPrefabEditorModule::FLPrefabEditor_OnExtendActorActionMenu FLPrefabEditorModule::PrefabEditor_OnExtendActorActionMenu;
FLPrefabEditorModule::FLPrefabEditor_OnExtendSequencerAddTrackMenu FLPrefabEditorModule::PrefabEditor_OnExtendSequencerAddTrackMenu;

FLPrefabEditorModule::FPrefabEditor_EndDuplicateActors FLPrefabEditorModule::PrefabEditor_EndDuplicateActors;
FLPrefabEditorModule::FPrefabEditor_EndPasteActors FLPrefabEditorModule::PrefabEditor_EndPasteActors;
FLPrefabEditorModule::FPrefabEditor_EndPasteComponentValues FLPrefabEditorModule::PrefabEditor_EndPasteComponentValues;
FLPrefabEditorModule::FPrefabEditor_ReplaceActorByClass FLPrefabEditorModule::PrefabEditor_ReplaceActorByClass;
FLPrefabEditorModule::FPrefabEditor_ActorSupportRestoreTemporarilyHidden FLPrefabEditorModule::PrefabEditor_ActorSupportRestoreTemporarilyHidden;
FLPrefabEditorModule::FPrefabEditor_SortActorOnLGUIInfoColumn FLPrefabEditorModule::PrefabEditor_SortActorOnLGUIInfoColumn;
FLPrefabEditorModule::FPrefabEditor_IsCanvasActor FLPrefabEditorModule::PrefabEditor_IsCanvasActor;
FLPrefabEditorModule::FPrefabEditor_GetCanvasActorDrawcallCount FLPrefabEditorModule::PrefabEditor_GetCanvasActorDrawcallCount;
FLPrefabEditorModule::FPrefabEditor_GenerateAgentActorForPrefabThumbnail FLPrefabEditorModule::PrefabEditor_GenerateAgentActorForPrefabThumbnail;

FLPrefabEditorModule::FPrefabEditorViewport_MouseClick FLPrefabEditorModule::PrefabEditorViewport_MouseClick;
FLPrefabEditorModule::FPrefabEditorViewport_MouseMove FLPrefabEditorModule::PrefabEditorViewport_MouseMove;
FLPrefabEditorModule::FPrefabEditor_CreateRootAgent FLPrefabEditorModule::PrefabEditor_CreateRootAgent;
FLPrefabEditorModule::FPrefabEditor_SavePrefab FLPrefabEditorModule::PrefabEditor_SavePrefab;

#define LOCTEXT_NAMESPACE "FLPrefabEditorModule"
DEFINE_LOG_CATEGORY(LPrefabEditor);

void FLPrefabEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLPrefabEditorStyle::Initialize();
	FLPrefabEditorStyle::ReloadTextures();

	OnInitializeSequenceHandle = ULPrefabSequence::OnInitializeSequence().AddStatic(FLPrefabEditorModule::OnInitializeSequence);

	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequenceEditorHandle = SequencerModule.RegisterSequenceEditor(ULPrefabSequence::StaticClass(), MakeUnique<FMovieSceneSequenceEditor_LPrefabSequence>());

	FLPrefabEditorModuleCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	//Editor tools
	{
		auto editorCommand = FLPrefabEditorModuleCommands::Get();

		//actor action
		PluginCommands->MapAction(
			editorCommand.CopyActor,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::CopySelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanCopyActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanCopyActor)
		);
		PluginCommands->MapAction(
			editorCommand.CutActor,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::CutSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanCutActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanCutActor)
		);
		PluginCommands->MapAction(
			editorCommand.PasteActor,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::PasteSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanPasteActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanPasteActor)
		);
		PluginCommands->MapAction(
			editorCommand.DuplicateActor,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::DuplicateSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanDuplicateActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanDuplicateActor)
		);
		PluginCommands->MapAction(
			editorCommand.DestroyActor,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::DeleteSelectedActors_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanDeleteActor),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanDeleteActor)
		);
		PluginCommands->MapAction(
			editorCommand.ToggleSpatiallyLoaded,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::ToggleSelectedActorsSpatiallyLoaded_Impl),
			FCanExecuteAction::CreateStatic(&LPrefabEditorTools::CanToggleActorSpatiallyLoaded),
			FGetActionCheckState::CreateStatic(&LPrefabEditorTools::GetActorSpatiallyLoadedProperty),
			FIsActionButtonVisible::CreateStatic(&LPrefabEditorTools::CanToggleActorSpatiallyLoaded)
		);

		//component action
		PluginCommands->MapAction(
			editorCommand.CopyComponentValues,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::CopyComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([] {return GEditor->GetSelectedComponentCount() > 0; })
		);
		PluginCommands->MapAction(
			editorCommand.PasteComponentValues,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::PasteComponentValues_Impl),
			FCanExecuteAction::CreateLambda([] {return LPrefabEditorTools::HaveValidCopiedComponent(); }),
			FGetActionCheckState(),
			FIsActionButtonVisible::CreateLambda([] {return LPrefabEditorTools::HaveValidCopiedComponent(); })
		);
		//settings
		PluginCommands->MapAction(
			editorCommand.ToggleLGUIInfoColume,
			FExecuteAction::CreateRaw(this, &FLPrefabEditorModule::ToggleLGUIColumnInfo),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FLPrefabEditorModule::IsLGUIColumnInfoChecked)
		);
		//gc
		PluginCommands->MapAction(
			editorCommand.ForceGC,
			FExecuteAction::CreateStatic(&LPrefabEditorTools::ForceGC)
		);

		//TSharedPtr<FExtender> toolbarExtender = MakeShareable(new FExtender);
		//toolbarExtender->AddToolBarExtension("Play", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLPrefabEditorModule::AddEditorToolsToToolbarExtension));
		//LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(toolbarExtender);
		//LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());
	}
	//register SceneOutliner ColumnInfo
	{
		ApplyLGUIColumnInfo(IsLGUIColumnInfoChecked(), false);
		//SceneOutliner extension
		NativeSceneOutlinerExtension = new FLPrefabNativeSceneOutlinerExtension();
	}
	//register window
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LPrefabSequenceTabName, FOnSpawnTab::CreateRaw(this, &FLPrefabEditorModule::HandleSpawnLPrefabSequenceTab))
			.SetDisplayName(LOCTEXT("LPrefabSequenceTabName", "LPrefab Sequence"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	//register custom editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.RegisterCustomClassLayout(ULPrefab::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLPrefabCustomization::MakeInstance));
		PropertyModule.RegisterCustomClassLayout(ULPrefabSequenceComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLPrefabSequenceComponentCustomization::MakeInstance));
	}
	//register asset
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		//register AssetCategory
		EAssetTypeCategories::Type LGUIAssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName(TEXT("LPrefab")));
		if (LGUIAssetCategoryBit == EAssetTypeCategories::Misc)
		{
			LGUIAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("LPrefab")), LOCTEXT("LPrefabAssetCategory", "LPrefab"));
		}

		TSharedPtr<FAssetTypeActions_Base> PrefabDataAction = MakeShareable(new FAssetTypeActions_LPrefab(EAssetTypeCategories::Basic));
		AssetTools.RegisterAssetTypeActions(PrefabDataAction.ToSharedRef());
		AssetTypeActionsArray.Add(PrefabDataAction);
	}
	//register Thumbnail
	{
		UThumbnailManager::Get().RegisterCustomRenderer(ULPrefab::StaticClass(), ULPrefabThumbnailRenderer::StaticClass());
	}
	//register right mouse button in content browser
	{
		if (!IsRunningCommandlet())
		{
			FLPrefabContentBrowserExtensions::InstallHooks();
			FLPrefabLevelEditorExtensions::InstallHooks();
		}
	}
	//register setting
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "LPrefab",
				LOCTEXT("LPrefabSettingsName", "LPrefab"),
				LOCTEXT("LPrefabSettingsDescription", "LPrefab Settings"),
				GetMutableDefault<ULPrefabSettings>());

			LPrefabSequencerSettings = USequencerSettingsContainer::GetOrCreate<ULPrefabSequencerSettings>(TEXT("EmbeddedLPrefabSequenceEditor"));
			SettingsModule->RegisterSettings("Editor", "ContentEditors", "EmbeddedLPrefabSequenceEditor",
				LOCTEXT("LPrefabSequencerSettingsName", "LPrefab Sequence Editor"),
				LOCTEXT("LPrefabSequencerSettingsDescription", "Configure the look and feel of the LPrefab Sequence Editor."),
				LPrefabSequencerSettings);
		}
	}

	CheckPrefabOverrideDataViewerEntry();
}

void FLPrefabEditorModule::OnInitializeSequence(ULPrefabSequence* Sequence)
{
	auto* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	UMovieScene* MovieScene = Sequence->GetMovieScene();

	FFrameNumber StartFrame = (ProjectSettings->DefaultStartTime * MovieScene->GetTickResolution()).RoundToFrame();
	int32        Duration = (ProjectSettings->DefaultDuration * MovieScene->GetTickResolution()).RoundToFrame().Value;

	MovieScene->SetPlaybackRange(StartFrame, Duration);
}

void FLPrefabEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FLPrefabEditorStyle::Shutdown();

	FLPrefabEditorModuleCommands::Unregister();

	ULPrefabSequence::OnInitializeSequence().Remove(OnInitializeSequenceHandle);
	ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
	if (SequencerModule)
	{
		SequencerModule->UnregisterSequenceEditor(SequenceEditorHandle);
	}

	//unregister SceneOutliner ColumnInfo
	if (FModuleManager::Get().IsModuleLoaded("SceneOutliner"))
	{
		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
		SceneOutlinerModule.UnRegisterColumnType<LPrefabSceneOutliner::FLPrefabSceneOutlinerInfoColumn>();
		delete NativeSceneOutlinerExtension;
		NativeSceneOutlinerExtension = nullptr;
	}
	//unregister window
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LPrefabSequenceTabName);
	}
	//unregister custom editor
	if (UObjectInitialized() && FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(ULPrefab::StaticClass()->GetFName());

		PropertyModule.UnregisterCustomClassLayout(ULPrefabSequenceComponent::StaticClass()->GetFName());
	}
	//unregister asset
	{
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			for (TSharedPtr<FAssetTypeActions_Base>& AssetTypeActions : AssetTypeActionsArray)
			{
				AssetTools.UnregisterAssetTypeActions(AssetTypeActions.ToSharedRef());
			}
		}
		AssetTypeActionsArray.Empty();
	}
	//unregister thumbnail
	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(ULPrefab::StaticClass());
	}
	//unregister right mouse button in content browser
	{
		FLPrefabContentBrowserExtensions::RemoveHooks();
		FLPrefabLevelEditorExtensions::RemoveHooks();
	}

	//unregister setting
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "LPrefab");
		SettingsModule->UnregisterSettings("Project", "Plugins", "LPrefabSequencerSettings");
	}
}

void FLPrefabEditorModule::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(LPrefabSequencerSettings);
}
FString FLPrefabEditorModule::GetReferencerName() const 
{
	return "LPrefabEditorModule";
}

FLPrefabEditorModule& FLPrefabEditorModule::Get()
{
	return FModuleManager::Get().GetModuleChecked<FLPrefabEditorModule>(TEXT("LPrefabEditor"));
}

void FLPrefabEditorModule::CheckPrefabOverrideDataViewerEntry()
{
	if (PrefabOverrideDataViewer != nullptr && PrefabOverrideDataViewer.IsValid())return;
	PrefabOverrideDataViewer = 
	SNew(SLPrefabOverrideDataViewer, nullptr)
	.AfterRevertPrefab_Lambda([=, this](ULPrefab* PrefabAsset) {
		OnOutlinerSelectionChange();//force refresh
		})
	.AfterApplyPrefab_Lambda([=, this](ULPrefab* PrefabAsset) {
		OnOutlinerSelectionChange();//force refresh
		LPrefabEditorTools::RefreshLevelLoadedPrefab(PrefabAsset);
		LPrefabEditorTools::RefreshOnSubPrefabChange(PrefabAsset);
		LPrefabEditorTools::RefreshOpenedPrefabEditor(PrefabAsset);
		})
	;
}

TSharedRef<SDockTab> FLPrefabEditorModule::HandleSpawnLPrefabSequenceTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	auto TabContentWidget = SNew(SLPrefabSequenceEditor);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

bool FLPrefabEditorModule::CanUnpackActorForPrefab()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor))
		{
			return true;
		}
		else if (PrefabHelperObject->MissingPrefab.Contains(SelectedActor))
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}
bool FLPrefabEditorModule::CanBrowsePrefab()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor))
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool FLPrefabEditorModule::CanUpdateLevelPrefab()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(SelectedActor) && !PrefabHelperObject->IsInsidePrefabEditor())//Can only update prefab in level editor
		{
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

ECheckBoxState FLPrefabEditorModule::GetAutoUpdateLevelPrefab()const
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return ECheckBoxState::Undetermined;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (auto SubPrefabDataPtr = PrefabHelperObject->SubPrefabMap.Find(SelectedActor))
		{
			return SubPrefabDataPtr->bAutoUpdate ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
		}
	}
	return ECheckBoxState::Undetermined;
}

bool FLPrefabEditorModule::CanCreateActor()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (!LPrefabEditorTools::IsActorCompatibleWithLGUIToolsMenu(SelectedActor))return false;
	return true;
}

bool FLPrefabEditorModule::CanCheckPrefabOverrideParameter()const
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Key == SelectedActor || SelectedActor->IsAttachedTo(KeyValue.Key))
			{
				return true;
			}
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool FLPrefabEditorModule::CanReplaceActor()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (!LPrefabEditorTools::IsActorCompatibleWithLGUIToolsMenu(SelectedActor))return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))//sub prefab's actor not allow replace
		{
			return false;
		}
		else if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(SelectedActor))//missing sub prefab's actor not allowed
		{
			return false;
		}
	}
	return true;
}

bool FLPrefabEditorModule::CanCreatePrefab()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return false;
	if (!LPrefabEditorTools::IsActorCompatibleWithLGUIToolsMenu(SelectedActor))return false;
	if (SelectedActor->HasAnyFlags(EObjectFlags::RF_Transient))return false;
	if (auto PrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
	{
		if (PrefabHelperObject->LoadedRootActor == SelectedActor)
		{
			return false;
		}
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))
		{
			return false;
		}
		else if (PrefabHelperObject->IsActorBelongsToMissingSubPrefab(SelectedActor))
		{
			return false;
		}
	}
	return true;
}

FLPrefabNativeSceneOutlinerExtension* FLPrefabEditorModule::GetNativeSceneOutlinerExtension()const
{
	return NativeSceneOutlinerExtension;
}
void FLPrefabEditorModule::OnOutlinerSelectionChange()
{
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor == nullptr)return;
	auto NewPrefabHelperObject = LPrefabEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor);
	if (CurrentPrefabHelperObject != NewPrefabHelperObject)
	{
		CurrentPrefabHelperObject = NewPrefabHelperObject;
		if (CurrentPrefabHelperObject != nullptr)
		{
			PrefabOverrideDataViewer->SetPrefabHelperObject(CurrentPrefabHelperObject.Get());
		}
	}
	if (CurrentPrefabHelperObject != nullptr)
	{
		bool bIsSubPrefabRoot = false;
		for (auto& KeyValue : CurrentPrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Key == SelectedActor)
			{
				bIsSubPrefabRoot = true;
				break;
			}
		}
		PrefabOverrideDataViewer->RefreshDataContent(CurrentPrefabHelperObject->GetSubPrefabData(SelectedActor).ObjectOverrideParameterArray, bIsSubPrefabRoot ? nullptr : SelectedActor);
	}
}

void FLPrefabEditorModule::AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.BeginSection("LGUI");
	{
		Builder.AddComboButton(
			FUIAction(),
			FOnGetContent::CreateRaw(this, &FLPrefabEditorModule::MakeEditorToolsMenu, true, true, true, true, true),
			LOCTEXT("LGUITools", "LGUI Tools"),
			LOCTEXT("LPrefabEditorTools", "LGUI Editor Tools"),
			FSlateIcon(FLPrefabEditorStyle::GetStyleSetName(), "LPrefabEditor.EditorTools")
		);
	}
	Builder.EndSection();
}

TSharedRef<SWidget> FLPrefabEditorModule::MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool PreviewInViewport, bool EditorCameraControl, bool Others)
{
	FMenuBuilder MenuBuilder(true, PluginCommands);
	auto commandList = FLPrefabEditorModuleCommands::Get();

	//prefab
	{
		MenuBuilder.BeginSection("Prefab", LOCTEXT("Prefab", "Prefab"));
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CreatePrefab", "Create Prefab"),
				LOCTEXT("CreatePrefab_Tooltip", "Use selected actor to create a new prefab"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::CreatePrefabAsset)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanCreatePrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanCreatePrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("UnpackPrefab", "Unpack this Prefab"),
				LOCTEXT("UnpackPrefab_Tooltip", "Unpack the actor from related prefab asset"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::UnpackPrefab)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanUnpackActorForPrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanUnpackActorForPrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("SelectPrefabAsset", "Browse to Prefab asset"),
				LOCTEXT("SelectPrefabAsset_Tooltip", "Browse to Prefab asset in Content Browser"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::SelectPrefabAsset)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanBrowsePrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanBrowsePrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("OpenPrefabAsset", "Open Prefab asset"),
				LOCTEXT("OpenPrefabAsset_Tooltip", "Open Prefab asset in PrefabEditor"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::OpenPrefabAsset)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanBrowsePrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanBrowsePrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("UpdateLevelPrefab", "Update Prefab"),
				LOCTEXT("UpdateLevelPrefab_Tooltip", "Update this prefab to latest version"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::UpdateLevelPrefab)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanUpdateLevelPrefab)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanUpdateLevelPrefab))
			);
			MenuBuilder.AddMenuEntry(
				LOCTEXT("AutoUpdateLevelPrefab", "Auto Update Prefab"),
				LOCTEXT("AutoUpdateLevelPrefab_Tooltip", "Auto update this prefab when detect newer version"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LPrefabEditorTools::ToggleLevelPrefabAutoUpdate)
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanUpdateLevelPrefab)
					, FGetActionCheckState::CreateRaw(this, &FLPrefabEditorModule::GetAutoUpdateLevelPrefab)
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanUpdateLevelPrefab)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
			CheckPrefabOverrideDataViewerEntry();
			MenuBuilder.AddMenuEntry(
				FUIAction(FExecuteAction()
					, FCanExecuteAction::CreateRaw(this, &FLPrefabEditorModule::CanCheckPrefabOverrideParameter)
					, FGetActionCheckState()
					, FIsActionButtonVisible::CreateRaw(this, &FLPrefabEditorModule::CanCheckPrefabOverrideParameter))
				, 
				SNew(SComboButton)
				.HasDownArrow(true)
				.ToolTipText(LOCTEXT("PrefabOverride", "Edit override parameters for this prefab"))
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.MenuContent()
				[
					SNew(SBox)
					.Padding(FMargin(4, 4))
					[
						SNew(SHorizontalBox)
						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									PrefabOverrideDataViewer.ToSharedRef()
								]
							]
						]
					]
				]
			);
		}
		MenuBuilder.EndSection();
	}

	PrefabEditor_OnExtendActorMenu.ExecuteIfBound(MenuBuilder, InitialSetup);

	MenuBuilder.BeginSection("CommonActor", LOCTEXT("CommonActor", "Create Common Actors"));
	{
		this->CreateCommonActorSubMenu(MenuBuilder);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("ActorAction", LOCTEXT("ActorAction", "Edit Actor With Hierarchy"));
	{
		MenuBuilder.AddMenuEntry(commandList.CopyActor);
		MenuBuilder.AddMenuEntry(commandList.PasteActor);
		MenuBuilder.AddMenuEntry(commandList.CutActor);
		MenuBuilder.AddMenuEntry(commandList.DuplicateActor);
		MenuBuilder.AddMenuEntry(commandList.DestroyActor);
		MenuBuilder.AddMenuEntry(commandList.ToggleSpatiallyLoaded);
		PrefabEditor_OnExtendActorActionMenu.ExecuteIfBound(MenuBuilder);
	}
	MenuBuilder.EndSection();

	if (ComponentAction)
	{
		MenuBuilder.BeginSection("ComponentAction", LOCTEXT("ComponentAction", "Edit Component"));
		{
			MenuBuilder.AddMenuEntry(commandList.CopyComponentValues);
			MenuBuilder.AddMenuEntry(commandList.PasteComponentValues);
		}
		MenuBuilder.EndSection();
	}

	PrefabEditor_OnExtendEditorCameraControlMenu.ExecuteIfBound(MenuBuilder, EditorCameraControl);

	if (Others)
	{
		MenuBuilder.BeginSection("Others", LOCTEXT("Others", "Others"));
		{
			PrefabEditor_OnExtendOthersMenu.ExecuteIfBound(MenuBuilder);
			MenuBuilder.AddMenuEntry(commandList.ToggleLGUIInfoColume);
			MenuBuilder.AddMenuEntry(commandList.ForceGC);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

#include "IPlacementModeModule.h"
#include "AssetSelection.h"
#include "LevelEditorViewport.h"
void FLPrefabEditorModule::CreateCommonActorSubMenu(FMenuBuilder& MenuBuilder)
{
	struct LOCAL
	{
		struct TempGWorldCurrentLevel
		{
			ULevel* OriginLevel = nullptr;
			TempGWorldCurrentLevel(ULevel* NewLevel)
			{
				OriginLevel = GWorld->GetCurrentLevel();
				GWorld->SetCurrentLevel(NewLevel);
			}
			~TempGWorldCurrentLevel()
			{
				GWorld->SetCurrentLevel(OriginLevel);
			}
		};
		//reference from GEditor->UseActorFactory
		static AActor* UseActorFactory(UActorFactory* Factory, const FAssetData& AssetData)
		{
			AActor* NewActor = nullptr;

			if (auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor())
			{
				LPrefabEditorTools::MakeCurrentLevel(SelectedActor);
				if (ULevel* DesiredLevel = SelectedActor->GetLevel())
				{
					TempGWorldCurrentLevel Temp(DesiredLevel);//temporary change level, because when create actor form asset, the function (PrivateAddActor) use level by GWorld->GetCurrentLevel
					if (UObject* LoadedAsset = AssetData.GetAsset())
					{
						auto Actors = FLevelEditorViewportClient::TryPlacingActorFromObject(DesiredLevel, LoadedAsset, true, RF_Transactional, Factory);
						if (Actors.Num() && (Actors[0] != nullptr))
						{
							NewActor = Actors[0];
							NewActor->SetActorRelativeTransform(FTransform::Identity);

							auto SelectedRootComp = SelectedActor->GetRootComponent();
							auto NewRootComp = NewActor->GetRootComponent();
							if (SelectedRootComp && NewRootComp)
							{
								NewRootComp->SetMobility(SelectedRootComp->Mobility);
								NewActor->AttachToActor(SelectedActor, FAttachmentTransformRules::KeepRelativeTransform);
							}
						}
					}
				}
			}

			return NewActor;
		}
		//reference from SPlacementAssetMenuEntry::OnMouseButtonUp
		static void CreateActor(TSharedPtr<FPlaceableItem> Item)
		{
			AActor* NewActor = nullptr;
			UActorFactory* Factory = Item->Factory;
			if (!Item->Factory)
			{
				// If no actor factory was found or failed, add the actor from the uclass
				UClass* AssetClass = Item->AssetData.GetClass();
				if (AssetClass)
				{
					UObject* ClassObject = AssetClass->GetDefaultObject();
					FActorFactoryAssetProxy::GetFactoryForAssetObject(ClassObject);
				}
			}
			//reference from FLevelEditorActionCallbacks::AddActor
			NewActor = UseActorFactory(Factory, Item->AssetData);
			if (NewActor != NULL && IPlacementModeModule::IsAvailable())
			{
				IPlacementModeModule::Get().AddToRecentlyPlaced(Item->AssetData.GetAsset(), Factory);
			}
		}
		static void CreateCommonActorMenuEntry(FMenuBuilder& InBuilder, TSharedPtr<FPlaceableItem> Item)
		{
			InBuilder.AddMenuEntry(
				Item->DisplayName,
				FText(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateStatic(&LOCAL::CreateActor, Item))
			);
		}
		static void MakeMenu(FMenuBuilder& MenuBuilder, const TArray<FPlacementCategoryInfo>& Categories, FLPrefabEditorModule* EditorModulePtr, IPlacementModeModule& PlacementMode)
		{
			for (auto GroupDataItem : Categories)
			{
				if (GroupDataItem.UniqueHandle == FBuiltInPlacementCategories::RecentlyPlaced())
					GroupDataItem.DisplayName = LOCTEXT("RecentlyPlaced", "Recently Created");

				PlacementMode.RegenerateItemsForCategory(GroupDataItem.UniqueHandle);
				TArray<TSharedPtr<FPlaceableItem>> Items;
				PlacementMode.GetItemsForCategory(GroupDataItem.UniqueHandle, Items);
				if (Items.Num() <= 0)
					continue;

				MenuBuilder.AddSubMenu(
					GroupDataItem.DisplayName,
					FText(),
					FNewMenuDelegate::CreateLambda([Items, UniqueHandle = GroupDataItem.UniqueHandle](FMenuBuilder& MenuBuilder) {
						MenuBuilder.BeginSection(UniqueHandle);
						{
							MenuBuilder.AddSearchWidget();
							for (auto& Item : Items)
							{
								CreateCommonActorMenuEntry(MenuBuilder, Item);
							}
						}
						MenuBuilder.EndSection();
						}),
					FUIAction(FExecuteAction()
						, FCanExecuteAction()
						, FGetActionCheckState()
						, FIsActionButtonVisible::CreateStatic(&FLPrefabEditorModule::CanCreateActor)),
					NAME_None, EUserInterfaceActionType::None
				);
			}
		}
	};

	auto& PlacementMode = IPlacementModeModule::Get();
	TArray<FPlacementCategoryInfo> Categories;
	PlacementMode.GetSortedCategories(Categories);
	LOCAL::MakeMenu(MenuBuilder, Categories, this, PlacementMode);
}

void FLPrefabEditorModule::ToggleLGUIColumnInfo()
{
	auto LPrefabEditorSettings = GetMutableDefault<ULPrefabEditorSettings>();
	LPrefabEditorSettings->ShowLGUIColumnInSceneOutliner = !LPrefabEditorSettings->ShowLGUIColumnInSceneOutliner;
	LPrefabEditorSettings->SaveConfig();

	ApplyLGUIColumnInfo(LPrefabEditorSettings->ShowLGUIColumnInSceneOutliner, true);
}
bool FLPrefabEditorModule::IsLGUIColumnInfoChecked()
{
	return GetDefault<ULPrefabEditorSettings>()->ShowLGUIColumnInSceneOutliner;
}

void FLPrefabEditorModule::ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner)
{
	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked< FSceneOutlinerModule >("SceneOutliner");
	if (value)
	{
		FSceneOutlinerColumnInfo ColumnInfo(ESceneOutlinerColumnVisibility::Visible, 15, FCreateSceneOutlinerColumn::CreateStatic(&LPrefabSceneOutliner::FLPrefabSceneOutlinerInfoColumn::MakeInstance));
		SceneOutlinerModule.RegisterDefaultColumnType<LPrefabSceneOutliner::FLPrefabSceneOutlinerInfoColumn>(ColumnInfo);
	}
	else
	{
		SceneOutlinerModule.UnRegisterColumnType<LPrefabSceneOutliner::FLPrefabSceneOutlinerInfoColumn>();
	}

	//refresh scene outliner
	if (refreshSceneOutliner)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

		TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
		if (LevelEditorTabManager->FindExistingLiveTab(FName("LevelEditorSceneOutliner")).IsValid())
		{
			if (LevelEditorTabManager.IsValid() && LevelEditorTabManager.Get())
			{
				if (LevelEditorTabManager->GetOwnerTab().IsValid())
				{
					LevelEditorTabManager->TryInvokeTab(FName("LevelEditorSceneOutliner"))->RequestCloseTab();
				}
			}

			if (LevelEditorTabManager.IsValid() && LevelEditorTabManager.Get())
			{
				if (LevelEditorTabManager->GetOwnerTab().IsValid())
				{
					LevelEditorTabManager->TryInvokeTab(FName("LevelEditorSceneOutliner"));
				}
			}
		}
	}
}

IMPLEMENT_MODULE(FLPrefabEditorModule, LPrefabEditor)

#undef LOCTEXT_NAMESPACE