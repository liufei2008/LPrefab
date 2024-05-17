// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Thumbnail/LPrefabThumbnailRenderer.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "SceneView.h"
#include "Engine/EngineTypes.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "LPrefabEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#include "PrefabSystem/LPrefab.h"

ULPrefabThumbnailRenderer::ULPrefabThumbnailRenderer()
{

}

bool ULPrefabThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (auto prefab = Cast<ULPrefab>(Object))
		return true;
	return false;
}
void ULPrefabThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	auto prefab = Cast<ULPrefab>(Object);
	if (IsValid(prefab))
	{
		TSharedRef<FLPrefabThumbnailScene> ThumbnailScene = ThumbnailScenes.EnsureThumbnailScene(prefab->GetPathName());
		ThumbnailScene->SetPrefab(prefab);
		if (!ThumbnailScene->IsValidForVisualization())
			return;

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetTime(UThumbnailRenderer::GetTime()));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;

		auto View = ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily, View);

		//draw prefab icon
		static FString LGUIBasePath = (IPluginManager::Get().FindPlugin("LGUI") != nullptr ? IPluginManager::Get().FindPlugin("LGUI") : IPluginManager::Get().FindPlugin("LPrefab"))->GetBaseDir();
		LPrefabEditorUtils::DrawThumbnailIcon(LGUIBasePath + (prefab->GetIsPrefabVariant() ? TEXT("/Resources/Icons/PrefabVariant_40x.png") : TEXT("/Resources/Icons/Prefab_40x.png"))
			, X, Y, Width, Height, Canvas);
	}
}
void ULPrefabThumbnailRenderer::BeginDestroy()
{
	ThumbnailScenes.Clear();
	Super::BeginDestroy();
}