// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLPrefabEditor;
struct FLPrefabOverrideParameterData;
class ULPrefabHelperObject;
class AActor;
class ULPrefab;

DECLARE_DELEGATE_OneParam(FLPrefabOverrideDataViewer_AfterRevertPrefab, ULPrefab*);
DECLARE_DELEGATE_OneParam(FLPrefabOverrideDataViewer_AfterApplyPrefab, ULPrefab*);

class SLPrefabOverrideDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPrefabOverrideDataViewer) {}
		SLATE_EVENT(FLPrefabOverrideDataViewer_AfterRevertPrefab, AfterRevertPrefab)
		SLATE_EVENT(FLPrefabOverrideDataViewer_AfterApplyPrefab, AfterApplyPrefab)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, ULPrefabHelperObject* InPrefabHelperObject);
		/**
		 * Refresh override parameter data.
		 * @param ObjectOverrideParameterArray All override parameter data in this sub prefab.
		 * @param InReferenceActor Pass in actor in sub prefab means only show override parameters of the actor or it's components. Pass in nullptr menas show all.
		 */
	void RefreshDataContent(TArray<FLPrefabOverrideParameterData> ObjectOverrideParameterArray, AActor* InReferenceActor);
	void SetPrefabHelperObject(ULPrefabHelperObject* InPrefabHelperObject);
private:
	FLPrefabOverrideDataViewer_AfterRevertPrefab AfterRevertPrefab;
	FLPrefabOverrideDataViewer_AfterApplyPrefab AfterApplyPrefab;

	TSharedPtr<SVerticalBox> RootContentVerticalBox;
	TWeakObjectPtr<ULPrefabHelperObject> PrefabHelperObject;
};
