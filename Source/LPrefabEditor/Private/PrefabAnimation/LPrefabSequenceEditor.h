// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Input/Reply.h"
#include "UObject/WeakObjectPtr.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/SCompoundWidget.h"

class ULPrefabSequenceComponent;
class ULPrefabSequence;
class SLPrefabSequenceEditorWidget;
struct FWidgetAnimationListItem;
class ULPrefabHelperObject;

class SLPrefabSequenceEditor : public SCompoundWidget
{
public:
	~SLPrefabSequenceEditor();

	SLATE_BEGIN_ARGS(SLPrefabSequenceEditor) {}
	SLATE_END_ARGS();
	void Construct(const FArguments& InArgs);

	void AssignLPrefabSequenceComponent(TWeakObjectPtr<ULPrefabSequenceComponent> InSequenceComponent);
	ULPrefabSequence* GetLPrefabSequence() const;
	ULPrefabSequenceComponent* GetSequenceComponent()const { return WeakSequenceComponent.Get(); }
	void RefreshAnimationList();
	void OnEditingPrefabChanged(AActor* RootActor);
private:
	TWeakObjectPtr<ULPrefabSequenceComponent> WeakSequenceComponent;
	FDelegateHandle OnObjectsReplacedHandle;
	FDelegateHandle EditingPrefabChangedHandle;
	FDelegateHandle OnBeforeApplyPrefabHandle;
	void OnBeforeApplyPrefab(ULPrefabHelperObject* InObject);

	TSharedPtr<SLPrefabSequenceEditorWidget> PrefabSequenceEditor;

	TSharedPtr<SListView<TSharedPtr<FWidgetAnimationListItem>>> AnimationListView;
	TArray< TSharedPtr<FWidgetAnimationListItem> > Animations;
	int32 CurrentSelectedAnimationIndex = 0;
	TSharedRef<ITableRow> OnGenerateRowForAnimationListView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedRef<STableViewBase>& InOwnerTableView);
	void OnAnimationListViewSelectionChanged(TSharedPtr<FWidgetAnimationListItem> InListItem, ESelectInfo::Type InSelectInfo);
	void OnItemScrolledIntoView(TSharedPtr<FWidgetAnimationListItem> InListItem, const TSharedPtr<ITableRow>& InWidget) const;
	FReply OnNewAnimationClicked();
	TSharedPtr<class SSearchBox> SearchBoxPtr;
	void OnAnimationListViewSearchChanged(const FText& InSearchText);
	TSharedPtr<SWidget> OnContextMenuOpening()const;
	TSharedPtr<FUICommandList> CommandList;
	void CreateCommandList();
	void OnDuplicateAnimation();
	void OnDeleteAnimation();
	void OnRenameAnimation();
	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap);
};