// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorSettings.h"

FSimpleMulticastDelegate ULPrefabEditorSettings::LPrefabEditorSetting_PreserveHierarchyStateChange;
void ULPrefabEditorSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	auto MemberProperty = PropertyChangedEvent.MemberProperty;
	auto Property = PropertyChangedEvent.Property;
	if (MemberProperty && Property)
	{
		if (MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULPrefabEditorSettings, bPreserveHierarchyState))
		{
			LPrefabEditorSetting_PreserveHierarchyStateChange.Broadcast();
		}
	}
}
bool ULPrefabEditorSettings::GetPreserveHierarchyState()
{
	return GetDefault<ULPrefabEditorSettings>()->bPreserveHierarchyState;
}
float ULPrefabEditorSettings::GetDelayRestoreHierarchyTime()
{
	return GetDefault<ULPrefabEditorSettings>()->DelayRestoreHierarchyTime;
}
