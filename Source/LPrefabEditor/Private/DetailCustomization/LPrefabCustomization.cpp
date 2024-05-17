// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LPrefabCustomization.h"
#include "PrefabSystem/LPrefab.h"
#include "PrefabSystem/LPrefabHelperObject.h"
#include "PrefabSystem/LPrefabManager.h"
#include "LPrefabUtils.h"
#include "LPrefabEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "LPrefabEditorTools.h"

#define LOCTEXT_NAMESPACE "LPrefabCustomization"

TSharedRef<IDetailCustomization> FLPrefabCustomization::MakeInstance()
{
	return MakeShareable(new FLPrefabCustomization);
}

void FLPrefabCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULPrefab>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LPrefabEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& category = DetailBuilder.EditCategory("LPrefab");

	//category.AddCustomRow(LOCTEXT("Edit prefab", "Edit prefab"))
	//	.NameContent()
	//	[
	//		SNew(SButton)
	//		.Text(LOCTEXT("Edit prefab", "Edit prefab"))
	//		.ToolTipText(LOCTEXT("EditPrefab_Tooltip", "Edit this prefab in level editor, use selected actor as parent."))
	//		.OnClicked(this, &FLPrefabCustomization::OnClickEditPrefabButton)
	//	]
	//	;

	//show prefab version
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULPrefab, EngineMajorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULPrefab, EngineMinorVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULPrefab, PrefabVersion))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&DetailBuilder] {DetailBuilder.ForceRefreshDetails(); }));
	category.AddCustomRow(LOCTEXT("EngineVersion", "Engine Version"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("EngineVersion", "Engine Version"))
			.ToolTipText(LOCTEXT("EngineVersionTooltip", "Engine's version when creating this prefab."))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(4, 2))
				[
					SNew(STextBlock)
					.Text(this, &FLPrefabCustomization::GetEngineVersionText)
					.ToolTipText(LOCTEXT("EngineVersionTooltip", "Engine's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLPrefabCustomization::GetEngineVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixEngineVersion", "Fix it"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteButton)
				.Visibility(this, &FLPrefabCustomization::ShouldShowFixEngineVersionButton)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixAllEngineVersion", "Fix all"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteAllButton)
				.Visibility(this, &FLPrefabCustomization::ShouldShowFixEngineVersionButton)
			]
		]
		;
	category.AddCustomRow(LOCTEXT("PrefabVersion", "Prefab Version"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("PrefabVersion", "Prefab Version"))
			.ToolTipText(LOCTEXT("PrefabVersionTooltip", "LPrefab system's version when creating this prefab."))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(4, 2))
				[
					SNew(STextBlock)
					.Text(this, &FLPrefabCustomization::GetPrefabVersionText)
					.ToolTipText(LOCTEXT("PrefabVersionTooltip", "LPrefab system's version when creating this prefab."))
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.ColorAndOpacity(this, &FLPrefabCustomization::GetPrefabVersionTextColorAndOpacity)
					.AutoWrapText(true)
				]
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixPrefabVersion", "Fix it"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLPrefabCustomization::ShouldShowFixPrefabVersionButton)
			]
			+SHorizontalBox::Slot()
			.MaxWidth(80)
			[
				SNew(SButton)
				.Text(LOCTEXT("FixAllPrefabVersion", "Fix all"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteAllButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLPrefabCustomization::ShouldShowFixPrefabVersionButton)
			]
		]
		;

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(ULPrefab, PrefabHelperObject));
	auto PrefabHelperObjectProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULPrefab, PrefabHelperObject));
	category.AddCustomRow(LOCTEXT("PrefabHelperObject", "PrefabHelperObject"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AgentObjectsWidgetName", "AgentObjects"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.Padding(FMargin(4, 2))
				[
					SNew(STextBlock)
					.Text(this, &FLPrefabCustomization::AgentObjectText)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("FixPrefabHelperObject", "Fix"))
				.ToolTipText(LOCTEXT("FixAgentRootActor_Tooltip", "Missing agent objects! This may cause cook & package fail. Click to fix it. Because we can't fix it in cook thread, so you need to do it manually."))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreateAgentObjects)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.Visibility(this, &FLPrefabCustomization::ShouldShowFixAgentObjectsButton)
			]
		]
	;
	category.AddCustomRow(LOCTEXT("AdditionalButton", "Additional Button"), true)
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RecreateThis", "Recreate this prefab"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(LOCTEXT("RecreateAll", "Recreate all prefabs"))
				.OnClicked(this, &FLPrefabCustomization::OnClickRecreteAllButton)
				.HAlign(EHorizontalAlignment::HAlign_Center)
			]
		]
		;
}
FText FLPrefabCustomization::GetEngineVersionText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return FText::FromString(FString::Printf(TEXT("%d.%d"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion));
		}
		else
		{
			return FText::Format(LOCTEXT("PrefabEngineVersionError", "{0}.{1} (This prefab is made by a different engine version.)"), TargetScriptPtr->EngineMajorVersion, TargetScriptPtr->EngineMinorVersion);
		}
	}
	else
	{
		return LOCTEXT("Error", "Error");
	}
}
FText FLPrefabCustomization::GetPrefabVersionText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LPREFAB_CURRENT_VERSION)
		{
			return FText::FromString(FString::Printf(TEXT("%d"), TargetScriptPtr->PrefabVersion));
		}
		else
		{
			return FText::Format(LOCTEXT("PrefabSystemVersionError", "{0} (This prefab is made by a different prefab system version.)"), TargetScriptPtr->PrefabVersion);
		}
	}
	else
	{
		return LOCTEXT("Error", "Error");
	}
}
EVisibility FLPrefabCustomization::ShouldShowFixEngineVersionButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Visible;
		}
	}
	else
	{
		return EVisibility::Hidden;
	}
}
FSlateColor FLPrefabCustomization::GetEngineVersionTextColorAndOpacity()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->EngineMajorVersion == ENGINE_MAJOR_VERSION && TargetScriptPtr->EngineMinorVersion == ENGINE_MINOR_VERSION)
		{
			return FSlateColor::UseForeground();
		}
		else
		{
			return FLinearColor::Yellow;
		}
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}
FSlateColor FLPrefabCustomization::GetPrefabVersionTextColorAndOpacity()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LPREFAB_CURRENT_VERSION)
		{
			return FSlateColor::UseForeground();
		}
		else
		{
			return FLinearColor::Yellow;
		}
	}
	else
	{
		return FSlateColor::UseForeground();
	}
}
EVisibility FLPrefabCustomization::ShouldShowFixPrefabVersionButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion == LPREFAB_CURRENT_VERSION)
		{
			return EVisibility::Hidden;
		}
		else
		{
			return EVisibility::Visible;
		}
	}
	else
	{
		return EVisibility::Hidden;
	}
}
EVisibility FLPrefabCustomization::ShouldShowFixAgentObjectsButton()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive
			&& (!IsValid(TargetScriptPtr->PrefabHelperObject) || !IsValid(TargetScriptPtr->PrefabHelperObject->LoadedRootActor))
			)
		{
			return EVisibility::Visible;
		}
		return EVisibility::Hidden;
	}
	else
	{
		return EVisibility::Hidden;
	}
}

FText FLPrefabCustomization::AgentObjectText()const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive
			&& (!IsValid(TargetScriptPtr->PrefabHelperObject) || !IsValid(TargetScriptPtr->PrefabHelperObject->LoadedRootActor))
			)
		{
			return LOCTEXT("AgentObjectNotValid", "NotValid");
		}
	}
	return LOCTEXT("AgentObjectValid", "Valid");
}

FReply FLPrefabCustomization::OnClickRecreteButton()
{
	if (auto Prefab = TargetScriptPtr.Get())
	{
		Prefab->RecreatePrefab();
	}
	return FReply::Handled();
}
FReply FLPrefabCustomization::OnClickRecreteAllButton()
{
	auto World = ULPrefabManagerObject::GetPreviewWorldForPrefabPackage();
	if (!IsValid(World))
	{
		UE_LOG(LPrefabEditor, Error, TEXT("[FLPrefabCustomization::OnClickRecreteButton]Can not get World! This is wired..."));
	}
	else
	{
		auto AllPrefabs = LPrefabEditorTools::GetAllPrefabArray();
		for (auto Prefab : AllPrefabs)
		{
			if (
				Prefab->EngineMajorVersion != ENGINE_MAJOR_VERSION || Prefab->EngineMinorVersion != ENGINE_MINOR_VERSION
				|| Prefab->PrefabVersion != LPREFAB_CURRENT_VERSION
				)
			{
				Prefab->RecreatePrefab();
			}
		}
	}
	return FReply::Handled();
}
FReply FLPrefabCustomization::OnClickEditPrefabButton()
{
	return FReply::Handled();
}
FReply FLPrefabCustomization::OnClickRecreateAgentObjects()
{
	auto AllPrefabs = LPrefabEditorTools::GetAllPrefabArray();
	for (auto Prefab : AllPrefabs)
	{
		Prefab->MakeAgentObjectsInPreviewWorld();
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE