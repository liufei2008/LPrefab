// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "SceneOutliner/LPrefabNativeSceneOutlinerExtension.h"
#include "LPrefabEditorModule.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Views/STreeView.h"
#include "ISceneOutliner.h"
#include "ActorTreeItem.h"
#include "FolderTreeItem.h"
#include "WorldTreeItem.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "EditorActorFolders.h"
#include "LPrefabEditorSettings.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "LPrefabUtils.h"
#include "UObject/ObjectSaveContext.h"
#include "PrefabSystem/LPrefabManager.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "LPrefabNativeSceneOutlinerExtension"
FLPrefabNativeSceneOutlinerExtension::FLPrefabNativeSceneOutlinerExtension()
{
	OnPreSaveWorldDelegateHandle = FEditorDelegates::PreSaveWorldWithContext.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::OnPreSaveWorld);
	OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::OnMapOpened);
	OnPreBeginPIEDelegateHandle = FEditorDelegates::PreBeginPIE.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::OnPreBeginPIE);
	OnBeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::OnBeginPIE);
	OnEndPIEDelegateHandle = FEditorDelegates::EndPIE.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::OnEndPIE);
	OnLPrefabEditorPreserveHierarchyStateChangeDelegateHandle = ULPrefabEditorSettings::LPrefabEditorSetting_PreserveHierarchyStateChange.AddRaw(this, &FLPrefabNativeSceneOutlinerExtension::PreserveHierarchyStateChange);
}
FLPrefabNativeSceneOutlinerExtension::~FLPrefabNativeSceneOutlinerExtension()
{
	FEditorDelegates::PreSaveWorldWithContext.Remove(OnPreSaveWorldDelegateHandle);
	FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	FEditorDelegates::PreBeginPIE.Remove(OnPreBeginPIEDelegateHandle);
	FEditorDelegates::BeginPIE.Remove(OnBeginPIEDelegateHandle);
	FEditorDelegates::EndPIE.Remove(OnEndPIEDelegateHandle);
}
void FLPrefabNativeSceneOutlinerExtension::Tick(float DeltaTime)
{
	if (needToRestore)
	{
		delayRestoreTime += DeltaTime;
		if (delayRestoreTime > ULPrefabEditorSettings::GetDelayRestoreHierarchyTime())
		{
			RestoreSceneOutlinerState();
			delayRestoreTime = 0;
			needToRestore = false;
		}
	}
}
TStatId FLPrefabNativeSceneOutlinerExtension::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULPrefabEditorManagerObject, STATGROUP_Tickables);
}

void FLPrefabNativeSceneOutlinerExtension::Restore() 
{
	SetDelayRestore(true, false); 
}

void FLPrefabNativeSceneOutlinerExtension::OnPreSaveWorld(UWorld* World, FObjectPreSaveContext Context)
{
	SaveSceneOutlinerState();
}
void FLPrefabNativeSceneOutlinerExtension::OnMapOpened(const FString& FileName, bool AsTemplate)
{
	SetDelayRestore(true, false);
}
void FLPrefabNativeSceneOutlinerExtension::OnPreBeginPIE(const bool IsSimulating)
{
	SaveSceneOutlinerStateForPIE();
}
void FLPrefabNativeSceneOutlinerExtension::OnBeginPIE(const bool IsSimulating)
{
	SetDelayRestore(false, true);
}
void FLPrefabNativeSceneOutlinerExtension::OnEndPIE(const bool IsSimulating)
{
	SetDelayRestore(true, false);
}
void FLPrefabNativeSceneOutlinerExtension::SetDelayRestore(bool RestoreTemporarilyHidden, bool RestoreUseFName)
{
	shouldRestoreTemporarilyHidden = RestoreTemporarilyHidden;
	needToRestore = true;
	delayRestoreTime = 0;
	shouldRestoreUseFNameData = RestoreUseFName;
}
void FLPrefabNativeSceneOutlinerExtension::PreserveHierarchyStateChange()
{
	if (!ULPrefabEditorSettings::GetPreserveHierarchyState())//no need to preseve it, just delete the actor
	{
		auto storageActor = FindDataStorageActor(false);
		LPrefabUtils::DestroyActorWithHierarchy(storageActor);
	}
}

