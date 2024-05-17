// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "ContentBrowserExtensions/LPrefabContentBrowserExtensions.h"
#include "EngineModule.h"
#include "Engine/EngineTypes.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "LPrefabEditorStyle.h"
#include "DataFactory/LPrefabFactory.h"
#include "PrefabSystem/LPrefab.h"

#define LOCTEXT_NAMESPACE "LPrefabContentBrowserExtensions"

//////////////////////////////////////////////////////////////////////////

FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
FDelegateHandle ContentBrowserExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////
// FCreateSpriteFromTextureExtension

#include "IAssetTools.h"
#include "AssetToolsModule.h"


//////////////////////////////////////////////////////////////////////////
// FLPrefabContentBrowserExtensions_Impl

class FLPrefabContentBrowserExtensions_Impl
{
public:
	static void CreatePrefabActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<ULPrefab*> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("PrefabActionsSubMenuLabel", "LPrefab"),
			LOCTEXT("PrefabActionsSubMenuToolTip", "Prefab-related actions for this prefab."),
			FNewMenuDelegate::CreateStatic(&FLPrefabContentBrowserExtensions_Impl::PopulatePrefabActionMenu, SelectedAssets),
			false,
			FSlateIcon(FLPrefabEditorStyle::GetStyleSetName(), "LPrefabEditor.PrefabDataAction")
		);
	}

	static void PopulatePrefabActionMenu(FMenuBuilder& MenuBuilder, TArray<ULPrefab*> SelectedAssets)
	{
		struct LOCAL
		{
			static void CreatePrefabVariant(TArray<ULPrefab*> Prefabs)
			{
				FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
				FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

				TArray<UObject*> ObjectsToSync;

				for (auto Prefab : Prefabs)
				{
					// Create the factory used to generate the prefab
					auto PrefabFactory = NewObject<ULPrefabFactory>();
					PrefabFactory->SourcePrefab = Prefab;

					// Create the prefab
					FString Name;
					FString PackageName;

					// Get a unique name for the prefab
					const FString DefaultSuffix = TEXT("_Variant");
					AssetToolsModule.Get().CreateUniqueAssetName(Prefab->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);
					const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

					if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, PackagePath, ULPrefab::StaticClass(), PrefabFactory))
					{
						ObjectsToSync.Add(NewAsset);
					}
				}

				if (ObjectsToSync.Num() > 0)
				{
					ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
				}
			}
		};

		const FName LGUIStyleSetName = FLPrefabEditorStyle::GetStyleSetName();
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreatePrefabVariant", "Create PrefabVariant"),
			LOCTEXT("CreatePrefabVariant_Tooltip", "Create variant prefab using this prefab."),
			FSlateIcon(LGUIStyleSetName, "LPrefabEditor.PrefabDataAction"),
			FUIAction(FExecuteAction::CreateStatic(&LOCAL::CreatePrefabVariant, SelectedAssets)),
			NAME_None,
			EUserInterfaceActionType::Button);
	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		// Run thru the assets to determine if any meet our criteria
		TArray<ULPrefab*> Prefabs;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			auto AssetObject = Asset.GetAsset();
			if (auto Prefab = Cast<ULPrefab>(AssetObject))
			{
				Prefabs.Add(Prefab);
			}
		}

		if (Prefabs.Num() > 0)
		{
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FLPrefabContentBrowserExtensions_Impl::CreatePrefabActionsSubMenu, Prefabs));
		}

		return Extender;
	}

	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}
};

//////////////////////////////////////////////////////////////////////////
// FLPrefabContentBrowserExtensions

void FLPrefabContentBrowserExtensions::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FLPrefabContentBrowserExtensions_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FLPrefabContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FLPrefabContentBrowserExtensions::RemoveHooks()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FLPrefabContentBrowserExtensions_Impl::GetExtenderDelegates();
		CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE