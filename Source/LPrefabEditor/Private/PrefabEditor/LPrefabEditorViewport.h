// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "SEditorViewport.h"
#include "SCommonEditorViewportToolbarBase.h"

class FLPrefabEditor;
class FLPrefabEditorViewportClient;

//Encapsulates a simple scene setup for preview or thumbnail rendering.
class SLPrefabEditorViewport : public SEditorViewport, public ICommonEditorViewportToolbarInfoProvider
{
public:
	SLATE_BEGIN_ARGS(SLPrefabEditorViewport) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLPrefabEditor> InPrefabEditor, EViewModeIndex InViewMode);

	// SEditorViewport interface
	virtual void BindCommands() override;
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	virtual void OnFocusViewportToSelection() override;
	// End of SEditorViewport interface

	// ICommonEditorViewportToolbarInfoProvider interface
	virtual TSharedRef<class SEditorViewport> GetViewportWidget() override;
	virtual TSharedPtr<FExtender> GetExtenders() const override;
	virtual void OnFloatingButtonClicked() override;
	// End of ICommonEditorViewportToolbarInfoProvider interface

	// SWidget interface
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

private:
	// Pointer back to owning sprite editor instance (the keeper of state)
	TWeakPtr<FLPrefabEditor> PrefabEditorPtr;
	EViewModeIndex ViewMode = EViewModeIndex::VMI_Lit;

	// Viewport client
	TSharedPtr<FLPrefabEditorViewportClient> EditorViewportClient;
};