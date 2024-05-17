// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PrefabSystem/LPrefab.h"
#include LPREFAB_SERIALIZER_NEWEST_INCLUDE
#include "LPrefabBPLibrary.generated.h"

class ULPrefab;

namespace LPREFAB_SERIALIZER_NEWEST_NAMESPACE
{
	struct FDuplicateActorDataContainer;
}

USTRUCT(BlueprintType)
struct FLPrefabDuplicateDataContainer
{
	GENERATED_BODY()
public:
	bool bIsValid = false;
	LPREFAB_SERIALIZER_NEWEST_NAMESPACE::FDuplicateActorDataContainer DuplicateData;
};

UCLASS()
class LPREFAB_API ULPrefabBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Delete actor and all it's children actors */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "WithHierarchy", UnsafeDuringActorConstruction = "true"), Category = LPrefab)
		static void DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy = true);

	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour and LPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param InCallbackBeforeAwake This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
	 * @param SetRelativeTransformToIdentity Set created root actor's transform to zero after load.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InCallbackBeforeAwake,SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm="InCallbackBeforeAwake"), Category = LPrefab)
		static AActor* LoadPrefab(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour and LPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param Location Set created root actor's location after load.
	 * @param Rotation Set created root actor's rotation after load.
	 * @param Scale Set created root actor's scale after load.
	 * @param InCallbackBeforeAwake This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InCallbackBeforeAwake"), Category = LPrefab)
		static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake);
	static AActor* LoadPrefabWithTransform(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale, const TFunction<void(AActor*)>& InCallbackBeforeAwake = nullptr);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour and LPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param InReplaceAssetMap Replace source asset to dest before load the prefab.
	 * @param InReplaceClassMap Replace source class to dest before load the prefab.
	 * @param InCallbackBeforeAwake This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InCallbackBeforeAwake"), Category = LPrefab)
		static AActor* LoadPrefabWithReplacement(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake);

	/**
	 * Duplicate actor and all it's children actors
	 * If duplicate same actor for multiple times, then use PrepareDuplicateData node to get data, and pass the data to DuplicateActorWithPreparedData.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true", ToolTip = "Duplicate actor with hierarchy"), Category = LPrefab)
		static AActor* DuplicateActor(AActor* Target, USceneComponent* Parent);
	/**
	 * Optimized version of DuplicateActor node when you need to duplicate same actor for multiple times. Use the result data in DuplicateActorWithPreparedData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true"), Category = LPrefab)
		static void PrepareDuplicateData(AActor* Target, FLPrefabDuplicateDataContainer& Data);
	/**
	 * Use this with PrepareDuplicateData node.
	 */
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Target", UnsafeDuringActorConstruction = "true"), Category = LPrefab)
		static AActor* DuplicateActorWithPreparedData(UPARAM(Ref) FLPrefabDuplicateDataContainer& Data, USceneComponent* Parent);
	template<class T>
	static T* DuplicateActorT(T* Target, USceneComponent* Parent)
	{
		static_assert(TPointerIsConvertibleFromTo<T, const AActor>::Value, "'T' template parameter to DuplicateActor must be derived from AActor");
		return (T*)ULPrefabBPLibrary::DuplicateActor(Target, Parent);
	}

	/**
	 * Find the first component in parent and up parent hierarchy with type.
	 * @param IncludeSelf	Include actor self.
	 * @param InStopNode	If parent is InStopNode then break the search chain. Can be null to ignore it.
	 */
	UFUNCTION(BlueprintPure, Category = LPrefab, meta = (ComponentClass = "ActorComponent", DeterminesOutputType = "ComponentClass"))
	static UActorComponent* GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf = true, AActor* InStopNode = nullptr);
	/**
	 * Find all compoents in children with type.
	 * @param InActor Root actor to start from.
	 * @param ComponentClass The component type that need to search.
	 * @param IncludeSelf true- also search component at InActor.
	 * @param InExcludeNode If any child actor is included in this InExcludeNode, will skip that child actor and all it's children.
	 */
	UFUNCTION(BlueprintPure, Category = LPrefab, meta = (ComponentClass = "ActorComponent", DeterminesOutputType = "ComponentClass", AutoCreateRefTerm = "InExcludeNode"))
	static TArray<UActorComponent*> GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode);
	/**
	 * Find the first component in children with type.
	 * @param InActor Root actor to start from.
	 * @param ComponentClass The component type that need to search.
	 * @param IncludeSelf true- also search component at InActor.
	 * @param InExcludeNode If any child actor is included in this InExcludeNode, will skip that child actor and all it's children.
	 */
	UFUNCTION(BlueprintPure, Category = LPrefab, meta = (ComponentClass = "ActorComponent", DeterminesOutputType = "ComponentClass", AutoCreateRefTerm = "InExcludeNode"))
	static UActorComponent* GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode);
};
