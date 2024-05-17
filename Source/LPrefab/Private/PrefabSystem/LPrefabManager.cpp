// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefabManager.h"
#include "LPrefabModule.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
#include "EditorViewportClient.h"
#include "PrefabSystem/LPrefab.h"
#include "EngineUtils.h"
#endif

#define LOCTEXT_NAMESPACE "LPrefabManagerObject"

#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITOR
class FLPrefabObjectCreateDeleteListener : public FUObjectArray::FUObjectCreateListener, public FUObjectArray::FUObjectDeleteListener
{
public:
	ULPrefabManagerObject* Manager = nullptr;
	FLPrefabObjectCreateDeleteListener(ULPrefabManagerObject* InManager)
	{
		Manager = InManager;
		GUObjectArray.AddUObjectCreateListener(this);
		GUObjectArray.AddUObjectDeleteListener(this);
	}
	~FLPrefabObjectCreateDeleteListener()
	{
		GUObjectArray.RemoveUObjectCreateListener(this);
		GUObjectArray.RemoveUObjectDeleteListener(this);
	}

	virtual void NotifyUObjectCreated(const class UObjectBase* Object, int32 Index)override
	{
		if (auto Comp = Cast<UActorComponent>((UObject*)Object))
		{
			if (Comp->IsVisualizationComponent())return;
			if (auto Actor = Comp->GetOwner())
			{
				Manager->OnComponentCreateDelete().Broadcast(true, Comp, Actor);
			}
		}
	}
	virtual void NotifyUObjectDeleted(const class UObjectBase* Object, int32 Index)override
	{
		if (auto Comp = Cast<UActorComponent>((UObject*)Object))
		{
			if (Comp->IsVisualizationComponent())return;
			if (auto Actor = Comp->GetOwner())
			{
				Manager->OnComponentCreateDelete().Broadcast(false, Comp, Actor);
			}
		}
	}
	virtual void OnUObjectArrayShutdown()override {};
};
#endif

ULPrefabManagerObject* ULPrefabManagerObject::Instance = nullptr;
ULPrefabManagerObject::ULPrefabManagerObject()
{

}
void ULPrefabManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnAssetReimportDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			if (auto ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
			{
				ImportSubsystem->OnAssetReimport.Remove(OnAssetReimportDelegateHandle);
			}
		}
	}
	if (OnMapOpenedDelegateHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	}
	if (OnPackageReloadedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnPackageReloaded.Remove(OnPackageReloadedDelegateHandle);
	}
	if (OnBlueprintPreCompileDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileDelegateHandle);
		}
	}
	if (OnBlueprintCompiledDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintCompiled().Remove(OnBlueprintCompiledDelegateHandle);
		}
	}

	//cleanup preview world
	if (PreviewWorldForPrefabPackage && GEngine)
	{
		PreviewWorldForPrefabPackage->CleanupWorld();
		GEngine->DestroyWorldContext(PreviewWorldForPrefabPackage);
		PreviewWorldForPrefabPackage->ReleasePhysicsScene();
	}

	delete ObjectCreateDeleteListener;
	ObjectCreateDeleteListener = nullptr;
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULPrefabManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITORONLY_DATA
	if (EditorTick.IsBound())
	{
		EditorTick.Broadcast(DeltaTime);
	}
	if (OneShotFunctionsToExecuteInTick.Num() > 0)
	{
		for (int i = 0; i < OneShotFunctionsToExecuteInTick.Num(); i++)
		{
			auto& Item = OneShotFunctionsToExecuteInTick[i];
			if (Item.Key <= 0)
			{
				Item.Value();
				OneShotFunctionsToExecuteInTick.RemoveAt(i);
				i--;
			}
			else
			{
				Item.Key--;
			}
		}
	}
#endif
#if WITH_EDITOR
	if (bShouldBroadcastLevelActorListChanged)
	{
		bShouldBroadcastLevelActorListChanged = false;
		if (IsValid(GEditor))
		{
			GEditor->BroadcastLevelActorListChanged();
		}
	}
#endif
}
TStatId ULPrefabManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULPrefabManagerObject, STATGROUP_Tickables);
}

#if WITH_EDITOR

