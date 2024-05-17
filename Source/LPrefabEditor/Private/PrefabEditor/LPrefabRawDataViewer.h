// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FLPrefabEditor;

class SLPrefabRawDataViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLPrefabRawDataViewer) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FLPrefabEditor> InPrefabEditorPtr, UObject* InObject);
private:
	TWeakPtr<FLPrefabEditor> PrefabEditorPtr;
	TSharedPtr<IDetailsView> DescriptorDetailView;
};
