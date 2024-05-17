// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LPrefab.h"
#include "Misc/PackageName.h"
#include "EditorStyleSet.h"
#include "EditorFramework/AssetImportData.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "AssetNotifications.h"
#include "Algo/Transform.h"
#include "PrefabSystem/LPrefab.h"
#include "PrefabEditor/LPrefabEditor.h"
#include "LPrefabEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LPrefab"

FAssetTypeActions_LPrefab::FAssetTypeActions_LPrefab(EAssetTypeCategories::Type InAssetType)
: FAssetTypeActions_Base(), AssetType(InAssetType)
{

}

FText FAssetTypeActions_LPrefab::GetName() const
{
	return LOCTEXT("Name", "LPrefab");
}

UClass* FAssetTypeActions_LPrefab::GetSupportedClass() const
{
	return ULPrefab::StaticClass();
}

void FAssetTypeActions_LPrefab::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	//FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);

	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (auto LPrefab = Cast<ULPrefab>(*ObjIt))
		{
			TSharedRef<FLPrefabEditor> NewPrefabEditor(new FLPrefabEditor());
			NewPrefabEditor->InitPrefabEditor(Mode, EditWithinLevelEditor, LPrefab);
		}
	}
}

uint32 FAssetTypeActions_LPrefab::GetCategories()
{
	return AssetType;
}

bool FAssetTypeActions_LPrefab::CanFilter()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
