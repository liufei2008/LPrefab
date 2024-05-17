// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SSubobjectEditor.h"
#pragma once

class FLPrefabEditor;
class AActor;

/**
 * 
 */
class SLPrefabEditorDetails : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPrefabEditorDetails)
    {
    }

    SLATE_END_ARGS()

    /** Widget constructor */
    void Construct(const FArguments& Args, TSharedPtr<FLPrefabEditor> InPrefabEditor);

	virtual ~SLPrefabEditorDetails();
public:
	void RefreshOverrideParameter();
private:
	AActor* GetActorContext() const;
	UObject* GetActorContextAsObject() const { return GetActorContext(); }
	void OnEditorSelectionChanged(UObject* Object);
	void OnEditorTreeViewSelectionChanged(const TArray<FSubobjectEditorTreeNodePtrType>& SelectedNodes);
	void OnEditorTreeViewItemDoubleClicked(const FSubobjectEditorTreeNodePtrType ClickedNode);

	TWeakPtr<FLPrefabEditor> PrefabEditorPtr;

	bool IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent);
	bool IsPrefabButtonEnable()const;
	FOptionalSize GetPrefabButtonHeight()const;
	EVisibility GetPrefabButtonVisibility()const;
	bool IsEditorAllowEditing()const;

	TSharedPtr<class SLPrefabOverrideDataViewer> OverrideParameterEditor;
	TSharedPtr<class IDetailsView> DetailsView;
	TSharedPtr<class SBox> ComponentsBox;
	TSharedPtr<class SSubobjectEditor> SubobjectEditor;
	TWeakObjectPtr<AActor> CachedActor;
};
