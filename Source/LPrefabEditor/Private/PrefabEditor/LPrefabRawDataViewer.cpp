// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabRawDataViewer.h"
#include "PrefabSystem/LPrefab.h"
#include "LPrefabEditor.h"

#define LOCTEXT_NAMESPACE "LPrefabRawDataViewer"

void SLPrefabRawDataViewer::Construct(const FArguments& InArgs, TSharedPtr<FLPrefabEditor> InPrefabEditorPtr, UObject* InObject)
{
	PrefabEditorPtr = InPrefabEditorPtr;
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
	DescriptorDetailView->SetObject(InObject);
	ChildSlot
		[
			DescriptorDetailView.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
