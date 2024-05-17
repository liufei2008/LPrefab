// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LevelEditorMenuExtensions/LPrefabLevelEditorExtensions.h"
#include "EngineModule.h"
#include "Engine/EngineTypes.h"
#include "LPrefabEditorStyle.h"
#include "LevelEditor.h"
#include "LPrefabEditorModule.h"
#include "LPrefabEditorTools.h"

#define LOCTEXT_NAMESPACE "FLPrefabLevelEditorExtensions"

//////////////////////////////////////////////////////////////////////////

FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelEditorMenuExtenderDelegate;
FDelegateHandle LevelEditorMenuExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////

class FLPrefabLevelEditorExtensions_Impl
{
public:
	static void CreateLGUISubMenu(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.AddWidget(
			FLPrefabEditorModule::Get().MakeEditorToolsMenu(false, false, false, false, false)
			, FText::GetEmpty()
		);
	}
	static void CreateHelperButtons(FMenuBuilder& MenuBuilder)
	{
		MenuBuilder.BeginSection("LPrefab", LOCTEXT("LPrefabLevelEditorHeading", "LPrefab"));
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("LPrefabEditorTools", "LPrefab Editor Tools"),
				FText::GetEmpty(),
				FNewMenuDelegate::CreateStatic(&FLPrefabLevelEditorExtensions_Impl::CreateLGUISubMenu),
				FUIAction(),
				NAME_None, EUserInterfaceActionType::None
			);
		}
		MenuBuilder.EndSection();
	}
	static TSharedRef<FExtender> OnExtendLevelEditorMenu(const TSharedRef<FUICommandList> CommandList, TArray<AActor*> SelectedActors)
	{
		TSharedRef<FExtender> Extender(new FExtender());
		if (SelectedActors.Num() == 1//only support one selection
			&& IsValid(SelectedActors[0])
			&& LPrefabEditorTools::IsActorCompatibleWithLGUIToolsMenu(SelectedActors[0])//only show menu with supported actor
			)
		{
			Extender->AddMenuExtension(
				"ActorType",
				EExtensionHook::Before,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FLPrefabLevelEditorExtensions_Impl::CreateHelperButtons)
			);
		}
		return Extender;
	}
};

// FLPrefabLevelEditorExtensions

void FLPrefabLevelEditorExtensions::InstallHooks()
{
	LevelEditorMenuExtenderDelegate = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&FLPrefabLevelEditorExtensions_Impl::OnExtendLevelEditorMenu);

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	MenuExtenders.Add(LevelEditorMenuExtenderDelegate);
	LevelEditorMenuExtenderDelegateHandle = MenuExtenders.Last().GetHandle();
}

void FLPrefabLevelEditorExtensions::RemoveHooks()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([&](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Delegate) {
			return Delegate.GetHandle() == LevelEditorMenuExtenderDelegateHandle;
		});
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE