// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LPrefabEditorStyle.h"

class ULPrefab;

class FLPrefabEditorCommand : public TCommands<FLPrefabEditorCommand>
{
public:
	FLPrefabEditorCommand()
		: TCommands<FLPrefabEditorCommand>(
			TEXT("LPrefabEditor"), // Context name for fast lookup
			NSLOCTEXT("Contexts", "LPrefabEditor", "LGUI Prefab Editor"), // Localized context name for displaying
			NAME_None, // Parent
			FLPrefabEditorStyle::Get().GetStyleSetName() // Icon Style Set
			)
	{
	}

	// TCommand<> interface
	virtual void RegisterCommands() override;
	// End of TCommand<> interface

public:
	TSharedPtr<FUICommandInfo> Apply;
	TSharedPtr<FUICommandInfo> RawDataViewer;
	TSharedPtr<FUICommandInfo> OpenPrefabHelperObject;
	TSharedPtr<FUICommandInfo> CopyActor;
	TSharedPtr<FUICommandInfo> PasteActor;
	TSharedPtr<FUICommandInfo> CutActor;
	TSharedPtr<FUICommandInfo> DuplicateActor;
	TSharedPtr<FUICommandInfo> DestroyActor;
};