// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LPrefabSettings.generated.h"

/** for LPrefab config */
UCLASS(config=Engine, defaultconfig)
class LPREFAB_API ULPrefabSettings :public UObject
{
	GENERATED_BODY()
public:
	/**
	 * For load prefab debug, display a log that shows how much time a LoadPrefab cost.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LPrefab")
		bool bLogPrefabLoadTime = false;
	/**
	 * Prefabs in these folders will appear in "LGUI Tools" menu, so we can easily create our own UI control.
	 */
	UPROPERTY(EditAnywhere, config, Category = "LPrefab Editor", meta = (LongPackageName))
		TArray<FDirectoryPath> ExtraPrefabFolders;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
public:
	static bool GetLogPrefabLoadTime();
};
