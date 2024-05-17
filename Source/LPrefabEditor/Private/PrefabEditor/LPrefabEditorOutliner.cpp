// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorOutliner.h"
#include "SceneOutlinerModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"
#include "Widgets/Layout/SBox.h"
#include "SceneOutlinerDelegates.h"
#include "ActorTreeItem.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "SceneOutliner/LPrefabSceneOutlinerInfoColumn.h"
#include "SOutlinerTreeView.h"
#include "Editor/GroupActor.h"
#include "LPrefabEditor.h"
#include "LPrefabEditorModule.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "SceneOutlinerDragDrop.h"
#include "PrefabSystem/LPrefab.h"
#include "PrefabSystem/LPrefabHelperObject.h"

PRAGMA_DISABLE_OPTIMIZATION

FLPrefabEditorOutliner::~FLPrefabEditorOutliner()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}

void FLPrefabEditorOutliner::InitOutliner(UWorld* World, TSharedPtr<FLPrefabEditor> InPrefabEditorPtr, const TSet<AActor*>& InUnexpendActorSet)
{
	CurrentWorld = World;
	PrefabEditorPtr = InPrefabEditorPtr;
	if (CurrentWorld == nullptr)
	{
		return;
	}

	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	FSceneOutlinerInitializationOptions InitOptions;
	InitOptions.bShowTransient = false;
	InitOptions.bFocusSearchBoxWhenOpened = false;
	InitOptions.bShowCreateNewFolder = false;
	InitOptions.ColumnMap.Add(LPrefabSceneOutliner::FLPrefabSceneOutlinerInfoColumn::GetID(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 2));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Gutter(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 0));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 1));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::ActorInfo(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 10));
	if (ActorFilter.IsBound())
	{
		InitOptions.Filters->AddFilterPredicate<FActorTreeItem>(ActorFilter);
	}
	InitOptions.OutlinerIdentifier = "LPrefabEditorOutliner";
	InitOptions.CustomDelete = FCustomSceneOutlinerDeleteDelegate::CreateRaw(this, &FLPrefabEditorOutliner::OnDelete);

	TSharedRef<ISceneOutliner> SceneOutlinerRef = SceneOutlinerModule.CreateActorBrowser(InitOptions, World);
	SceneOutlinerPtr = StaticCastSharedRef<SSceneOutliner>(SceneOutlinerRef->AsShared());

	//SceneOutlinerPtr->GetOnItemSelectionChanged().AddRaw(this, &FLPrefabEditorOutliner::OnSceneOutlinerSelectionChanged);
	SceneOutlinerPtr->GetDoubleClickEvent().AddRaw(this, &FLPrefabEditorOutliner::OnSceneOutlinerDoubleClick);
	GEditor->OnLevelActorListChanged().AddLambda([SceneOutlinerWeak = TWeakPtr<SSceneOutliner>(SceneOutlinerPtr)]() {//UE5 not auto refresh the actor label display, so manually refresh it
		if (SceneOutlinerWeak.IsValid())
		{
			SceneOutlinerWeak.Pin()->Refresh();
		}
		});
	FCoreDelegates::OnActorLabelChanged.AddLambda([SceneOutlinerWeak = TWeakPtr<SSceneOutliner>(SceneOutlinerPtr)](AActor* actor) {//UE5 not auto refresh the actor label display, so manually refresh it
		if (SceneOutlinerWeak.IsValid())
		{
			SceneOutlinerWeak.Pin()->FullRefresh();
		}
	});
	

	auto& TreeView = SceneOutlinerPtr->GetTree();
	TSet<FSceneOutlinerTreeItemPtr> VisitingItems;
	TreeView.GetExpandedItems(VisitingItems);
	auto& MutableTreeView = const_cast<STreeView<FSceneOutlinerTreeItemPtr>&>(TreeView);
	for (auto& Item : VisitingItems)
	{
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				if (InUnexpendActorSet.Contains(ActorTreeItem->Actor.Get()))
				{
					MutableTreeView.SetItemExpansion(Item, false);
				}
			}
		}
	}
	SceneOutlinerPtr->Refresh();

	OutlinerWidget =
		SNew(SBox)
		.WidthOverride(OutlinerWidth)
		.HeightOverride(OutlinerHeight)
		[
			SceneOutlinerRef
		];



	USelection::SelectionChangedEvent.AddRaw(this, &FLPrefabEditorOutliner::OnEditorSelectionChanged);
}

