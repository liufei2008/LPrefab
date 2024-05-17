// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefab.h"
#include "LPrefabModule.h"
#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "PrefabSystem/ActorSerializer4.h"
#include "PrefabSystem/ActorSerializer5.h"
#include "PrefabSystem/ActorSerializer6.h"
#include "PrefabSystem/ActorSerializer7.h"
#endif
#include LPREFAB_SERIALIZER_NEWEST_INCLUDE
#include "LPrefabUtils.h"
#include "PrefabSystem/LPrefabManager.h"
#include "PrefabSystem/LPrefabHelperObject.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "LPrefab"


FLSubPrefabData::FLSubPrefabData()
{
#if WITH_EDITORONLY_DATA
	EditorIdentifyColor = FLinearColor::MakeRandomColor();
#endif
}
void FLSubPrefabData::AddMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyNames.Add(InPropertyName);
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (!DataItem.MemberPropertyNames.Contains(InPropertyName))
		{
			DataItem.MemberPropertyNames.Add(InPropertyName);
		}
	}
}

void FLSubPrefabData::AddMemberProperty(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyNames = InPropertyNames;
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		for (auto& NameItem : InPropertyNames)
		{
			if (!DataItem.MemberPropertyNames.Contains(NameItem))
			{
				DataItem.MemberPropertyNames.Add(NameItem);
			}
		}
	}
}

void FLSubPrefabData::RemoveMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (DataItem.MemberPropertyNames.Contains(InPropertyName))
		{
			DataItem.MemberPropertyNames.Remove(InPropertyName);
		}
		if (DataItem.MemberPropertyNames.Num() <= 0)
		{
			ObjectOverrideParameterArray.RemoveAt(Index);
		}
	}
}

void FLSubPrefabData::RemoveMemberProperty(UObject* InObject)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
	}
}

bool FLSubPrefabData::CheckParameters()
{
	bool AnythingChanged = false;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto& DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())
		{
			ObjectOverrideParameterArray.RemoveAt(i);
			i--;
			AnythingChanged = true;
		}
		else
		{
			TSet<FName> PropertyNamesToRemove;
			auto Object = DataItem.Object;
			for (auto PropertyName : DataItem.MemberPropertyNames)
			{
				auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
				if (Property == nullptr)
				{
					PropertyNamesToRemove.Add(PropertyName);
				}
			}
			for (auto PropertyName : PropertyNamesToRemove)
			{
				DataItem.MemberPropertyNames.Remove(PropertyName);
				AnythingChanged = true;
			}
		}
	}
	return AnythingChanged;
}

ULPrefab::ULPrefab()
{

}

#if WITH_EDITOR
void ULPrefab::RefreshAgentObjectsInPreviewWorld()
{
	ClearAgentObjectsInPreviewWorld();
	MakeAgentObjectsInPreviewWorld();
}
void ULPrefab::MakeAgentObjectsInPreviewWorld()
{
	if (PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive)
	{
		if (!IsValid(PrefabHelperObject))
		{
			PrefabHelperObject = NewObject<ULPrefabHelperObject>(this, "PrefabHelper");
			PrefabHelperObject->PrefabAsset = this;
		}
		if (!IsValid(PrefabHelperObject->LoadedRootActor))
		{
			if (auto World = ULPrefabManagerObject::GetPreviewWorldForPrefabPackage())
			{
				PrefabHelperObject->LoadPrefab(World, nullptr);
			}
		}
	}
}
void ULPrefab::ClearAgentObjectsInPreviewWorld()
{
	if (IsValid(PrefabHelperObject))
	{
		PrefabHelperObject->ClearLoadedPrefab();
	}
}

struct FLGUIVersionScope
{
public:
	uint16 PrefabVersion = 0;
	uint16 EngineMajorVersion = 0;
	uint16 EngineMinorVersion = 0;
	uint16 EnginePatchVersion = 0;
	int32 ArchiveVersion = 0;
	int32 ArchiveLicenseeVer = 0;
	uint32 ArEngineNetVer = 0;
	uint32 ArGameNetVer = 0;

