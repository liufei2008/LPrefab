// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Thumbnail/LPrefabThumbnailScene.h"
#include "Components/PrimitiveComponent.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "LPrefabEditorModule.h"
#include "LPrefabUtils.h"
#include "PrefabSystem/LPrefabManager.h"


FLPrefabInstanceThumbnailScene::FLPrefabInstanceThumbnailScene()
{
	InstancedThumbnailScenes.Reserve(MAX_NUM_SCENES);
}
TSharedPtr<FLPrefabThumbnailScene> FLPrefabInstanceThumbnailScene::FindThumbnailScene(const FString& InPrefabPath)const
{
	return InstancedThumbnailScenes.FindRef(InPrefabPath);
}
TSharedRef<FLPrefabThumbnailScene> FLPrefabInstanceThumbnailScene::EnsureThumbnailScene(const FString& InPrefabPath)
{
	TSharedPtr<FLPrefabThumbnailScene> ExistingThumbnailScene = InstancedThumbnailScenes.FindRef(InPrefabPath);
	if (!ExistingThumbnailScene.IsValid())
	{
		if (InstancedThumbnailScenes.Num() >= MAX_NUM_SCENES)
		{
			InstancedThumbnailScenes.Reset();
		}
		ExistingThumbnailScene = MakeShareable(new FLPrefabThumbnailScene());
		InstancedThumbnailScenes.Add(InPrefabPath, ExistingThumbnailScene);
	}
	return ExistingThumbnailScene.ToSharedRef();
}
void FLPrefabInstanceThumbnailScene::Clear()
{
	InstancedThumbnailScenes.Reset();
}



FLPrefabThumbnailScene::FLPrefabThumbnailScene()
	:FThumbnailPreviewScene()
	, NumStartingActors(0)
	, CurrentPrefab(nullptr)
{
	NumStartingActors = GetWorld()->GetCurrentLevel()->Actors.Num();
}
void FLPrefabThumbnailScene::SpawnPreviewActor()
{
	if (CurrentPrefab.IsValid())
	{
		if (auto RootActor = CurrentPrefab->LoadPrefabInEditor(GetWorld(), nullptr))
		{
			if (FLPrefabEditorModule::PrefabEditor_GenerateAgentActorForPrefabThumbnail.IsBound())
			{
				bIsUI = FLPrefabEditorModule::PrefabEditor_GenerateAgentActorForPrefabThumbnail.Execute(RootActor, CurrentPrefab.Get());
			}
			else
			{
				bIsUI = false;
			}

			bool isFirstBounds = true;
			GetBoundsRecursive(RootActor->GetRootComponent(), PreviewActorsBound, isFirstBounds);
			if (isFirstBounds)
			{
				PreviewActorsBound = FBoxSphereBounds(EForceInit::ForceInitToZero);
			}
			if (PreviewActorsBound.SphereRadius < KINDA_SMALL_NUMBER)//if bounds is too small, set to 1x1 box
			{
				PreviewActorsBound = FBoxSphereBounds(FBox(FVector(-0.5f, -0.5f, -0.5f), FVector(0.5f, 0.5f, 0.5f)));
			}
		}
	}
}
void FLPrefabThumbnailScene::GetBoundsRecursive(USceneComponent* RootComp, FBoxSphereBounds& OutBounds, bool& IsFirstPrimitive)const
{
	if (!IsValid(RootComp))return;
	if (RootComp->IsVisualizationComponent())return;
	FBoxSphereBounds Bounds;
	bool bIsValidBounds = false;
	if (RootComp->IsRegistered())
	{
		if (!RootComp->GetOwner()->IsHiddenEd() && RootComp->IsVisible())
		{
			Bounds = RootComp->Bounds;
			bIsValidBounds = true;
		}

		if (bIsValidBounds)
		{
			if (IsFirstPrimitive)
			{
				OutBounds = Bounds;
				IsFirstPrimitive = false;
			}
			else
			{
				OutBounds = OutBounds + Bounds;
			}
		}
	}

	auto childrenArray = RootComp->GetAttachChildren();
	for (auto childComp : childrenArray)
	{
		GetBoundsRecursive(childComp, OutBounds, IsFirstPrimitive);
	}
}
void FLPrefabThumbnailScene::ClearOldActors()
{
	auto Level = GetWorld()->GetCurrentLevel();
	for (int i = NumStartingActors; i < Level->Actors.Num(); i++)
	{
		if (Level->Actors[i])
		{
			Level->Actors[i]->Destroy();
		}
	}
}
bool FLPrefabThumbnailScene::IsValidForVisualization()
{
	if (CurrentPrefab.Get())
	{
		if (CurrentPrefab->BinaryData.Num() == 0)
			return false;
	}
	if (PreviewActorsBound.ContainsNaN())
	{
		UE_LOG(LPrefabEditor, Warning, TEXT("[FLPrefabThumbnailScene::IsValidForVisualization]prefab:%s bounds is invalid!"), *(CurrentPrefab->GetPathName()));
		return false;
	}
	return true;
}
void FLPrefabThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom)const
{
	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;

	const float PreviewSize = PreviewActorsBound.SphereRadius * 1.2f;
	const float BoundsZOffset = GetBoundsZOffset(PreviewActorsBound);
	const float TargetDistance = PreviewSize / FMath::Tan(HalfFOVRadians);

	USceneThumbnailInfo* ThumbnailInfo = GetSceneThumbnailInfo(TargetDistance);
	check(ThumbnailInfo);

	OutOrigin = -1 * PreviewActorsBound.Origin;
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + ThumbnailInfo->OrbitZoom;
}
void FLPrefabThumbnailScene::SetPrefab(class ULPrefab* Prefab)
{
	if (!CurrentPrefab.IsValid())
	{
		CurrentPrefab = nullptr;
		ClearOldActors();
	}
	if (CurrentPrefab.IsValid() && IsValid(Prefab))
	{
		if (CurrentPrefab == Prefab && !CurrentPrefab->ThumbnailDirty)
		{
			return;
		}
		ClearOldActors();
	}
	CurrentPrefab = Prefab;
	CurrentPrefab->ThumbnailDirty = false;
	if (IsValid(Prefab))
	{
		SpawnPreviewActor();
	}
}
USceneThumbnailInfo* FLPrefabThumbnailScene::GetSceneThumbnailInfo(const float TargetDistance)const
{
	ULPrefab* Prefab = CurrentPrefab.Get();
	check(Prefab);
	USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(Prefab->ThumbnailInfo);
	if (!IsValid(ThumbnailInfo))
	{
		ThumbnailInfo = NewObject<USceneThumbnailInfo>(Prefab);
		Prefab->ThumbnailInfo = ThumbnailInfo;
	}
	if (bIsUI)
	{
		ThumbnailInfo->OrbitPitch = 0;
		ThumbnailInfo->OrbitYaw = 90.0f;
		ThumbnailInfo->OrbitZoom = 0;
	}
	else
	{
		ThumbnailInfo->OrbitPitch = -11.25f;
		ThumbnailInfo->OrbitYaw = -157.5f;
		ThumbnailInfo->OrbitZoom = 0;
	}
	return ThumbnailInfo;
}