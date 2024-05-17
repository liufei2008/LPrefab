// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorViewportToolbar.h"
#include "LPrefabEditorViewport.h"

#define LOCTEXT_NAMESPACE "SLPrefabEditorViewportToolbar"

///////////////////////////////////////////////////////////
// SSpriteEditorViewportToolbar

void SLPrefabEditorViewportToolbar::Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
	SCommonEditorViewportToolbarBase::Construct(SCommonEditorViewportToolbarBase::FArguments(), InInfoProvider);
}

TSharedRef<SWidget> SLPrefabEditorViewportToolbar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();
	
	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowSockets);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowPivot);

		//ShowMenuBuilder.AddMenuSeparator();

		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowGrid);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowBounds);
		//ShowMenuBuilder.AddMenuEntry(FSpriteGeometryEditCommands::Get().SetShowNormals);

		//ShowMenuBuilder.AddMenuSeparator();

		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowCollision);
		//ShowMenuBuilder.AddMenuEntry(FSpriteEditorCommands::Get().SetShowMeshEdges);
	}

	return ShowMenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
