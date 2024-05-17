// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture.h"
#include "LPrefabEditorSettings.generated.h"

UCLASS(config=Editor, defaultconfig)
class LPREFABEDITOR_API ULPrefabEditorSettings : public UObject
{
	GENERATED_BODY()
public:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual bool IsEditorOnly()const override { return true; }

	static bool GetPreserveHierarchyState();
	static float GetDelayRestoreHierarchyTime();
	/**
	 * Keep World Outliner's actor state: expand and temporarily-hidden. When reload a level or play & endplay, all actors will expand and temporarily-hidden actors become visible, so we can check this on to keep these actor and folder's state.
	 * Note: If actors in folder and the folder is not expanded, then these actors's state will not affected, because I can't get these tree items.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor")
		bool bPreserveHierarchyState = true;
	static FSimpleMulticastDelegate LPrefabEditorSetting_PreserveHierarchyStateChange;

	/**
	 * Sometimes when there are too many actors in level, restore hierarchy will not work. Then increase this value may solve the issue.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LGUI Editor")
		float DelayRestoreHierarchyTime = 0.2f;

	UPROPERTY(config)
		bool ShowLGUIColumnInSceneOutliner = true;
};
