// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class FLPrefabEditorModuleCommands : public TCommands<FLPrefabEditorModuleCommands>
{
public:

	FLPrefabEditorModuleCommands();
	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> CopyActor;
	TSharedPtr<FUICommandInfo> PasteActor;
	TSharedPtr<FUICommandInfo> CutActor;
	TSharedPtr<FUICommandInfo> DuplicateActor;
	TSharedPtr<FUICommandInfo> DestroyActor;
	TSharedPtr<FUICommandInfo> ToggleSpatiallyLoaded;

	TSharedPtr<FUICommandInfo> CopyComponentValues;
	TSharedPtr<FUICommandInfo> PasteComponentValues;

	TSharedPtr<FUICommandInfo> ToggleLGUIInfoColume;
	TSharedPtr<FUICommandInfo> ForceGC;
};