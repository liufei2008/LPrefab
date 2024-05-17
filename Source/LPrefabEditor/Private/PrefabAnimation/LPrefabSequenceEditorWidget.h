// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "WorkflowOrientedApp/WorkflowUObjectDocuments.h"


class FBlueprintEditor;
class SLPrefabSequenceEditorWidgetImpl;
class ULPrefabSequence;
class ULPrefabSequenceComponent;


class SLPrefabSequenceEditorWidget
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLPrefabSequenceEditorWidget){}
	SLATE_END_ARGS();

	void Construct(const FArguments&, TWeakPtr<FBlueprintEditor> InBlueprintEditor);
	void AssignSequence(ULPrefabSequence* NewLPrefabSequence);
	ULPrefabSequence* GetSequence() const;
	FText GetDisplayLabel() const;

private:

	TWeakPtr<SLPrefabSequenceEditorWidgetImpl> Impl;
};