void FLPrefabNativeSceneOutlinerExtension::OnIterateTreeItem(const TFunction<void(STreeView<FSceneOutlinerTreeItemPtr>&, FSceneOutlinerTreeItemPtr&)>& Function)
{
	TWeakPtr<class ILevelEditor> LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor")).GetLevelEditorInstance();
	if (LevelEditor.IsValid())
	{
		TWeakPtr<class ISceneOutliner> SceneOutlinerPtr = LevelEditor.Pin()->GetMostRecentlyUsedSceneOutliner();
		if (TSharedPtr<class ISceneOutliner> SceneOutlinerPin = SceneOutlinerPtr.Pin())
		{
			auto& TreeView = SceneOutlinerPin->GetTree();
			TSet<FSceneOutlinerTreeItemPtr> VisitingItems;
			TreeView.GetExpandedItems(VisitingItems);
			auto& MutableTreeView = const_cast<STreeView<FSceneOutlinerTreeItemPtr>&>(TreeView);
			for (FSceneOutlinerTreeItemPtr& Item : VisitingItems)
			{
				Function(MutableTreeView, Item);
			}
		}
	}
}

void FLPrefabNativeSceneOutlinerExtension::SaveSceneOutlinerState()
{
	if (!ULPrefabEditorSettings::GetPreserveHierarchyState())return;
	auto storageActor = FindDataStorageActor();
	if (!storageActor)return;
	storageActor->ExpandedFolderArray.Reset();
	storageActor->ExpandedActorArray.Reset();
	storageActor->ExpandedSoftActorArray.Reset();
	OnIterateTreeItem([&](STreeView<FSceneOutlinerTreeItemPtr>& TreeView, FSceneOutlinerTreeItemPtr& Item) {
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				if (storageActor->GetLevel() == ActorTreeItem->Actor->GetLevel())
				{
					storageActor->ExpandedActorArray.Add(ActorTreeItem->Actor);
				}
				else
				{
					storageActor->ExpandedSoftActorArray.Add(ActorTreeItem->Actor.Get());
				}
			}
		}
		else if (auto FolderTreeItem = Item->CastTo<FFolderTreeItem>())
		{
			storageActor->ExpandedFolderArray.Add(FolderTreeItem->Path);
		}
		else if (auto WorldTreeItem = Item->CastTo<FWorldTreeItem>())
		{
			if (WorldTreeItem->World.IsValid())
			{

			}
		}
		});

	storageActor->TemporarilyHiddenActorArray.Reset();
	storageActor->TemporarilyHiddenSoftActorArray.Reset();
	if (auto world = GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if(!IsValid(*ActorItr))continue;
			if (ActorItr->GetClass() == ALPrefabEditorLevelDataStorageActor::StaticClass())continue;//skip it-self
			if (ActorItr->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient
			if ((*ActorItr)->IsTemporarilyHiddenInEditor())
			{
				if (storageActor->GetLevel() == (*ActorItr)->GetLevel())
				{
					storageActor->TemporarilyHiddenActorArray.Add(*ActorItr);
				}
				else
				{
					storageActor->TemporarilyHiddenSoftActorArray.Add(*ActorItr);
				}
			}
		}
	}
}
void FLPrefabNativeSceneOutlinerExtension::SaveSceneOutlinerStateForPIE()
{
	if (!ULPrefabEditorSettings::GetPreserveHierarchyState())return;
	ExpandedActorArray.Reset();
	ExpandedFolderArray.Reset();
	OnIterateTreeItem([&](STreeView<FSceneOutlinerTreeItemPtr>& TreeView, FSceneOutlinerTreeItemPtr& Item) {
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				ExpandedActorArray.Add(ActorTreeItem->Actor->GetFName());
			}
		}
		else if (auto FolderTreeItem = Item->CastTo<FFolderTreeItem>())
		{
			ExpandedFolderArray.Add(FolderTreeItem->Path);
		}
		else if (auto WorldTreeItem = Item->CastTo<FWorldTreeItem>())
		{
			if (WorldTreeItem->World.IsValid())
			{

			}
		}
		});
}
ALPrefabEditorLevelDataStorageActor* FLPrefabNativeSceneOutlinerExtension::FindDataStorageActor(bool CreateIfNotExist)
{
	ALPrefabEditorLevelDataStorageActor* result = nullptr;
	if (auto world = GEditor->GetEditorWorldContext().World())
	{
		auto baseLevel = world->GetLevel(0);
		TArray<ALPrefabEditorLevelDataStorageActor*> needToDelete;
		for (TActorIterator<ALPrefabEditorLevelDataStorageActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if (!IsValid(*ActorItr))continue;
			if (ActorItr->GetLevel() == baseLevel)
			{
				if (result == nullptr)
				{
					result = *ActorItr;
				}
				else
				{
					needToDelete.Add(*ActorItr);
				}
			}
		}
		if (needToDelete.Num() > 1)
		{
			auto msg = FText::Format(LOCTEXT("MultipleDataStorageActorError", "[ULPrefabNativeSceneOutlinerExtension::FindOrCreateDataStorageActor]There are {0} count of LPrefabEditorLevelDataStorageActor, this is weird..."), needToDelete.Num());
			UE_LOG(LPrefabEditor, Error, TEXT("%s"), *msg.ToString());
			LPrefabUtils::EditorNotification(msg);
			for (auto item : needToDelete)
			{
				item->Destroy();
			}
			needToDelete.Empty();
		}
		if (!result && CreateIfNotExist)
		{
			world->SetCurrentLevel(baseLevel);
			result = world->SpawnActor<ALPrefabEditorLevelDataStorageActor>();
		}
	}
	return result;
}

