// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorDetails.h"
#include "Engine/Selection.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "ISCSEditorUICustomization.h"
#include "GameFramework/Actor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Misc/NotifyHook.h"
#include "LPrefabEditor.h"
#include "DetailLayoutBuilder.h"
#include "LPrefabOverrideDataViewer.h"
#include "PrefabSystem/LPrefab.h"
#include "LPrefabEditorTools.h"
#include "SSubobjectEditorModule.h"
#include "SSubobjectInstanceEditor.h"

#define LOCTEXT_NAMESPACE "LPrefabEditorDetailTab"

class LGUISCSEditorUICustomization : public ISCSEditorUICustomization
{
	virtual bool HideBlueprintButtons() const override { return true; }
};

void SLPrefabEditorDetails::Construct(const FArguments& Args, TSharedPtr<FLPrefabEditor> InPrefabEditor)
{
	PrefabEditorPtr = InPrefabEditor;

	USelection::SelectionChangedEvent.AddRaw(this, &SLPrefabEditorDetails::OnEditorSelectionChanged);

    FPropertyEditorModule& PropPlugin = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bUpdatesFromSelection = true;
    DetailsViewArgs.bLockable = true;
    DetailsViewArgs.NotifyHook = GUnrealEd;
    DetailsViewArgs.ViewIdentifier = FName(TEXT("LPrefabEditor"));
    DetailsViewArgs.bCustomNameAreaLocation = true;
    DetailsViewArgs.bCustomFilterAreaLocation = false;
    DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ComponentsAndActorsUseNameArea;
    DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bAllowSearch = true;
    //DetailsViewArgs.HostCommandList = InCommandList;

    DetailsView = PropPlugin.CreateDetailView(DetailsViewArgs);
    DetailsView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SLPrefabEditorDetails::IsPropertyReadOnly));

	FModuleManager::LoadModuleChecked<FSubobjectEditorModule>("SubobjectEditor");
	SubobjectEditor = SNew(SSubobjectInstanceEditor)
		.AllowEditing(this, &SLPrefabEditorDetails::IsEditorAllowEditing)
		.ObjectContext(this, &SLPrefabEditorDetails::GetActorContextAsObject)
		.OnSelectionUpdated(this, &SLPrefabEditorDetails::OnEditorTreeViewSelectionChanged)
		.OnItemDoubleClicked(this, &SLPrefabEditorDetails::OnEditorTreeViewItemDoubleClicked);

	
	TSharedPtr<ISCSEditorUICustomization> Customization = MakeShared<LGUISCSEditorUICustomization>();
	SubobjectEditor->SetUICustomization(Customization);
	auto ButtonBox = SubobjectEditor->GetToolButtonsBox().ToSharedRef();
	DetailsView->SetNameAreaCustomContent(ButtonBox);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				DetailsView->GetNameAreaWidget().ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(this, &SLPrefabEditorDetails::GetPrefabButtonVisibility)
				.IsEnabled(this, &SLPrefabEditorDetails::IsPrefabButtonEnable)
				.HeightOverride(this, &SLPrefabEditorDetails::GetPrefabButtonHeight)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4, 0))
					[
						SNew(SBox)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("PrefabFunctions", "Prefab"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=]() {
							PrefabEditorPtr.Pin()->OpenSubPrefab(CachedActor.Get());
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("OpenPrefab", "Open"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=]() {
							PrefabEditorPtr.Pin()->SelectSubPrefab(CachedActor.Get());
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SelectPrefab", "Select"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					.Padding(FMargin(2, 0))
					[
						SNew(SComboButton)
						.HasDownArrow(true)
						.ToolTipText(LOCTEXT("PrefabOverride", "Edit override parameters for this prefab"))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
						.MenuContent()
						[
							SNew(SBox)
							.Padding(FMargin(4, 4))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											SAssignNew(OverrideParameterEditor, SLPrefabOverrideDataViewer, PrefabEditorPtr.Pin()->GetPrefabManagerObject())
											.AfterRevertPrefab_Lambda([=](ULPrefab* PrefabAsset) {
												RefreshOverrideParameter();
												})
											.AfterApplyPrefab_Lambda([=](ULPrefab* PrefabAsset){
												RefreshOverrideParameter();
												LPrefabEditorTools::RefreshLevelLoadedPrefab(PrefabAsset);
												LPrefabEditorTools::RefreshOnSubPrefabChange(PrefabAsset);
												LPrefabEditorTools::RefreshOpenedPrefabEditor(PrefabAsset);
												})
										]
									]
								]
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 2))
			.AutoHeight()
			[
				SubobjectEditor.ToSharedRef()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 2))
			[
				DetailsView.ToSharedRef()
			]
		];
}

