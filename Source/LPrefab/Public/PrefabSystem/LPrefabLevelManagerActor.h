// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LPrefab.h"
#include "LPrefabLevelManagerActor.generated.h"

class ULPrefabHelperObject;

/**
 * Wraper or container for ULPrefabHelperObject. One level should only have one LPrefabLevelManagerActor.
 */
UCLASS(ClassGroup = (LPrefab), NotBlueprintable, NotPlaceable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LPREFAB_API ALPrefabLevelManagerActor : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ALPrefabLevelManagerActor();

#if WITH_EDITOR
	virtual void BeginPlay()override;
	virtual void PostInitProperties()override;
	virtual void PostActorCreated()override;
	virtual void BeginDestroy() override;
	virtual void Destroyed()override;
private:
	FDelegateHandle OnSubPrefabNewVersionUpdatedDelegateHandle;
	FDelegateHandle BeginPIEDelegateHandle;
	void CollectWhenCreate();
	void CleanupWhenDestroy();
	static TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALPrefabLevelManagerActor>> MapLevelToManagerActor;
public:
	static FName PrefabFolderName;
	static ALPrefabLevelManagerActor* GetInstance(ULevel* InLevel, bool CreateIfNotExist = true);
	static ALPrefabLevelManagerActor* GetInstanceByPrefabHelperObject(ULPrefabHelperObject* InHelperObject);
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LPrefab")
		TObjectPtr<ULPrefabHelperObject> PrefabHelperObject = nullptr;
#endif
};
