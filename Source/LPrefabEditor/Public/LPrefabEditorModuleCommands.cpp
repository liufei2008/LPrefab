// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorModuleCommands.h"
#include "LPrefabEditorModule.h"
#include "LPrefabEditorStyle.h"

#define LOCTEXT_NAMESPACE "FLPrefabEditorModuleCommands"

FLPrefabEditorModuleCommands::FLPrefabEditorModuleCommands()
	: TCommands<FLPrefabEditorModuleCommands>(TEXT("LPrefabEditorModule"), NSLOCTEXT("Contexts", "LPrefabEditorModule", "LPrefabEditor Plugin"), NAME_None, FLPrefabEditorStyle::GetStyleSetName())
{
}
void FLPrefabEditorModuleCommands::RegisterCommands()
{
	UI_COMMAND(CopyActor, "Copy Actors", "Copy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::C, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(PasteActor, "Paste Actors", "Paste actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::V, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(CutActor, "Cut Actors", "Cut actors with hierarchy", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(DuplicateActor, "Duplicate Actors", "Duplicate selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord(EKeys::D, EModifierKey::Shift | EModifierKey::Alt));
	UI_COMMAND(DestroyActor, "Destroy Actors", "Destroy selected actors with hierarchy", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ToggleSpatiallyLoaded, "Toggle Actors IsSpatiallyLoaded", "Toggle selected actor's IsSpatiallyLoaded property for WorldPartition", EUserInterfaceActionType::ToggleButton, FInputChord());
	
	UI_COMMAND(CopyComponentValues, "Copy Component Values", "Copy selected component values", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(PasteComponentValues, "Paste Component Values", "Paste values to selected component", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(ToggleLGUIInfoColume, "Show LGUI column in SceneOutliner", "Show LGUI column in SceneOutliner", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ForceGC, "ForceGC", "Force garbage collection immediately, this is useful in some test work", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
