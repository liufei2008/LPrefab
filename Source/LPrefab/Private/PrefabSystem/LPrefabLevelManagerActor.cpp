// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefabLevelManagerActor.h"
#include "LPrefabModule.h"
#include "LPrefabUtils.h"
#include "PrefabSystem/LPrefabHelperObject.h"
#if WITH_EDITOR
#include "PrefabSystem/LPrefabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "EditorActorFolders.h"
#endif

#define LOCTEXT_NAMESPACE "LPrefabLevelManagerActor"


ALPrefabLevelManagerActor::ALPrefabLevelManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	PrefabHelperObject = CreateDefaultSubobject<ULPrefabHelperObject>(TEXT("PrefabHelper"));
#endif
}

#if WITH_EDITOR

FName ALPrefabLevelManagerActor::PrefabFolderName(TEXT("--LPrefabActor--"));
TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALPrefabLevelManagerActor>> ALPrefabLevelManagerActor::MapLevelToManagerActor;

ALPrefabLevelManagerActor* ALPrefabLevelManagerActor::GetInstance(ULevel* InLevel, bool CreateIfNotExist)
{
	if (!MapLevelToManagerActor.Contains(InLevel) && CreateIfNotExist)
	{
		auto PrefabManagerActor = InLevel->GetWorld()->SpawnActor<ALPrefabLevelManagerActor>();
		MapLevelToManagerActor.Add(InLevel, PrefabManagerActor);
		InLevel->MarkPackageDirty();
		FActorFolders::Get().CreateFolder(*PrefabManagerActor->GetWorld(), FFolder(FFolder::FRootObject(PrefabManagerActor), ALPrefabLevelManagerActor::PrefabFolderName));
		PrefabManagerActor->SetFolderPath(ALPrefabLevelManagerActor::PrefabFolderName);
	}
	if (auto ResultPtr = MapLevelToManagerActor.Find(InLevel))
	{
		if (ResultPtr->IsValid())
		{
			if (auto World = ResultPtr->Get()->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					(*ResultPtr)->PrefabHelperObject->MarkAsManagerObject();//can only manage prefab in edit mode
				}
			}
			return ResultPtr->Get();
		}
		else
		{
			MapLevelToManagerActor.Remove(InLevel);
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

ALPrefabLevelManagerActor* ALPrefabLevelManagerActor::GetInstanceByPrefabHelperObject(ULPrefabHelperObject* InHelperObject)
{
	for (auto& KeyValue : MapLevelToManagerActor)
	{
		if (KeyValue.Value->PrefabHelperObject == InHelperObject)
		{
			return KeyValue.Value.Get();
		}
	}
	return nullptr;
}

void ALPrefabLevelManagerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALPrefabLevelManagerActor::PostInitProperties()
{
	Super::PostInitProperties();
	if (this != GetDefault<ALPrefabLevelManagerActor>())
	{
		CollectWhenCreate();
	}
}

void ALPrefabLevelManagerActor::CollectWhenCreate()
{
	if (auto Level = this->GetLevel())
	{
		if (!MapLevelToManagerActor.Contains(Level))
		{
			MapLevelToManagerActor.Add(Level, this);
		}
		OnSubPrefabNewVersionUpdatedDelegateHandle = PrefabHelperObject->OnSubPrefabNewVersionUpdated.AddLambda([Actor = MakeWeakObjectPtr(this)]() {
			if (Actor.IsValid())
			{
				Actor->MarkPackageDirty();
			}
		});
	}
	ULPrefabManagerObject::AddOneShotTickFunction([Actor = MakeWeakObjectPtr(this)]{
		if (Actor.IsValid())
		{
			if (auto World = Actor->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					Actor->PrefabHelperObject->CheckPrefabVersion();
				}
			}
		}
		}, 1);

	BeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddLambda([Actor = MakeWeakObjectPtr(this)](const bool isSimulating) {
		if (Actor.IsValid())
		{
			Actor->PrefabHelperObject->DismissAllVersionNotifications();
		}
	});
}

void ALPrefabLevelManagerActor::PostActorCreated()
{
	Super::PostActorCreated();
	if (this != GetDefault<ALPrefabLevelManagerActor>())
	{
		CollectWhenCreate();
	}
}

void ALPrefabLevelManagerActor::BeginDestroy()
{
	Super::BeginDestroy();
	CleanupWhenDestroy();
}
void ALPrefabLevelManagerActor::Destroyed()
{
	Super::Destroyed();
	CleanupWhenDestroy();
	if (!this->GetWorld()->IsGameWorld())
	{
		ULPrefabManagerObject::AddOneShotTickFunction([Actor = this, World = this->GetWorld()]() {
			auto InfoText = LOCTEXT("DeleteLPrefabLevelManagerActor", "\
LPrefabLevelManagerActor is being destroyed!\
\nThis actor is responsible for managing LPrefabs in current level, if you delete it then all LPrefabs linked in the level will be lost!\
\nCilck OK to confirm delete, or Cancel to undo it.");
			auto Return = FMessageDialog::Open(EAppMsgType::OkCancel, InfoText);
			if (Return == EAppReturnType::Cancel)
			{
				GEditor->UndoTransaction(false);
				Actor->CollectWhenCreate();
			}
			else if (Return == EAppReturnType::Ok)
			{
				//cleanup LPrefabHelperActor
				for (TObjectIterator<ULPrefabHelperObject> Itr; Itr; ++Itr)
				{
					Itr->CleanupInvalidSubPrefab();
				}
			}
			}, 1);
	}
}
void ALPrefabLevelManagerActor::CleanupWhenDestroy()
{
	if (OnSubPrefabNewVersionUpdatedDelegateHandle.IsValid())
	{
		PrefabHelperObject->OnSubPrefabNewVersionUpdated.Remove(OnSubPrefabNewVersionUpdatedDelegateHandle);
	}
	if (BeginPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(BeginPIEDelegateHandle);
	}
	if (this != GetDefault<ALPrefabLevelManagerActor>())
	{
		if (this->GetWorld() != nullptr && !this->GetWorld()->IsGameWorld())
		{
			auto Level = this->GetLevel();
			if (MapLevelToManagerActor.Contains(Level))
			{
				MapLevelToManagerActor.Remove(Level);
			}
		}
	}
	//cleanup
	{
		MapLevelToManagerActor.Remove(nullptr);
		TSet<TWeakObjectPtr<ULevel>> ToClear;
		for (auto& KeyValue : MapLevelToManagerActor)
		{
			if (!KeyValue.Key.IsValid() && !KeyValue.Value.IsValid())
			{
				ToClear.Add(KeyValue.Key.Get());
			}
		}
		for (auto& Item : ToClear)
		{
			MapLevelToManagerActor.Remove(Item);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE