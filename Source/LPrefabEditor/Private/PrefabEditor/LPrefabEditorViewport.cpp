// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorViewport.h"
#include "LPrefabEditorViewportClient.h"
#include "LPrefabEditor.h"
#include "LPrefabEditorViewportToolbar.h"

#define LOCTEXT_NAMESPACE "LPrefabEditorViewport"

void SLPrefabEditorViewport::Construct(const FArguments& InArgs, TSharedPtr<FLPrefabEditor> InPrefabEditor, EViewModeIndex InViewMode)
{
	this->PrefabEditorPtr = InPrefabEditor;
	this->ViewMode = InViewMode;
	SEditorViewport::Construct(SEditorViewport::FArguments());
}
void SLPrefabEditorViewport::BindCommands()
{
	SEditorViewport::BindCommands();
}
TSharedRef<FEditorViewportClient> SLPrefabEditorViewport::MakeEditorViewportClient()
{
	EditorViewportClient = MakeShareable(new FLPrefabEditorViewportClient(this->PrefabEditorPtr.Pin()->GetPreviewScene(), this->PrefabEditorPtr, SharedThis(this)));
	EditorViewportClient->ViewportType = LVT_Perspective;
	EditorViewportClient->bSetListenerPosition = false;
	EditorViewportClient->SetRealtime(true);
	EditorViewportClient->SetShowStats(true);
	EditorViewportClient->VisibilityDelegate.BindLambda([]() {return true; });
	EditorViewportClient->SetViewMode(ViewMode);
	return EditorViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SLPrefabEditorViewport::MakeViewportToolbar()
{
	return SNew(SLPrefabEditorViewportToolbar, SharedThis(this));
}
EVisibility SLPrefabEditorViewport::GetTransformToolbarVisibility() const
{
	return EVisibility::Visible;
}
void SLPrefabEditorViewport::OnFocusViewportToSelection()
{
	EditorViewportClient->FocusViewportToTargets();
}

TSharedRef<SEditorViewport> SLPrefabEditorViewport::GetViewportWidget()
{
	return SharedThis(this);
}
TSharedPtr<FExtender> SLPrefabEditorViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}
void SLPrefabEditorViewport::OnFloatingButtonClicked()
{

}

FReply SLPrefabEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return PrefabEditorPtr.Pin()->TryHandleAssetDragDropOperation(DragDropEvent);
}

#undef LOCTEXT_NAMESPACE