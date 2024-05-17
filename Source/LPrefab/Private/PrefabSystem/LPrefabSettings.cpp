// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LPrefabSettings.h"
#include "LPrefabModule.h"

#if WITH_EDITOR
void ULPrefabSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	auto MemberProperty = PropertyChangedEvent.MemberProperty;
	auto Property = PropertyChangedEvent.Property;
	if (MemberProperty && Property)
	{
		if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULPrefabSettings, ExtraPrefabFolders)
			|| (
				Property->GetFName() == GET_MEMBER_NAME_CHECKED(FDirectoryPath, Path)
				&& MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULPrefabSettings, ExtraPrefabFolders)
				)
			)
		{
			for (FDirectoryPath& PathToFix : ExtraPrefabFolders)
			{
				if (!PathToFix.Path.IsEmpty() && !PathToFix.Path.StartsWith(TEXT("/"), ESearchCase::CaseSensitive))
				{
					PathToFix.Path = FString::Printf(TEXT("/Game/%s"), *PathToFix.Path);
				}
			}
			if (IsValid(GEditor))
			{
				GEditor->BroadcastLevelActorListChanged();//refresh Outliner menu
			}
		}
	}
}
#endif

bool ULPrefabSettings::GetLogPrefabLoadTime()
{
	return GetDefault<ULPrefabSettings>()->bLogPrefabLoadTime;
}
