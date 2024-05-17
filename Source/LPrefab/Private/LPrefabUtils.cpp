// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabUtils.h"
#include "LPrefabModule.h"
#include "Sound/SoundBase.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif

#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

void LPrefabUtils::DestroyActorWithHierarchy(AActor* Target, bool WithHierarchy)
{
	if (!Target->IsValidLowLevelFast())
	{
		UE_LOG(LPrefab, Error, TEXT("[LPrefabUtils::DestroyActorWithHierarchy]Try to delete not valid actor"));
		return;
	}
	if (WithHierarchy)
	{
		TArray<AActor*> AllChildrenActors;
		CollectChildrenActors(Target, AllChildrenActors);//collect all actor
		for (auto item : AllChildrenActors)
		{
#if WITH_EDITOR
			if (auto world = item->GetWorld())
			{
				if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
				{
					world->EditorDestroyActor(item, true);
				}
				else
				{
					item->Destroy();
				}
			}
#else
			item->Destroy();
#endif
		}
	}
	else
	{
#if WITH_EDITOR
		if (auto world = Target->GetWorld())
		{
			if (world->WorldType == EWorldType::Editor || world->WorldType == EWorldType::EditorPreview)
			{
				world->EditorDestroyActor(Target, true);
			}
			else
			{
				Target->Destroy();
			}
		}
#else
		Target->Destroy();
#endif
	}
}
void LPrefabUtils::CollectChildrenActors(AActor* Target, TArray<AActor*>& AllChildrenActors, bool IncludeTarget)
{
	if (IncludeTarget)
	{
		AllChildrenActors.Add(Target);
	}
	TArray<AActor*> actorList;
	Target->GetAttachedActors(actorList);
	for (auto item : actorList)
	{
		CollectChildrenActors(item, AllChildrenActors, true);
	}
}

TArray<uint8> LPrefabUtils::GetMD5(const FString& InString)
{
	FMD5 Md5Gen;
	Md5Gen.Update((unsigned char*)TCHAR_TO_ANSI(*InString), FCString::Strlen(*InString));
	TArray<uint8> Digest;
	Digest.SetNumZeroed(16);
	Md5Gen.Final(Digest.GetData());
	return Digest;
}
FString LPrefabUtils::GetMD5String(const FString& InString)
{
	TArray<uint8> MD5Digest = GetMD5(InString);
	FString Md5String;
	for (TArray<uint8>::TConstIterator it(MD5Digest); it; ++it)
	{
		Md5String += FString::Printf(TEXT("%02x"), *it);
	}
	return Md5String;
}