void FLPrefabEditorOutliner::UnexpandActorForDragDroppedPrefab(AActor* InActor)
{
	auto& TreeView = SceneOutlinerPtr->GetTree();
	TSet<FSceneOutlinerTreeItemPtr> VisitingItems;
	TreeView.GetExpandedItems(VisitingItems);
	auto& MutableTreeView = const_cast<STreeView<FSceneOutlinerTreeItemPtr>&>(TreeView);
	for (auto& Item : VisitingItems)
	{
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				if (ActorTreeItem->Actor->IsAttachedTo(InActor) || ActorTreeItem->Actor.Get() == InActor)
				{
					MutableTreeView.SetItemExpansion(Item, false);
				}
			}
		}
	}
}

void FLPrefabEditorOutliner::OnDelete(const TArray<TWeakPtr<ISceneOutlinerTreeItem>>& InSelectedTreeItemArray)
{
	TArray<TWeakObjectPtr<AActor>> InSelectedActorArray;
	for (auto Item : InSelectedTreeItemArray)
	{
		if (Item.IsValid())
		{
			if (auto Actor = GetActorFromTreeItem(Item.Pin()))
			{
				InSelectedActorArray.Add(Actor);
			}
		}
	}
	PrefabEditorPtr.Pin()->DeleteActors(InSelectedActorArray);
}

AActor* FLPrefabEditorOutliner::GetActorFromTreeItem(FSceneOutlinerTreeItemPtr TreeItem)const
{
	if (auto ActorTreeItem = TreeItem->CastTo<FActorTreeItem>())
	{
		if (ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
		{
			if (ActorTreeItem->Actor->GetWorld())
			{
				return Cast<AActor>(ActorTreeItem->Actor.Get());
			}
		}
	}
	return nullptr;
}

void FLPrefabEditorOutliner::OnSceneOutlinerDoubleClick(FSceneOutlinerTreeItemPtr ItemPtr)
{
	if (OnActorDoubleClickDelegate.IsBound())
	{
		FActorTreeItem* ActorTreeItem = (FActorTreeItem*)ItemPtr.Get();
		if (ActorTreeItem)
		{
			OnActorDoubleClickDelegate.ExecuteIfBound(ActorTreeItem->Actor.Get());
		}
	}
}

void FLPrefabEditorOutliner::OnEditorSelectionChanged(UObject* Object)
{
	if (!SceneOutlinerPtr.IsValid())
	{
		return;
	}

	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			if (Actor->GetWorld() != CurrentWorld.Get())
			{
				return;
			}
			OnActorPickedDelegate.ExecuteIfBound(Actor);

			SceneOutlinerPtr->SetSelection([=](ISceneOutlinerTreeItem& TreeItem) {
				if (auto ActorTree = TreeItem.CastTo<FActorTreeItem>())
				{
					return ActorTree->Actor.Get() == Actor;
				}
				return false;
				});

			SelectedActor = Actor;
		}
		else
		{
			SceneOutlinerPtr->ClearSelection();
			SelectedActor = nullptr;
		}
	}
}

void FLPrefabEditorOutliner::Refresh()
{
	if (SceneOutlinerPtr.IsValid())
	{
		SceneOutlinerPtr->Refresh();
	}
	else
	{
	}
}

void FLPrefabEditorOutliner::FullRefresh()
{
	if (SceneOutlinerPtr.IsValid())
	{
		SceneOutlinerPtr->FullRefresh();

	}
	else
	{
	}
}

void FLPrefabEditorOutliner::ClearSelectedActor()
{
	SelectedActor = nullptr;
}

void FLPrefabEditorOutliner::GetUnexpendActor(TArray<AActor*>& InOutAllActors)const
{
	auto& TreeView = SceneOutlinerPtr->GetTree();
	TSet<FSceneOutlinerTreeItemPtr> VisitingItems;
	TreeView.GetExpandedItems(VisitingItems);
	for (auto& Item : VisitingItems)
	{
		if (auto ActorTreeItem = Item->CastTo<FActorTreeItem>())
		{
			if (ActorTreeItem->Actor.IsValid())
			{
				InOutAllActors.Remove(ActorTreeItem->Actor.Get());
			}
		}
	}
}

PRAGMA_ENABLE_OPTIMIZATION