void ULPrefabManagerObject::AddOneShotTickFunction(const TFunction<void()>& InFunction, int InDelayFrameCount)
{
	InitCheck();
	InDelayFrameCount = FMath::Max(0, InDelayFrameCount);
	TTuple<int, TFunction<void()>> Item;
	Item.Key = InDelayFrameCount;
	Item.Value = InFunction;
	Instance->OneShotFunctionsToExecuteInTick.Add(Item);
}
FDelegateHandle ULPrefabManagerObject::RegisterEditorTickFunction(const TFunction<void(float)>& InFunction)
{
	InitCheck();
	return Instance->EditorTick.AddLambda(InFunction);
}
void ULPrefabManagerObject::UnregisterEditorTickFunction(const FDelegateHandle& InDelegateHandle)
{
	if (Instance != nullptr)
	{
		Instance->EditorTick.Remove(InDelegateHandle);
	}
}

ULPrefabManagerObject* ULPrefabManagerObject::GetInstance(bool CreateIfNotValid)
{
	if (CreateIfNotValid)
	{
		InitCheck();
	}
	return Instance;
}
bool ULPrefabManagerObject::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULPrefabManagerObject>();
		Instance->AddToRoot();
		UE_LOG(LPrefab, Log, TEXT("[%s].%d No Instance for LPrefabManagerObject, create!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		//open map
		Instance->OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddUObject(Instance, &ULPrefabManagerObject::OnMapOpened);
		Instance->OnPackageReloadedDelegateHandle = FCoreUObjectDelegates::OnPackageReloaded.AddUObject(Instance, &ULPrefabManagerObject::OnPackageReloaded);
		if (GEditor)
		{
			//reimport asset
			Instance->OnAssetReimportDelegateHandle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(Instance, &ULPrefabManagerObject::OnAssetReimport);
			//blueprint recompile
			Instance->OnBlueprintPreCompileDelegateHandle = GEditor->OnBlueprintPreCompile().AddUObject(Instance, &ULPrefabManagerObject::OnBlueprintPreCompile);
			Instance->OnBlueprintCompiledDelegateHandle = GEditor->OnBlueprintCompiled().AddUObject(Instance, &ULPrefabManagerObject::OnBlueprintCompiled);
		}
		Instance->ObjectCreateDeleteListener = new FLPrefabObjectCreateDeleteListener(Instance);
	}
	return true;
}

void ULPrefabManagerObject::OnBlueprintPreCompile(UBlueprint* InBlueprint)
{
	bIsBlueprintCompiling = true;
}
void ULPrefabManagerObject::OnBlueprintCompiled()
{
	bIsBlueprintCompiling = true;
	AddOneShotTickFunction([this] {
		bIsBlueprintCompiling = false; 
		}, 2);
}

void ULPrefabManagerObject::OnAssetReimport(UObject* asset)
{
	if (IsValid(asset))
	{
		auto textureAsset = Cast<UTexture2D>(asset);
		if (IsValid(textureAsset))
		{
			
		}
	}
}

void ULPrefabManagerObject::OnMapOpened(const FString& FileName, bool AsTemplate)
{

}

void ULPrefabManagerObject::OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
{
	if (Phase == EPackageReloadPhase::PostBatchPostGC && Event != nullptr && Event->GetNewPackage() != nullptr)
	{
		auto Asset = Event->GetNewPackage()->FindAssetInPackage();
		if (auto PrefabAsset = Cast<ULPrefab>(Asset))
		{
			
		}
	}
}

