// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "HitProxies.h"

//this file mostly reference from "UnrealEd/public/LevelViewportClickHandlers.h"

class AActor;
class ABrush;
class FLPrefabEditorViewportClient;
class UModel;
struct FTypedElementHandle;
struct FViewportClick;
struct HActor;

namespace LPrefabViewportClickHandlers
{
	bool ClickViewport(FLPrefabEditorViewportClient* ViewportClient, const FViewportClick& Click);

	bool ClickElement(FLPrefabEditorViewportClient* ViewportClient, const FTypedElementHandle& HitElement, const FViewportClick& Click);

	bool ClickActor(FLPrefabEditorViewportClient* ViewportClient,AActor* Actor,const FViewportClick& Click,bool bAllowSelectionChange);

	bool ClickComponent(FLPrefabEditorViewportClient* ViewportClient, HActor* ActorHitProxy, const FViewportClick& Click);

	void ClickBrushVertex(FLPrefabEditorViewportClient* ViewportClient,ABrush* InBrush,FVector* InVertex,const FViewportClick& Click);

	void ClickStaticMeshVertex(FLPrefabEditorViewportClient* ViewportClient,AActor* InActor,FVector& InVertex,const FViewportClick& Click);
	
	void ClickSurface(FLPrefabEditorViewportClient* ViewportClient, UModel* Model, int32 iSurf, const FViewportClick& Click);

	void ClickBackdrop(FLPrefabEditorViewportClient* ViewportClient,const FViewportClick& Click);

	void ClickLevelSocket(FLPrefabEditorViewportClient* ViewportClient, HHitProxy* HitProxy, const FViewportClick& Click);
};


