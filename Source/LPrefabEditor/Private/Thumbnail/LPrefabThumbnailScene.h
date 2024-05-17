// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "ThumbnailHelpers.h"
#include "PrefabSystem/LPrefab.h"

class FLPrefabThumbnailScene :public FThumbnailPreviewScene
{
public:
	FLPrefabThumbnailScene();
	bool IsValidForVisualization();
	void SetPrefab(class ULPrefab* Prefab);
protected:
	virtual void GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom)const override;
	virtual USceneThumbnailInfo* GetSceneThumbnailInfo(const float TargetDistance)const;
	void SpawnPreviewActor();
	void GetBoundsRecursive(USceneComponent* RootComp, FBoxSphereBounds& OutBounds, bool& IsFirstPrimitive)const;
private:
	void ClearOldActors();
private:
	int32 NumStartingActors;
	TWeakObjectPtr<class ULPrefab> CurrentPrefab;
	FText CachedPrefabContent;
	FBoxSphereBounds PreviewActorsBound;
	bool bIsUI = false;
};

class FLPrefabInstanceThumbnailScene
{
public:
	FLPrefabInstanceThumbnailScene();

	TSharedPtr<FLPrefabThumbnailScene> FindThumbnailScene(const FString& InPrefabPath) const;
	TSharedRef<FLPrefabThumbnailScene> EnsureThumbnailScene(const FString& InPrefabPath);
	void Clear();

private:
	TMap<FString, TSharedPtr<FLPrefabThumbnailScene>> InstancedThumbnailScenes;
	const int32 MAX_NUM_SCENES = 400;
};