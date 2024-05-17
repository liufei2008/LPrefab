// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LPrefabActorFactory.h"
#include "PrefabSystem/LPrefab.h"
#include "PrefabSystem/LPrefabLevelManagerActor.h"
#include "PrefabSystem/LPrefabHelperObject.h"
#include "PrefabSystem/LPrefabManager.h"
#include "LPrefabEditorTools.h"
#include "AssetRegistry/AssetData.h"
#include "LPrefabUtils.h"
#include "Editor.h"
#include "EditorActorFolders.h"


#define LOCTEXT_NAMESPACE "LPrefabActorFactory"


ULPrefabActorFactory::ULPrefabActorFactory()
{
	DisplayName = LOCTEXT("PrefabDisplayName", "Prefab");
	NewActorClass = ALPrefabLoadHelperActor::StaticClass();
	bShowInEditorQuickMenu = false;
	bUseSurfaceOrientation = false;
}

bool ULPrefabActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(ULPrefab::StaticClass()))
	{
		return true;
	}

	return false;
}

bool ULPrefabActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	ULPrefab* Prefab = CastChecked<ULPrefab>(Asset);

	if (Prefab == NULL)
	{
		return false;
	}
	return true;
}

void ULPrefabActorFactory::PostSpawnActor(UObject* Asset, AActor* InNewActor)
{
	Super::PostSpawnActor(Asset, InNewActor);

	ULPrefab* Prefab = CastChecked<ULPrefab>(Asset);

	auto PrefabActor = CastChecked<ALPrefabLoadHelperActor>(InNewActor);

	PrefabActor->PrefabAsset = Prefab;
	auto SelectedActor = LPrefabEditorTools::GetFirstSelectedActor();
	if (SelectedActor != nullptr && PrefabActor->GetWorld() == SelectedActor->GetWorld())
	{
		LPrefabEditorTools::MakeCurrentLevel(SelectedActor);
		auto ParentComp = SelectedActor->GetRootComponent();
		PrefabActor->LoadPrefab(ParentComp);
	}
	else
	{
		PrefabActor->LoadPrefab(nullptr);
	}
	PrefabActor->MoveActorToPrefabFolder();
	PrefabActor->SetFlags(EObjectFlags::RF_Transient);
	ULPrefabManagerObject::AddOneShotTickFunction([=]() {
		GEditor->SelectActor(PrefabActor, false, true, false, true);
		GEditor->SelectActor(PrefabActor->LoadedRootActor, true, true, false, true);
		});
}

void ULPrefabActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		auto Prefab = CastChecked<ULPrefab>(Asset);
		auto PrefabActor = CastChecked<ALPrefabLoadHelperActor>(CDO);

		PrefabActor->PrefabAsset = Prefab;
	}
}

UObject* ULPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALPrefabLoadHelperActor>(ActorInstance);
	return PrefabActor->PrefabAsset;
}

#undef LOCTEXT_NAMESPACE



// Sets default values
ALPrefabLoadHelperActor::ALPrefabLoadHelperActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
	bListedInSceneOutliner = false;
}

void ALPrefabLoadHelperActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALPrefabLoadHelperActor::Destroyed()
{
	Super::Destroyed();
	if (IsValid(LoadedRootActor))
	{
		if (auto PrefabManagerActor = ALPrefabLevelManagerActor::GetInstance(this->GetLevel()))
		{
			PrefabManagerActor->PrefabHelperObject->RemoveSubPrefabByAnyActorOfSubPrefab(LoadedRootActor);
			LPrefabUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
		}
	}
}
void ALPrefabLoadHelperActor::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALPrefabLoadHelperActor::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), FFolder(FFolder::FRootObject(this), ALPrefabLevelManagerActor::PrefabFolderName));
	this->SetFolderPath(ALPrefabLevelManagerActor::PrefabFolderName);
}

void ALPrefabLoadHelperActor::LoadPrefab(USceneComponent* InParent)
{
	if (this->GetWorld() != nullptr && this->GetWorld()->IsGameWorld())return;
	if (IsValid(LoadedRootActor))return;
	auto PrefabHelperObject = ALPrefabLevelManagerActor::GetInstance(this->GetLevel())->PrefabHelperObject;
	PrefabHelperObject->SetCanNotifyAttachment(false);
	TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
	TMap<TObjectPtr<AActor>, FLSubPrefabData> SubSubPrefabMap;
	LoadedRootActor = PrefabAsset->LoadPrefabWithExistingObjects(this->GetWorld()
		, InParent
		, SubPrefabMapGuidToObject, SubSubPrefabMap
	);
	PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedRootActor, SubPrefabMapGuidToObject, {});
	PrefabHelperObject->SetCanNotifyAttachment(true);
}

