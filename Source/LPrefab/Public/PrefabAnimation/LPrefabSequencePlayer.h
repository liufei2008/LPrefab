// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "IMovieScenePlayer.h"
#include "LPrefabSequence.h"
#include "MovieSceneSequencePlayer.h"
#include "LPrefabSequencePlayer.generated.h"

/**
 * ULPrefabSequencePlayer is used to actually "play" an actor sequence asset at runtime.
 */
UCLASS(BlueprintType)
class LPREFAB_API ULPrefabSequencePlayer
	: public UMovieSceneSequencePlayer
{
public:
	GENERATED_BODY()

protected:

	//~ IMovieScenePlayer interface
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;
};

