// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "ActorFactories/ActorFactory.h"
#include "LPrefabActorFactory.generated.h"

/** Create a agent actor and use it to spawn the actual prefab. */
UCLASS()
class ULPrefabActorFactory : public UActorFactory
{
	GENERATED_BODY()
public:
	ULPrefabActorFactory();
	//~ Begin UActorFactory
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual bool PreSpawnActor(UObject* Asset, FTransform& InOutLocation) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	//virtual FQuat AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const override;
	//~ End UActorFactory

};


class ULPrefab;

UCLASS(ClassGroup = (LPrefab), Transient, NotBlueprintable, NotBlueprintType, NotPlaceable, HideCategories = (Rendering, Actor, Input))
class ALPrefabLoadHelperActor : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	ALPrefabLoadHelperActor();

	virtual void BeginPlay()override;
	virtual void Destroyed()override;
	virtual void BeginDestroy() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "LPrefab")
		TObjectPtr<ULPrefab> PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LPrefab")
		TObjectPtr<AActor> LoadedRootActor = nullptr;
	void LoadPrefab(USceneComponent* InParent);
	void MoveActorToPrefabFolder();

};

