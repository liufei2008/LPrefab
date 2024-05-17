// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabBPLibrary.h"
#include "LPrefabUtils.h"
#include "PrefabSystem/LPrefab.h"
#include "LPrefabModule.h"
#include LPREFAB_SERIALIZER_NEWEST_INCLUDE

void ULPrefabBPLibrary::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	LPrefabUtils::DestroyActorWithHierarchy(Target, WithHierarchy);
}
AActor* ULPrefabBPLibrary::LoadPrefab(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake, bool SetRelativeTransformToIdentity)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefab(WorldContextObject, InParent, InCallbackBeforeAwake, SetRelativeTransformToIdentity);
}
AActor* ULPrefabBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
}
AActor* ULPrefabBPLibrary::LoadPrefabWithTransform(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale, const TFunction<void(AActor*)>& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithTransform(WorldContextObject, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
}
AActor* ULPrefabBPLibrary::LoadPrefabWithReplacement(UObject* WorldContextObject, ULPrefab* InPrefab, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	if (!IsValid(InPrefab))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InPrefab not valid"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	return InPrefab->LoadPrefabWithReplacement(WorldContextObject, InParent, InReplaceAssetMap, InReplaceClassMap, InCallbackBeforeAwake);
}

AActor* ULPrefabBPLibrary::DuplicateActor(AActor* Target, USceneComponent* Parent)
{
	return LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::DuplicateActor(Target, Parent);
}
void ULPrefabBPLibrary::PrepareDuplicateData(AActor* Target, FLPrefabDuplicateDataContainer& DataContainer)
{
	DataContainer.bIsValid = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::PrepareDataForDuplicate(Target, DataContainer.DuplicateData);
}
AActor* ULPrefabBPLibrary::DuplicateActorWithPreparedData(FLPrefabDuplicateDataContainer& Data, USceneComponent* Parent)
{
	if (Data.bIsValid)
	{
		return LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::DuplicateActorWithPreparedData(Data.DuplicateData, Parent);
	}
	else
	{
		return nullptr;
	}
}

UActorComponent* ULPrefabBPLibrary::GetComponentInParent(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, AActor* InStopNode)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabBPLibrary::GetComponentInParent]InActor is not valid!"));
		return nullptr;
	}
	AActor* parentActor = IncludeSelf ? InActor : InActor->GetAttachParentActor();
	while (parentActor != nullptr)
	{
		if (InStopNode != nullptr)
		{
			if (parentActor == InStopNode)return nullptr;
		}
		auto resultComp = parentActor->FindComponentByClass(ComponentClass);
		if (resultComp != nullptr)
		{
			return resultComp;
		}
		else
		{
			parentActor = parentActor->GetAttachParentActor();
		}
	}
	return nullptr;
}
TArray<UActorComponent*> ULPrefabBPLibrary::GetComponentsInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode)
{
	TArray<UActorComponent*> result;
	if (!IsValid(InActor))
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabBPLibrary::GetComponentInParent]InActor is not valid!"));
		return result;
	}

	struct LOCAL
	{
		static void CollectComponentsInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, TArray<UActorComponent*>& InOutArray, const TSet<AActor*>& InExcludeNode)
		{
			if (InExcludeNode.Contains(InActor))return;
			auto& components = InActor->GetComponents();
			for (UActorComponent* comp : components)
			{
				if (IsValid(comp) && comp->IsA(ComponentClass))
				{
					InOutArray.Add(comp);
				}
			}

			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			if (childrenActors.Num() > 0)
			{
				for (AActor* actor : childrenActors)
				{
					CollectComponentsInChildrenRecursive(actor, ComponentClass, InOutArray, InExcludeNode);
				}
			}
		}
	};
	if (IncludeSelf)
	{
		LOCAL::CollectComponentsInChildrenRecursive(InActor, ComponentClass, result, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				LOCAL::CollectComponentsInChildrenRecursive(actor, ComponentClass, result, InExcludeNode);
			}
		}
	}
	return result;
}

UActorComponent* ULPrefabBPLibrary::GetComponentInChildren(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, bool IncludeSelf, const TSet<AActor*>& InExcludeNode)
{
	if (!IsValid(InActor))
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabBPLibrary::GetComponentInChildren]InActor is not valid!"));
		return nullptr;
	}

	struct LOCAL
	{
		static UActorComponent* FindComponentInChildrenRecursive(AActor* InActor, TSubclassOf<UActorComponent> ComponentClass, const TSet<AActor*>& InExcludeNode)
		{
			if (InExcludeNode.Contains(InActor))return nullptr;
			if (auto comp = InActor->GetComponentByClass(ComponentClass))
			{
				if (IsValid(comp))
				{
					return comp;
				}
			}
			TArray<AActor*> childrenActors;
			InActor->GetAttachedActors(childrenActors);
			for (auto childActor : childrenActors)
			{
				auto comp = FindComponentInChildrenRecursive(childActor, ComponentClass, InExcludeNode);
				if (IsValid(comp))
				{
					return comp;
				}
			}
			return nullptr;
		}
	};

	UActorComponent* result = nullptr;
	if (IncludeSelf)
	{
		result = LOCAL::FindComponentInChildrenRecursive(InActor, ComponentClass, InExcludeNode);
	}
	else
	{
		TArray<AActor*> childrenActors;
		InActor->GetAttachedActors(childrenActors);
		if (childrenActors.Num() > 0)
		{
			for (AActor* actor : childrenActors)
			{
				result = LOCAL::FindComponentInChildrenRecursive(actor, ComponentClass, InExcludeNode);
				if (IsValid(result))
				{
					return result;
				}
			}
		}
	}
	return result;
}