	ULPrefab* Prefab = nullptr;
	FLGUIVersionScope(ULPrefab* InPrefab)
	{
		Prefab = InPrefab;
		this->EngineMajorVersion = Prefab->EngineMajorVersion;
		this->EngineMinorVersion = Prefab->EngineMinorVersion;
		this->PrefabVersion = Prefab->PrefabVersion;
		this->ArchiveVersion = Prefab->ArchiveVersion;
		this->ArchiveLicenseeVer = Prefab->ArchiveLicenseeVer;
		this->ArEngineNetVer = Prefab->ArEngineNetVer;
		this->ArGameNetVer = Prefab->ArGameNetVer;
	}
	~FLGUIVersionScope()
	{
		Prefab->EngineMajorVersion = this->EngineMajorVersion;
		Prefab->EngineMinorVersion = this->EngineMinorVersion;
		Prefab->PrefabVersion = this->PrefabVersion;
		Prefab->ArchiveVersion = this->ArchiveVersion;
		Prefab->ArchiveLicenseeVer = this->ArchiveLicenseeVer;
		Prefab->ArEngineNetVer = this->ArEngineNetVer;
		Prefab->ArGameNetVer = this->ArGameNetVer;
	}
};

ULPrefabHelperObject* ULPrefab::GetPrefabHelperObject()
{
	if (!IsValid(PrefabHelperObject))
	{
		PrefabHelperObject = NewObject<ULPrefabHelperObject>(this, "PrefabHelper");
		PrefabHelperObject->PrefabAsset = this;
	}
	if (!IsValid(PrefabHelperObject->LoadedRootActor))
	{
		auto World = ULPrefabManagerObject::GetPreviewWorldForPrefabPackage();
		PrefabHelperObject->LoadPrefab(World, nullptr);
	}
	return PrefabHelperObject;
}

void ULPrefab::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	BinaryDataForBuild.Empty();
	if (!IsValid(PrefabHelperObject) || !IsValid(PrefabHelperObject->LoadedRootActor))
	{
		UE_LOG(LPrefab, Log, TEXT("[%s].%d AgentObjects not valid, recreate it! prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
		MakeAgentObjectsInPreviewWorld();
	}

	//serialize to runtime data
	{
		FLGUIVersionScope VersionProtect(this);
		//check override parameter. although parameter is refreshed when sub prefab change, but what if sub prefab is changed outside of editor?
		bool AnythingChange = false;
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.CheckParameters())
			{
				AnythingChange = true;
			}
		}
		if (AnythingChange)
		{
			UE_LOG(LPrefab, Log, TEXT("[%s].%d Something changed in sub prefab override parameter, refresh it. Prefab: '%s'."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
		}

		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		this->SavePrefab(PrefabHelperObject->LoadedRootActor
			, MapObjectToGuid, PrefabHelperObject->SubPrefabMap
			, false
		);
		PrefabHelperObject->MapGuidToObject.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			PrefabHelperObject->MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}
	}
}
void ULPrefab::WillNeverCacheCookedPlatformDataAgain()
{
	if (PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}
void ULPrefab::ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
	if (PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive)
	{
		BinaryDataForBuild.Empty();
		ReferenceAssetListForBuild.Empty();
		ReferenceClassListForBuild.Empty();
		ReferenceNameListForBuild.Empty();
	}
}

void ULPrefab::PostInitProperties()
{
	Super::PostInitProperties();
}
void ULPrefab::PostCDOContruct()
{
	Super::PostCDOContruct();
}

void ULPrefab::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
}
void ULPrefab::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
}

void ULPrefab::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (PrefabVersion >= (uint16)ELPrefabVersion::BuildinFArchive)
	{

	}
	else
	{
		//generate new guid for actor data
		LPrefabSystem::ActorSerializer::RenewActorGuidForDuplicate(this);
	}
}

void ULPrefab::PostLoad()
{
	Super::PostLoad();
}

void ULPrefab::BeginDestroy()
{
#if WITH_EDITOR
	if (IsValid(PrefabHelperObject))
	{
		ClearAgentObjectsInPreviewWorld();
		PrefabHelperObject->ConditionalBeginDestroy();
	}
#endif
	Super::BeginDestroy();
}

void ULPrefab::FinishDestroy()
{
	Super::FinishDestroy();
}

void ULPrefab::PostEditUndo()
{
	Super::PostEditUndo();
	RefreshAgentObjectsInPreviewWorld();
}
bool ULPrefab::IsEditorOnly()const
{
	auto PathName = this->GetPathName();
	if (PathName.StartsWith(TEXT("/LGUI/Prefabs/"))//LGUI's preset prefab no need to use in runtime
		|| PathName.Contains(TEXT("/EditorOnly/"))//if prefab stays in a folder named "EditorOnly" then skip it too
		)
	{
		return true;
	}
	return false;
}

#endif

AActor* ULPrefab::LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity, const TFunction<void(AActor*)>& InCallbackBeforeAwake)
{
	AActor* LoadedRootActor = nullptr;
	if (InWorld)
	{
#if WITH_EDITOR
		switch ((ELPrefabVersion)PrefabVersion)
		{
		case ELPrefabVersion::NewObjectOnNestedPrefab:
		{
			LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ActorAttachToSubPrefab:
		{
			LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::CommonActor:
		{
			LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ObjectName:
		{
			LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		default:
		{
			LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
		}
		break;
		}
#else
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(InWorld, this, InParent, SetRelativeTransformToIdentity, InCallbackBeforeAwake);
#endif
	}
	return LoadedRootActor;
}

AActor* ULPrefab::LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake, bool SetRelativeTransformToIdentity)
{
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		return LoadPrefab(World, InParent, SetRelativeTransformToIdentity, [&InCallbackBeforeAwake](AActor* RootActor) {
			InCallbackBeforeAwake.ExecuteIfBound(RootActor);
			});
	}
	return nullptr;
}
AActor* ULPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		auto CallbackBeforeAwake = [&InCallbackBeforeAwake](AActor* RootActor) {
			InCallbackBeforeAwake.ExecuteIfBound(RootActor);
			};
#if WITH_EDITOR
		switch ((ELPrefabVersion)PrefabVersion)
		{
		case ELPrefabVersion::NewObjectOnNestedPrefab:
		{
			LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ActorAttachToSubPrefab:
		{
			LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::CommonActor:
		{
			LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ObjectName:
		{
			LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		default:
		{
			LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
		}
		break;
		}
#else
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation.Quaternion(), Scale, CallbackBeforeAwake);
#endif
	}
	return LoadedRootActor;
}
AActor* ULPrefab::LoadPrefabWithReplacement(UObject* WorldContextObject, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, const FLPrefab_LoadPrefabCallback& InCallbackBeforeAwake)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		TSet<TTuple<int, UObject*>> ReplacedAssets;
		TSet<TTuple<int, UClass*>> ReplacedClasses;
		if (InReplaceAssetMap.Num() > 0)
		{
			auto& List =
#if WITH_EDITOR
				ReferenceAssetList;
#else
				ReferenceAssetListForBuild;
#endif
			for (int i = 0; i < List.Num(); i++)
			{
				if (auto ReplaceAssetPtr = InReplaceAssetMap.Find(List[i]))
				{
					ReplacedAssets.Add({ i, List[i] });
					List[i] = *ReplaceAssetPtr;
				}
			}
		}
		if (InReplaceClassMap.Num() > 0)
		{
			auto& List =
#if WITH_EDITOR
				ReferenceClassList;
#else
				ReferenceClassListForBuild;
#endif
			for (int i = 0; i < List.Num(); i++)
			{
				if (auto ReplaceClassPtr = InReplaceClassMap.Find(List[i]))
				{
					ReplacedClasses.Add({ i, List[i] });
					List[i] = *ReplaceClassPtr;
				}
			}
		}
		auto CallbackBeforeAwake = [&InCallbackBeforeAwake](AActor* RootActor) {
			InCallbackBeforeAwake.ExecuteIfBound(RootActor);
			};
#if WITH_EDITOR
		switch ((ELPrefabVersion)PrefabVersion)
		{
		case ELPrefabVersion::NewObjectOnNestedPrefab:
		{
			LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ActorAttachToSubPrefab:
		{
			LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::CommonActor:
		{
			LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ObjectName:
		{
			LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
		}
		break;
		default:
		{
			UE_LOG(LPrefab, Error, TEXT("[%s].%d This prefab version is too old to support this function, open this prefab and hit \"Apply\" button to fix it. Prefab: '%s'"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
		}
		break;
		}
#else
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, false, CallbackBeforeAwake);
#endif
		if (ReplacedAssets.Num() > 0)
		{
			auto& List =
#if WITH_EDITOR
				ReferenceAssetList;
#else
				ReferenceAssetListForBuild;
#endif
			for (auto& Item : ReplacedAssets)
			{
				List[Item.Key] = Item.Value;
			}
		}
		if (ReplacedClasses.Num() > 0)
		{
			auto& List =
#if WITH_EDITOR
				ReferenceClassList;
#else
				ReferenceClassListForBuild;
#endif
			for (auto& Item : ReplacedClasses)
			{
				List[Item.Key] = Item.Value;
			}
		}
	}
	return LoadedRootActor;
}
AActor* ULPrefab::LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale, const TFunction<void(AActor*)>& InCallbackBeforeAwake)
{
	AActor* LoadedRootActor = nullptr;
	auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
#if WITH_EDITOR
		switch ((ELPrefabVersion)PrefabVersion)
		{
		case ELPrefabVersion::NewObjectOnNestedPrefab:
		{
			LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ActorAttachToSubPrefab:
		{
			LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::CommonActor:
		{
			LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::ObjectName:
		{
			LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::NestedDefaultSubObject:
		{
			LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		case ELPrefabVersion::BuildinFArchive:
		{
			LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		default:
		{
			LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
		}
		break;
		}
#else
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefab(World, this, InParent, Location, Rotation, Scale, InCallbackBeforeAwake);
#endif
	}
	return LoadedRootActor;
}

#if WITH_EDITOR
AActor* ULPrefab::LoadPrefabWithExistingObjects(UWorld* InWorld, USceneComponent* InParent
	, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObject, TMap<TObjectPtr<AActor>, FLSubPrefabData>& OutSubPrefabMap
)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELPrefabVersion)PrefabVersion)
	{
	case ELPrefabVersion::NewObjectOnNestedPrefab:
	{
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ActorAttachToSubPrefab:
	{
		LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::CommonActor:
	{
		LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ObjectName:
	{
		LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::NestedDefaultSubObject:
	{
		LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::BuildinFArchive:
	{
		LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this, InParent
			, InOutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	default:
	{
		TArray<AActor*> OutCreatedActors;
		TArray<FGuid> OutCreatedActorsGuid;
		LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefabForEdit(InWorld, this, InParent
			, nullptr, nullptr, OutCreatedActors, OutCreatedActorsGuid);
		for (int i = 0; i < OutCreatedActors.Num(); i++)
		{
			InOutMapGuidToObject.Add(OutCreatedActorsGuid[i], OutCreatedActors[i]);
		}
	}
	break;
	}
	return LoadedRootActor;
}

bool ULPrefab::IsPrefabBelongsToThisSubPrefab(ULPrefab* InPrefab, bool InRecursive)
{
	MakeAgentObjectsInPreviewWorld();
	if (!PrefabHelperObject)return false;
	if (this == InPrefab)return false;
	for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
	{
		if (KeyValue.Value.PrefabAsset == InPrefab)
		{
			return true;
		}
	}
	if (InRecursive)
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.PrefabAsset->IsPrefabBelongsToThisSubPrefab(InPrefab, InRecursive))
			{
				return true;
			}
		}
	}
	return false;
}

void ULPrefab::CopyDataTo(ULPrefab* TargetPrefab)
{
	TargetPrefab->ReferenceAssetList = this->ReferenceAssetList;
	TargetPrefab->ReferenceClassList = this->ReferenceClassList;
	TargetPrefab->ReferenceNameList = this->ReferenceNameList;
	TargetPrefab->ReferenceStringList = this->ReferenceStringList;
	TargetPrefab->ReferenceTextList = this->ReferenceTextList;
	TargetPrefab->BinaryData = this->BinaryData;
	TargetPrefab->PrefabVersion = this->PrefabVersion;
	TargetPrefab->EngineMajorVersion = this->EngineMajorVersion;
	TargetPrefab->EngineMinorVersion = this->EngineMinorVersion;
	TargetPrefab->EnginePatchVersion = this->EnginePatchVersion;
	TargetPrefab->ArchiveVersion = this->ArchiveVersion;
	TargetPrefab->ArchiveLicenseeVer = this->ArchiveLicenseeVer;
	TargetPrefab->ArEngineNetVer = this->ArEngineNetVer;
	TargetPrefab->ArGameNetVer = this->ArGameNetVer;
	TargetPrefab->PrefabDataForPrefabEditor = this->PrefabDataForPrefabEditor;
}

FString ULPrefab::GenerateOverallVersionMD5()
{
	struct LOCAL
	{
		static void CollectOverallPrefab(ULPrefab* Parent, TArray<ULPrefab*>& Collection)
		{
			Collection.Add(Parent);
			for (auto& Item : Parent->ReferenceAssetList)
			{
				if (auto SubPrefab = Cast<ULPrefab>(Item))
				{
					CollectOverallPrefab(SubPrefab, Collection);
				}
			}
		}
	};
	TArray<ULPrefab*> Collection;
	LOCAL::CollectOverallPrefab(this, Collection);
	Collection.Sort([](const ULPrefab& A, const ULPrefab& B) {
		return A.CreateTime > B.CreateTime;
		});

	FString CreateTimeOverall;
	for (auto& Item : Collection)
	{
		CreateTimeOverall += Item->CreateTime.ToIso8601();
	}
	return LPrefabUtils::GetMD5String(CreateTimeOverall);
}

void ULPrefab::SavePrefab(AActor* RootActor
	, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<TObjectPtr<AActor>, FLSubPrefabData>& InSubPrefabMap
	, bool InForEditorOrRuntimeUse
)
{
	LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::SavePrefab(RootActor, this
		, InOutMapObjectToGuid, InSubPrefabMap
		, InForEditorOrRuntimeUse
	);
}

void ULPrefab::RecreatePrefab()
{
	auto World = ULPrefabManagerObject::GetPreviewWorldForPrefabPackage();

	TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
	TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
	auto RootActor = this->LoadPrefabWithExistingObjects(World, nullptr
		, MapGuidToObject, SubPrefabMap
	);
	TMap<UObject*, FGuid> MapObjectToGuid;
	for (auto KeyValue : MapGuidToObject)
	{
		MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
	}
	this->SavePrefab(RootActor, MapObjectToGuid, SubPrefabMap);
	this->RefreshAgentObjectsInPreviewWorld();
}

AActor* ULPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELPrefabVersion)PrefabVersion)
	{
	case ELPrefabVersion::NewObjectOnNestedPrefab:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ActorAttachToSubPrefab:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::CommonActor:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ObjectName:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::NestedDefaultSubObject:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::BuildinFArchive:
	{
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<TObjectPtr<AActor>, FLSubPrefabData> SubPrefabMap;
		LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, MapGuidToObject, SubPrefabMap
		);
	}
	break;
	default:
	{
		LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	break;
	}
	return LoadedRootActor;
}

AActor* ULPrefab::LoadPrefabInEditor(UWorld* InWorld, USceneComponent* InParent, TMap<TObjectPtr<AActor>, FLSubPrefabData>& OutSubPrefabMap, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject, bool SetRelativeTransformToIdentity)
{
	AActor* LoadedRootActor = nullptr;
	switch ((ELPrefabVersion)PrefabVersion)
	{
	case ELPrefabVersion::NewObjectOnNestedPrefab:
	{
		LoadedRootActor = LPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ActorAttachToSubPrefab:
	{
		LoadedRootActor = LPrefabSystem7::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::CommonActor:
	{
		LoadedRootActor = LPrefabSystem6::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::ObjectName:
	{
		LoadedRootActor = LPrefabSystem5::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::NestedDefaultSubObject:
	{
		LoadedRootActor = LPrefabSystem4::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	case ELPrefabVersion::BuildinFArchive:
	{
		LoadedRootActor = LPrefabSystem3::ActorSerializer::LoadPrefabWithExistingObjects(InWorld, this
			, InParent, OutMapGuidToObject, OutSubPrefabMap
		);
	}
	break;
	default:
	{
		LoadedRootActor = LPrefabSystem::ActorSerializer::LoadPrefabInEditor(InWorld, this
			, InParent);
	}
	break;
	}
	return LoadedRootActor;
}

#endif

#undef LOCTEXT_NAMESPACE