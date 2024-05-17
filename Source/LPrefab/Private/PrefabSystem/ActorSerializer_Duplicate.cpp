// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer8.h"
#include "PrefabSystem/LPrefabObjectReaderAndWriter.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PrefabSystem/LPrefabManager.h"
#include "LPrefabModule.h"
#include "PrefabSystem/LPrefabSettings.h"
#if WITH_EDITOR
#include "Tools/UEdMode.h"
#include "LPrefabUtils.h"
#endif

#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif
namespace LPREFAB_SERIALIZER_NEWEST_NAMESPACE
{
	AActor* ActorSerializer::DuplicateActor(AActor* OriginRootActor, USceneComponent* Parent)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d OriginRootActor is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d Cannot get World from OriginRootActor!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		auto StartTime = FDateTime::Now();

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		FLPrefabSaveData SaveData;
		serializer.SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootActor = serializer.DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		if (ULPrefabSettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LPrefab, Log, TEXT("Duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		}

#if WITH_EDITOR
		ULPrefabManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif
		return CreatedRootActor;
	}
	bool ActorSerializer::PrepareDataForDuplicate(AActor* OriginRootActor, FDuplicateActorDataContainer& OutData)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d OriginRootActor is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d Cannot get World from OriginRootActor!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return false;
		}

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		auto StartTime = FDateTime::Now();

		auto& serializer = OutData.Serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;

		//serialize
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.SerializeActorToData(OriginRootActor, OutData.ActorData);

		//for deserialize, set once for all use
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};

		if (ULPrefabSettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LPrefab, Log, TEXT("PrepareData_ForDuplicate, actor: '%s' total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());
		}
		return true;
	}
	AActor* ActorSerializer::DuplicateActorWithPreparedData(FDuplicateActorDataContainer& InData, USceneComponent* InParent)
	{
		auto StartTime = FDateTime::Now();
		auto& serializer = InData.Serializer;//use copied, incase undesired data
		//clear these data for deserializer use
		serializer.WillSerializeActorArray.Reset();
		serializer.WillSerializeObjectArray.Reset();
		serializer.MapGuidToObject.Reset();
		serializer.MapObjectToGuid.Reset();
		serializer.ComponentsInThisPrefab.Reset();
		serializer.SubPrefabMap.Reset();
		serializer.SubPrefabRootComponents.Reset();
		serializer.AllActors.Reset();
		serializer.AllComponents.Reset();
		serializer.SubPrefabOverrideParameters.Reset();
		serializer.DeserializationSessionId = FGuid();
		serializer.bIsSubPrefab = false;
		serializer.SubPrefabObjectOverrideData.Reset();

		auto CreatedRootActor = serializer.DeserializeActorFromData(InData.ActorData, InParent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
		if (ULPrefabSettings::GetLogPrefabLoadTime())
		{
			auto TimeSpan = FDateTime::Now() - StartTime;
			UE_LOG(LPrefab, Log, TEXT("DuplicateActorWithPreparedData total time: %fms"), TimeSpan.GetTotalMilliseconds());
		}
#if WITH_EDITOR
		ULPrefabManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif
		return CreatedRootActor;
	}

	AActor* ActorSerializer::DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
		, const TMap<TObjectPtr<AActor>, FLSubPrefabData>& InSubPrefabMap
		, const TMap<UObject*, FGuid>& InMapObjectToGuid
		, TMap<TObjectPtr<AActor>, FLSubPrefabData>& OutDuplicatedSubPrefabMap
		, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject
	)
	{
		if (!OriginRootActor)
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d OriginRootActor is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}
		if (!OriginRootActor->GetWorld())
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d Cannot get World from OriginRootActor!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			return nullptr;
		}

		auto Name =
#if WITH_EDITOR
			OriginRootActor->GetActorLabel();
#else
			OriginRootActor->GetPathName();
#endif
		UE_LOG(LPrefab, Log, TEXT("Begin duplicate actor: '%s'"), *Name);
		auto StartTime = FDateTime::Now();

		ActorSerializer serializer;
		serializer.TargetWorld = OriginRootActor->GetWorld();
		serializer.MapObjectToGuid = InMapObjectToGuid;
#if !WITH_EDITOR
		serializer.bIsEditorOrRuntime = false;
#endif
		serializer.bOverrideVersions = false;
		//serialize
		serializer.SubPrefabMap = InSubPrefabMap;
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectWriter Writer(InOutBuffer, serializer, ExcludeProperties);
			Writer.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNames) {
			LPrefabSystem::FLPrefabDuplicateOverrideParameterObjectWriter Writer(InOutBuffer, serializer, InOverridePropertyNames);
			Writer.DoSerialize(InObject);
		};
		FLPrefabSaveData SaveData;
		serializer.SerializeActorToData(OriginRootActor, SaveData);

		//deserialize
		serializer.SubPrefabMap = {};//clear it for deserializer to fill
		serializer.WriterOrReaderFunction = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, bool InIsSceneComponent) {
			auto ExcludeProperties = InIsSceneComponent ? serializer.GetSceneComponentExcludeProperties() : TSet<FName>();
			LPrefabSystem::FLPrefabDuplicateObjectReader Reader(InOutBuffer, serializer, ExcludeProperties);
			Reader.DoSerialize(InObject);
		};
		serializer.WriterOrReaderFunctionForSubPrefabOverride = [&serializer](UObject* InObject, TArray<uint8>& InOutBuffer, const TArray<FName>& InOverridePropertyNameSet) {
			LPrefabSystem::FLPrefabDuplicateOverrideParameterObjectReader Reader(InOutBuffer, serializer, InOverridePropertyNameSet);
			Reader.DoSerialize(InObject);
		};
		auto CreatedRootActor = serializer.DeserializeActorFromData(SaveData, Parent, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);

		OutDuplicatedSubPrefabMap = serializer.SubPrefabMap;
		OutMapGuidToObject = serializer.MapGuidToObject;
		auto TimeSpan = FDateTime::Now() - StartTime;
		UE_LOG(LPrefab, Log, TEXT("End duplicate actor: '%s', total time: %fms"), *Name, TimeSpan.GetTotalMilliseconds());

#if WITH_EDITOR
		ULPrefabManagerObject::MarkBroadcastLevelActorListChanged();//UE5 will not auto refresh scene outliner and display actor label, so manually refresh it.
#endif

		return CreatedRootActor;
	}
}
#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