UWorld* ULPrefabManagerObject::GetPreviewWorldForPrefabPackage()
{
	InitCheck();
	auto& PreviewWorldForPrefabPackage = Instance->PreviewWorldForPrefabPackage;
	if (PreviewWorldForPrefabPackage == nullptr)
	{
		FName UniqueWorldName = MakeUniqueObjectName(Instance, UWorld::StaticClass(), FName("LPrefab_PreviewWorldForPrefabPackage"));
		PreviewWorldForPrefabPackage = NewObject<UWorld>(Instance, UniqueWorldName);
		PreviewWorldForPrefabPackage->AddToRoot();
		PreviewWorldForPrefabPackage->WorldType = EWorldType::EditorPreview;

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(PreviewWorldForPrefabPackage->WorldType);
		WorldContext.SetCurrentWorld(PreviewWorldForPrefabPackage);

		PreviewWorldForPrefabPackage->InitializeNewWorld(UWorld::InitializationValues()
			.AllowAudioPlayback(false)
			.CreatePhysicsScene(false)
			.RequiresHitProxies(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
			.SetTransactional(false));
	}
	return PreviewWorldForPrefabPackage;
}
bool ULPrefabManagerObject::GetIsBlueprintCompiling()
{
	if (InitCheck())
	{
		return Instance->bIsBlueprintCompiling;
	}
	return false;
}
bool ULPrefabManagerObject::GetIsProcessingDelete()
{
	if (InitCheck())
	{
		return Instance->bIsProcessingDelete;
	}
	return false;
}

void ULPrefabManagerObject::MarkBroadcastLevelActorListChanged()
{
	if (Instance != nullptr)
	{
		Instance->bShouldBroadcastLevelActorListChanged = true;
	}
}

bool ULPrefabManagerObject::IsSelected(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	return GEditor->GetSelectedActors()->IsSelected(InObject);
}

bool ULPrefabManagerObject::AnySelectedIsChildOf(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator()); itr; ++itr)
	{
		auto itrActor = Cast<AActor>(*itr);
		if (IsValid(itrActor) && itrActor->IsAttachedTo(InObject))
		{
			return true;
		}
	}
	return false;
}

ULPrefabManagerObject::FSerialize_SortChildrenActors ULPrefabManagerObject::Serialize_SortChildrenActors;
ULPrefabManagerObject::FDeserialize_Components ULPrefabManagerObject::Deserialize_ProcessComponentsBeforeRerunConstructionScript;

ULPrefabManagerObject::FPrefabHelperObject_Refresh ULPrefabManagerObject::PrefabHelperObject_Refresh;
ULPrefabManagerObject::FPrefabHelperObject_ReplaceObjectPropertyForApplyOrRevert ULPrefabManagerObject::PrefabHelperObject_ReplaceObjectPropertyForApplyOrRevert;
ULPrefabManagerObject::FPrefabHelperObject_AfterObjectPropertyApplyOrRevert ULPrefabManagerObject::PrefabHelperObject_AfterObjectPropertyApplyOrRevert;
ULPrefabManagerObject::FPrefabHelperObject_AfterMakePrefabAsSubPrefab ULPrefabManagerObject::PrefabHelperObject_AfterMakePrefabAsSubPrefab;
ULPrefabManagerObject::FPrefabHelperObject_AfterCollectPropertyToOverride ULPrefabManagerObject::PrefabHelperObject_AfterCollectPropertyToOverride;
ULPrefabManagerObject::FPrefabHelperObject_CopyRootObjectParentAnchorData ULPrefabManagerObject::PrefabHelperObject_CopyRootObjectParentAnchorData;
#endif


ULPrefabWorldSubsystem* ULPrefabWorldSubsystem::GetInstance(UWorld* World)
{
	return World->GetSubsystem<ULPrefabWorldSubsystem>();
}
void ULPrefabWorldSubsystem::BeginPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	OnBeginDeserializeSession.Broadcast(InSessionId);
}
void ULPrefabWorldSubsystem::EndPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	OnEndDeserializeSession.Broadcast(InSessionId);
}
void ULPrefabWorldSubsystem::AddActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId)
{
	AllActors_PrefabSystemProcessing.Add(InActor, InSessionId);
}
void ULPrefabWorldSubsystem::RemoveActorForPrefabSystem(AActor* InActor, const FGuid& InSessionId)
{
	AllActors_PrefabSystemProcessing.Remove(InActor);
}
FGuid ULPrefabWorldSubsystem::GetPrefabSystemSessionIdForActor(AActor* InActor)
{
	if (auto FoundPtr = AllActors_PrefabSystemProcessing.Find(InActor))
	{
		return *FoundPtr;
	}
	return FGuid();
}

bool ULPrefabWorldSubsystem::IsLPrefabSystemProcessingActor(AActor* InActor)
{
	if (auto PrefabManager = ULPrefabWorldSubsystem::GetInstance(InActor->GetWorld()))
	{
		if (PrefabManager->IsPrefabSystemProcessingActor(InActor))
		{
			return true;
		}
	}
	return false;
}
bool ULPrefabWorldSubsystem::IsPrefabSystemProcessingActor(AActor* InActor)
{
	return AllActors_PrefabSystemProcessing.Contains(InActor);
}
#if LEXPREFAB_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
#undef LOCTEXT_NAMESPACE