#if WITH_EDITOR
void LPrefabUtils::NotifyPropertyChanged(UObject* Object, FProperty* Property)
{
	if (!IsValid(Object))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InValid object!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
	if (Property == nullptr)
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InValid property!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}

	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(Property);//@todo: how to build property chain?
	TArray<UObject*> ModifiedObjects;
	ModifiedObjects.Add(Object);
	FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet, MakeArrayView(ModifiedObjects));
	FPropertyChangedChainEvent PropertyChangedChainEvent(PropertyChain, PropertyChangedEvent);
	Object->PostEditChangeChainProperty(PropertyChangedChainEvent);
}
void LPrefabUtils::NotifyPropertyChanged(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyChanged(Object, Property);
}
void LPrefabUtils::NotifyPropertyPreChange(UObject* Object, FProperty* Property)
{
	if (!IsValid(Object))
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InValid object!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}
	if (Property == nullptr)
	{
		UE_LOG(LPrefab, Error, TEXT("[%s].%d InValid property!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		return;
	}

	FEditPropertyChain PropertyChain;
	PropertyChain.AddHead(Property);//@todo: how to build property chain?
	Object->PreEditChange(PropertyChain);
}
void LPrefabUtils::NotifyPropertyPreChange(UObject* Object, FName PropertyName)
{
	auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
	NotifyPropertyPreChange(Object, Property);
}
#endif

#if WITH_EDITOR
//nodify some informations in editor
void LPrefabUtils::EditorNotification(FText NofityText, float ExpireDuration)
{
	if (!IsValid(GEditor))return;
	FNotificationInfo Info(NofityText);
	Info.FadeInDuration = 0.1f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = ExpireDuration;
	Info.bUseSuccessFailIcons = false;
	Info.bUseLargeFont = false;
	Info.bFireAndForget = true;
	auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
	NotificationItem->ExpireAndFadeout();

	auto CompileFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
	GEditor->PlayEditorSound(CompileFailSound);
}
#endif


void LPrefabUtils::LogObjectFlags(UObject* obj)
{
	UE_LOG(LPrefab, Log, TEXT("object:%s\
\n	flagValue:%d\
\n	RF_Public:%d\
\n	RF_Standalone:%d\
\n	RF_MarkAsNative:%d\
\n	RF_Transactional:%d\
\n	RF_ClassDefaultObject:%d\
\n	RF_ArchetypeObject:%d\
\n	RF_Transient:%d\
\n	RF_MarkAsRootSet:%d\
\n	RF_TagGarbageTemp:%d\
\n	RF_NeedInitialization:%d\
\n	RF_NeedLoad:%d\
\n	RF_KeepForCooker:%d\
\n	RF_NeedPostLoad:%d\
\n	RF_NeedPostLoadSubobjects:%d\
\n	RF_NewerVersionExists:%d\
\n	RF_BeginDestroyed:%d\
\n	RF_FinishDestroyed:%d\
\n	RF_BeingRegenerated:%d\
\n	RF_DefaultSubObject:%d\
\n	RF_WasLoaded:%d\
\n	RF_TextExportTransient:%d\
\n	RF_LoadCompleted:%d\
\n	RF_InheritableComponentTemplate:%d\
\n	RF_DuplicateTransient:%d\
\n	RF_StrongRefOnFrame:%d\
\n	RF_NonPIEDuplicateTransient:%d\
\n	RF_WillBeLoaded:%d\
")
, *obj->GetPathName()
, obj->GetFlags()
, obj->HasAnyFlags(EObjectFlags::RF_Public)
, obj->HasAnyFlags(EObjectFlags::RF_Standalone)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsNative)
, obj->HasAnyFlags(EObjectFlags::RF_Transactional)
, obj->HasAnyFlags(EObjectFlags::RF_ClassDefaultObject)
, obj->HasAnyFlags(EObjectFlags::RF_ArchetypeObject)
, obj->HasAnyFlags(EObjectFlags::RF_Transient)
, obj->HasAnyFlags(EObjectFlags::RF_MarkAsRootSet)
, obj->HasAnyFlags(EObjectFlags::RF_TagGarbageTemp)
, obj->HasAnyFlags(EObjectFlags::RF_NeedInitialization)
, obj->HasAnyFlags(EObjectFlags::RF_NeedLoad)
, obj->HasAnyFlags(EObjectFlags::RF_KeepForCooker)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoad)
, obj->HasAnyFlags(EObjectFlags::RF_NeedPostLoadSubobjects)
, obj->HasAnyFlags(EObjectFlags::RF_NewerVersionExists)
, obj->HasAnyFlags(EObjectFlags::RF_BeginDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_FinishDestroyed)
, obj->HasAnyFlags(EObjectFlags::RF_BeingRegenerated)
, obj->HasAnyFlags(EObjectFlags::RF_DefaultSubObject)
, obj->HasAnyFlags(EObjectFlags::RF_WasLoaded)
, obj->HasAnyFlags(EObjectFlags::RF_TextExportTransient)
, obj->HasAnyFlags(EObjectFlags::RF_LoadCompleted)
, obj->HasAnyFlags(EObjectFlags::RF_InheritableComponentTemplate)
, obj->HasAnyFlags(EObjectFlags::RF_DuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_StrongRefOnFrame)
, obj->HasAnyFlags(EObjectFlags::RF_NonPIEDuplicateTransient)
, obj->HasAnyFlags(EObjectFlags::RF_WillBeLoaded)
);
}
void LPrefabUtils::LogClassFlags(UClass* cls)
{
	UE_LOG(LPrefab, Log, TEXT("class:%s\
\n	flagValue:%d\
\n	CLASS_Abstract:%d\
\n	CLASS_DefaultConfig:%d\
\n	CLASS_Config:%d\
\n	CLASS_Transient:%d\
\n	CLASS_Optional:%d\
\n	CLASS_MatchedSerializers:%d\
\n	CLASS_ProjectUserConfig:%d\
\n	CLASS_Native:%d\
\n	CLASS_NotPlaceable:%d\
\n	CLASS_PerObjectConfig:%d\
\n	CLASS_ReplicationDataIsSetUp:%d\
\n	CLASS_EditInlineNew:%d\
\n	CLASS_CollapseCategories:%d\
\n	CLASS_Interface:%d\
\n	CLASS_Const:%d\
\n	CLASS_NeedsDeferredDependencyLoading:%d\
\n	CLASS_CompiledFromBlueprint:%d\
\n	CLASS_MinimalAPI:%d\
\n	CLASS_RequiredAPI:%d\
\n	CLASS_DefaultToInstanced:%d\
\n	CLASS_TokenStreamAssembled:%d\
\n	CLASS_HasInstancedReference:%d\
\n	CLASS_Hidden:%d\
\n	CLASS_Deprecated:%d\
\n	CLASS_HideDropDown:%d\
\n	CLASS_GlobalUserConfig:%d\
\n	CLASS_Intrinsic:%d\
\n	CLASS_Constructed:%d\
\n	CLASS_ConfigDoNotCheckDefaults:%d\
\n	CLASS_NewerVersionExists:%d\
")
, *cls->GetPathName()
, cls->GetClassFlags()
, cls->HasAnyClassFlags(EClassFlags::CLASS_Abstract)
, cls->HasAnyClassFlags(EClassFlags::CLASS_DefaultConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Config)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Transient)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Optional)
, cls->HasAnyClassFlags(EClassFlags::CLASS_MatchedSerializers)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ProjectUserConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Native)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NotPlaceable)
, cls->HasAnyClassFlags(EClassFlags::CLASS_PerObjectConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ReplicationDataIsSetUp)
, cls->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
, cls->HasAnyClassFlags(EClassFlags::CLASS_CollapseCategories)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Interface)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Const)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NeedsDeferredDependencyLoading)
, cls->HasAnyClassFlags(EClassFlags::CLASS_CompiledFromBlueprint)
, cls->HasAnyClassFlags(EClassFlags::CLASS_MinimalAPI)
, cls->HasAnyClassFlags(EClassFlags::CLASS_RequiredAPI)
, cls->HasAnyClassFlags(EClassFlags::CLASS_DefaultToInstanced)
, cls->HasAnyClassFlags(EClassFlags::CLASS_TokenStreamAssembled)
, cls->HasAnyClassFlags(EClassFlags::CLASS_HasInstancedReference)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Hidden)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Deprecated)
, cls->HasAnyClassFlags(EClassFlags::CLASS_HideDropDown)
, cls->HasAnyClassFlags(EClassFlags::CLASS_GlobalUserConfig)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Intrinsic)
, cls->HasAnyClassFlags(EClassFlags::CLASS_Constructed)
, cls->HasAnyClassFlags(EClassFlags::CLASS_ConfigDoNotCheckDefaults)
, cls->HasAnyClassFlags(EClassFlags::CLASS_NewerVersionExists)
);
}

#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif