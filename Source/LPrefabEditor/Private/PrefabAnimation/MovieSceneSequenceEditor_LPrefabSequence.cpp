// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "MovieSceneSequenceEditor_LPrefabSequence.h"
#include "ISequencerModule.h"
#include "PrefabAnimation/LPrefabSequence.h"
#include "PrefabAnimation/LPrefabSequenceComponent.h"

#define LOCTEXT_NAMESPACE "MovieSceneSequenceEditor_LPrefabSequence"

UBlueprint* FMovieSceneSequenceEditor_LPrefabSequence::GetBlueprintForSequence(UMovieSceneSequence* InSequence) const
{
	auto PrefabSequence = CastChecked<ULPrefabSequence>(InSequence);
	auto Component = PrefabSequence->GetTypedOuter<ULPrefabSequenceComponent>();
	return Component->GetSequenceBlueprint();
}

UBlueprint* FMovieSceneSequenceEditor_LPrefabSequence::CreateBlueprintForSequence(UMovieSceneSequence* InSequence) const
{
	auto PrefabSequence = CastChecked<ULPrefabSequence>(InSequence);
	auto Component = PrefabSequence->GetTypedOuter<ULPrefabSequenceComponent>();
	check(!Component->GetSequenceBlueprint());
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
