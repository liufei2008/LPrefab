// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LPrefabSequencePlayer.h"
#include "PrefabAnimation/LPrefabSequenceComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"


UObject* ULPrefabSequencePlayer::GetPlaybackContext() const
{
	ULPrefabSequence* PrefabSequence = CastChecked<ULPrefabSequence>(Sequence);
	if (PrefabSequence)
	{
		auto Component = PrefabSequence->GetTypedOuter<ULPrefabSequenceComponent>();
		return Component->GetOwner();
	}

	return nullptr;
}

TArray<UObject*> ULPrefabSequencePlayer::GetEventContexts() const
{
	TArray<UObject*> Contexts;
	if (UObject* PlaybackContext = GetPlaybackContext())
	{
		Contexts.Add(PlaybackContext);
	}
	return Contexts;
}