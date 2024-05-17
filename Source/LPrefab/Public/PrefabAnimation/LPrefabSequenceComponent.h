// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "PrefabSystem/ILPrefabInterface.h"
#include "MovieSceneSequencePlayer.h"
//#include "LGUIComponentReference.h"
#include "LPrefabSequenceComponent.generated.h"


class ULPrefabSequence;
class ULPrefabSequencePlayer;

/**
 * Movie scene animation embedded within LPrefab.
 */
UCLASS(Blueprintable, ClassGroup=LPrefab, hidecategories=(Collision, Cooking, Activation), meta=(BlueprintSpawnableComponent))
class LPREFAB_API ULPrefabSequenceComponent
	: public UActorComponent, public ILPrefabInterface
{
public:
	GENERATED_BODY()

	ULPrefabSequenceComponent();

	UE_DEPRECATED(5.0, "Use GetSequenceByDisplayName instead.")
	UFUNCTION(BlueprintCallable, Category = LPrefab, meta=(DeprecatedFunction, DeprecationMessage = "Use GetSequenceByDisplayName instead"))
		ULPrefabSequence* GetSequenceByName(FName InName) const;
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		ULPrefabSequence* GetSequenceByDisplayName(const FString& InName) const;
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		ULPrefabSequence* GetSequenceByIndex(int32 InIndex) const;
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		const TArray<ULPrefabSequence*>& GetSequenceArray() const { return SequenceArray; }
	/** Init SequencePlayer with current sequence. */
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		void InitSequencePlayer();
	/** Find animation in SequenceArray by Index, then set it to SequencePlayer. */
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		void SetSequenceByIndex(int32 InIndex);
	/** Find animation in SequenceArray by Name, then set it to SequencePlayer */
	UE_DEPRECATED(5.0, "Use SetSequenceByDisplayName instead.")
	UFUNCTION(BlueprintCallable, Category = LPrefab, meta = (DeprecatedFunction, DeprecationMessage = "Use SetSequenceByDisplayName instead"))
		void SetSequenceByName(FName InName);
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		void SetSequenceByDisplayName(const FString& InName);
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		int32 GetCurrentSequenceIndex()const { return CurrentSequenceIndex; }

	UFUNCTION(BlueprintCallable, Category = LPrefab)
		ULPrefabSequence* GetCurrentSequence() const { return GetSequenceByIndex(CurrentSequenceIndex); }
	UFUNCTION(BlueprintCallable, Category = LPrefab)
		ULPrefabSequencePlayer* GetSequencePlayer() const { return SequencePlayer; }

	ULPrefabSequence* AddNewAnimation();
	bool DeleteAnimationByIndex(int32 InIndex);
	ULPrefabSequence* DuplicateAnimationByIndex(int32 InIndex);
	
	virtual void BeginPlay()override;
	// Begin ILPrefabInterface
	virtual void Awake_Implementation()override;
	// End ILPrefabInterface
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PreSave(class FObjectPreSaveContext SaveContext)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostInitProperties()override;
	virtual void PostLoad()override;

	void FixEditorHelpers();
	UBlueprint* GetSequenceBlueprint()const;
#endif
	UObject* GetSequenceBlueprintInstance()const
	{
		return nullptr;
		//SequenceEventHandler.GetComponent()
	}
protected:

	UPROPERTY(EditAnywhere, Category="Playback", meta=(ShowOnlyInnerProperties))
	FMovieSceneSequencePlaybackSettings PlaybackSettings;

	UPROPERTY(VisibleAnywhere, Instanced, Category= Playback)
		TArray<TObjectPtr<ULPrefabSequence>> SequenceArray;
	UPROPERTY(EditAnywhere, Category = Playback)
		int32 CurrentSequenceIndex = 0;
	/**
	 * Use a Blueprint component to handle callback for event track.
	 * Not working: Add event in prefab the event can work no problem, but if close editor and open again, the event not fire at all.
	 */
	//UPROPERTY(/*EditAnywhere, Category = Playback*/)
	//	FLGUIComponentReference SequenceEventHandler;

	UPROPERTY(transient)
		TObjectPtr<ULPrefabSequencePlayer> SequencePlayer;
};
