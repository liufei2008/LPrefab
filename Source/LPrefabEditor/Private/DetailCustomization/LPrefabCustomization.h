// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLPrefabCustomization : public IDetailCustomization
{
public:

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TWeakObjectPtr<class ULPrefab> TargetScriptPtr;
	FText GetEngineVersionText()const;
	FText GetPrefabVersionText()const;
	EVisibility ShouldShowFixEngineVersionButton()const;
	FSlateColor GetEngineVersionTextColorAndOpacity()const;
	FSlateColor GetPrefabVersionTextColorAndOpacity()const;
	EVisibility ShouldShowFixPrefabVersionButton()const;
	EVisibility ShouldShowFixAgentObjectsButton()const;
	FText AgentObjectText()const;

	FReply OnClickRecreteButton();
	FReply OnClickRecreteAllButton();
	FReply OnClickEditPrefabButton();
	FReply OnClickRecreateAgentObjects();
};