void FLPrefabNativeSceneOutlinerExtension::RestoreSceneOutlinerState()
{
	if (!ULPrefabEditorSettings::GetPreserveHierarchyState())return;
	auto storageActor = FindDataStorageActor();
	if (!storageActor)return;
	OnIterateTreeItem([&](STreeView<FSceneOutlinerTreeItemPtr>& TreeView, FSceneOutlinerTreeItemPtr& Item) {
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				//expend
				if (shouldRestoreUseFNameData)
				{
					bool needToExpand = ExpandedActorArray.Contains(ActorTreeItem->Actor->GetFName());
					TreeView.SetItemExpansion(Item, needToExpand);
				}
				else
				{
					if (storageActor->GetLevel() == ActorTreeItem->Actor->GetLevel())
					{
						bool needToExpand = storageActor->ExpandedActorArray.Contains(ActorTreeItem->Actor);
						TreeView.SetItemExpansion(Item, needToExpand);
					}
					else
					{
						bool needToExpand = storageActor->ExpandedSoftActorArray.Contains(ActorTreeItem->Actor.Get());
						TreeView.SetItemExpansion(Item, needToExpand);
					}
				}
			}
		}
		else if (auto FolderTreeItem = Item->CastTo<FFolderTreeItem>())
		{
			//expend
			if (shouldRestoreUseFNameData)
			{
				bool needToExpand = ExpandedFolderArray.Contains(FolderTreeItem->Path);
				TreeView.SetItemExpansion(Item, needToExpand);
			}
			else
			{
				bool needToExpand = storageActor->ExpandedFolderArray.Contains(FolderTreeItem->Path);
				TreeView.SetItemExpansion(Item, needToExpand);
			}
		}
		else if (auto WorldTreeItem = Item->CastTo<FWorldTreeItem>())
		{
			if (WorldTreeItem->World.IsValid())
			{

			}
		}
		});

	if (shouldRestoreTemporarilyHidden)
	{
		//hidden
		if (auto world = GEditor->GetEditorWorldContext().World())
		{
			for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
			{
				if (!IsValid(*ActorItr))continue;

				if (FLPrefabEditorModule::PrefabEditor_ActorSupportRestoreTemporarilyHidden.IsBound())
					if (!FLPrefabEditorModule::PrefabEditor_ActorSupportRestoreTemporarilyHidden.Execute(*ActorItr))
						continue;
				if (storageActor->GetLevel() == (ActorItr->GetLevel()))
				{
					if (storageActor->TemporarilyHiddenActorArray.Contains((*ActorItr)))
					{
						(*ActorItr)->SetIsTemporarilyHiddenInEditor(true);
					}
				}
				else
				{
					if (storageActor->TemporarilyHiddenSoftActorArray.Contains((*ActorItr)))
					{
						(*ActorItr)->SetIsTemporarilyHiddenInEditor(true);
					}
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE

PRAGMA_ENABLE_OPTIMIZATION