SLPrefabEditorDetails::~SLPrefabEditorDetails()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}

bool SLPrefabEditorDetails::IsPrefabButtonEnable()const
{
	if (PrefabEditorPtr.IsValid() && CachedActor.IsValid())
	{
		return PrefabEditorPtr.Pin()->ActorIsSubPrefabRoot(CachedActor.Get());
	}
	return false;
}

FOptionalSize SLPrefabEditorDetails::GetPrefabButtonHeight()const
{
	return IsPrefabButtonEnable() ? 26 : 0;
}

EVisibility SLPrefabEditorDetails::GetPrefabButtonVisibility()const
{
	return IsPrefabButtonEnable() ? EVisibility::Visible : EVisibility::Hidden;
}

bool SLPrefabEditorDetails::IsEditorAllowEditing()const
{
	if (PrefabEditorPtr.IsValid() && CachedActor.IsValid())
	{
		return !PrefabEditorPtr.Pin()->ActorBelongsToSubPrefab(CachedActor.Get());
	}
	return true;
}

void SLPrefabEditorDetails::RefreshOverrideParameter()
{
	FLSubPrefabData SubPrefabData = PrefabEditorPtr.Pin()->GetSubPrefabDataForActor(CachedActor.Get());
	OverrideParameterEditor->RefreshDataContent(SubPrefabData.ObjectOverrideParameterArray, nullptr);
}

AActor* SLPrefabEditorDetails::GetActorContext() const
{
	return CachedActor.Get();
}

void SLPrefabEditorDetails::OnEditorSelectionChanged(UObject* Object)
{
	// Make sure the selection set that changed is relevant to us
	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			if (Actor->GetWorld() != PrefabEditorPtr.Pin()->GetWorld())
			{
				return;
			}

			CachedActor = Actor;
			RefreshOverrideParameter();
			if (SubobjectEditor)
			{
				SubobjectEditor->ClearSelection();
				SubobjectEditor->UpdateTree();
			}
		}

		TArray<UObject*> SeletedObjectList;
		for (int32 i = 0; i < Selection->Num(); i++)
		{
			UObject* SeletedObject = Selection->GetSelectedObject(i);
			if (SeletedObject)
			{
				if (SeletedObject->GetWorld() != PrefabEditorPtr.Pin()->GetWorld())
				{
					continue;
				}

				SeletedObjectList.Add(SeletedObject);
			}
		}

		if (SeletedObjectList.Num() > 0)
		{
			if (DetailsView)
			{
				DetailsView->SetObjects(SeletedObjectList);
			}
		}
	}
}

void SLPrefabEditorDetails::OnEditorTreeViewSelectionChanged(const TArray<FSubobjectEditorTreeNodePtrType>& SelectedNodes)
{
	if (SelectedNodes.Num() > 0)
	{
		TArray<UObject*> SelectedObjects;
		TArray<UActorComponent*> SelectedComponents;
		for (auto& Node : SelectedNodes)
		{
			if (Node.IsValid())
			{
				UObject* Object = const_cast<UObject*>(Node->GetObject());
				if (Object)
				{
					SelectedObjects.Add(Object);
					if (auto Comp = Cast<UActorComponent>(Object))
					{
						SelectedComponents.Add(Comp);
					}
				}
			}
		}

		if (SelectedObjects.Num() > 0 && DetailsView.IsValid())
		{
			DetailsView->SetObjects(SelectedObjects);
		}
		if (SelectedComponents.Num() > 0)
		{
			GEditor->SelectNone(true, true);
			for (auto Comp : SelectedComponents)
			{
				GEditor->SelectComponent(Comp, true, true);
			}
		}
		else
		{
			GEditor->SelectNone(true, true);
		}
	}
}

void SLPrefabEditorDetails::OnEditorTreeViewItemDoubleClicked(const FSubobjectEditorTreeNodePtrType ClickedNode)
{

}

bool SLPrefabEditorDetails::IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent)
{
	return false;
}

#undef LOCTEXT_NAMESPACE