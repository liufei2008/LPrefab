// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LPrefabThumbnailScene.h"
#include "LPrefabThumbnailRenderer.generated.h"

UCLASS()
class ULPrefabThumbnailRenderer :public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()
public:
	ULPrefabThumbnailRenderer();

	virtual bool CanVisualizeAsset(UObject* Object)override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)override;

	virtual void BeginDestroy()override;

private:
	FLPrefabInstanceThumbnailScene ThumbnailScenes;
};