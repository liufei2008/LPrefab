// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LPrefabSequenceComponent.h"
#include "PrefabAnimation/LPrefabSequence.h"
#include "PrefabAnimation/LPrefabSequencePlayer.h"
#include "LPrefabModule.h"
#include "PrefabSystem/LPrefabManager.h"

ULPrefabSequenceComponent::ULPrefabSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	//SequenceEventHandler = FLGUIComponentReference(UActorComponent::StaticClass());
}

void ULPrefabSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto ManagerInstance = ULPrefabWorldSubsystem::GetInstance(this->GetWorld()))
	{
		if (!ManagerInstance->IsPrefabSystemProcessingActor(this->GetOwner()))//if not processing by PrefabSystem, then mannually call initialize function
		{
			Awake_Implementation();
		}
	}
}
void ULPrefabSequenceComponent::Awake_Implementation()
{
	InitSequencePlayer();

	if (PlaybackSettings.bAutoPlay)
	{
		SequencePlayer->Play();
	}
}

void ULPrefabSequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (SequencePlayer)
	{
		SequencePlayer->Stop();
		SequencePlayer->TearDown();
	}
}

#if WITH_EDITOR
void ULPrefabSequenceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
void ULPrefabSequenceComponent::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
	FixEditorHelpers();
}
#include "UObject/ObjectSaveContext.h"
void ULPrefabSequenceComponent::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	FixEditorHelpers();
}
void ULPrefabSequenceComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
}
void ULPrefabSequenceComponent::PostInitProperties()
{
	Super::PostInitProperties();
}
void ULPrefabSequenceComponent::PostLoad()
{
	Super::PostLoad();
}
void ULPrefabSequenceComponent::FixEditorHelpers()
{
	for (auto& Sequence : SequenceArray)
	{
		if (Sequence->IsObjectReferencesGood(GetOwner()) && !Sequence->IsEditorHelpersGood(this->GetOwner()))
		{
			Sequence->FixEditorHelpers(this->GetOwner());
		}
	}
}

UBlueprint* ULPrefabSequenceComponent::GetSequenceBlueprint()const
{
	//if (auto Comp = SequenceEventHandler.GetComponent())
	//{
	//	if (auto GeneratedClass = Cast<UBlueprintGeneratedClass>(Comp->GetClass()))
	//	{
	//		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	//	}
	//}
	return nullptr;
}
#endif

ULPrefabSequence* ULPrefabSequenceComponent::GetSequenceByName(FName InName) const
{
	for (auto Item : SequenceArray)
	{
		if (Item->GetFName() == InName)
		{
			return Item;
		}
	}
	return nullptr;
}
ULPrefabSequence* ULPrefabSequenceComponent::GetSequenceByDisplayName(const FString& InName) const
{
	for (auto Item : SequenceArray)
	{
		if (Item->GetDisplayNameString() == InName)
		{
			return Item;
		}
	}
	return nullptr;
}
ULPrefabSequence* ULPrefabSequenceComponent::GetSequenceByIndex(int32 InIndex) const
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabSequenceComponent::GetSequenceByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return nullptr;
	}
	return SequenceArray[InIndex];
}

void ULPrefabSequenceComponent::InitSequencePlayer()
{
	if (!SequencePlayer)
	{
		SequencePlayer = NewObject<ULPrefabSequencePlayer>(this, "SequencePlayer");
		SequencePlayer->SetPlaybackClient(this);

		// Initialize this player for tick as soon as possible to ensure that a persistent
		// reference to the tick manager is maintained
		SequencePlayer->InitializeForTick(this);
	}
	if (auto CurrentSequence = GetCurrentSequence())
	{
		SequencePlayer->Initialize(CurrentSequence, PlaybackSettings);
	}
}
void ULPrefabSequenceComponent::SetSequenceByIndex(int32 InIndex)
{
	CurrentSequenceIndex = InIndex;
	InitSequencePlayer();
}
void ULPrefabSequenceComponent::SetSequenceByName(FName InName)
{
	int FoundIndex = -1;
	FoundIndex = SequenceArray.IndexOfByPredicate([InName](const ULPrefabSequence* Item) {
		return Item->GetFName() == InName;
		});
	if (FoundIndex != INDEX_NONE)
	{
		CurrentSequenceIndex = FoundIndex;
		InitSequencePlayer();
	}
}
void ULPrefabSequenceComponent::SetSequenceByDisplayName(const FString& InName)
{
	int FoundIndex = -1;
	FoundIndex = SequenceArray.IndexOfByPredicate([InName](const ULPrefabSequence* Item) {
		return Item->GetDisplayNameString() == InName;
		});
	if (FoundIndex != INDEX_NONE)
	{
		CurrentSequenceIndex = FoundIndex;
		InitSequencePlayer();
	}
}

ULPrefabSequence* ULPrefabSequenceComponent::AddNewAnimation()
{
	auto NewSequence = NewObject<ULPrefabSequence>(this, NAME_None, RF_Public | RF_Transactional);
	auto MovieScene = NewSequence->GetMovieScene();
	SequenceArray.Add(NewSequence);
	return NewSequence;
}

bool ULPrefabSequenceComponent::DeleteAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabSequenceComponent::DeleteAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return false;
	}
	if (auto SequenceItem = SequenceArray[InIndex])
	{
		SequenceItem->ConditionalBeginDestroy();
	}
	SequenceArray.RemoveAt(InIndex);
	return true;
}
ULPrefabSequence* ULPrefabSequenceComponent::DuplicateAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LPrefab, Error, TEXT("[ULPrefabSequenceComponent::DuplicateAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return nullptr;
	}
	auto SourceSequence = SequenceArray[InIndex];
	auto NewSequence = DuplicateObject(SourceSequence, this);
	NewSequence->SetDisplayNameString(NewSequence->GetName());
	{
		NewSequence->GetMovieScene()->SetTickResolutionDirectly(SourceSequence->GetMovieScene()->GetTickResolution());
		NewSequence->GetMovieScene()->SetPlaybackRange(SourceSequence->GetMovieScene()->GetPlaybackRange());
		NewSequence->GetMovieScene()->SetDisplayRate(SourceSequence->GetMovieScene()->GetDisplayRate());
	}
	SequenceArray.Insert(NewSequence, InIndex + 1);
	return NewSequence;
}
