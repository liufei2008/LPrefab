// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ILPrefabInterface.generated.h"


/**
 * Interface for Actor or ActorComponent that loaded from LPrefab
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULPrefabInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for Actor or ActorComponent that loaded from LPrefab
 */ 
class LPREFAB_API ILPrefabInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when LPrefab finish load. This is called late than BeginPlay.
	 *		Awake execute order in prefab: higher in hierarchy will execute earlier, so scripts on root actor will execute the first. Actor execute first, then execute on component.
	 *		And this Awake is execute later than LGUILifeCycleBehaviour's Awake when in same prefab.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LPrefab)
		void Awake();
	/**
	 * Same as Awake but only execute in edit mode.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LPrefab)
		void EditorAwake();
};