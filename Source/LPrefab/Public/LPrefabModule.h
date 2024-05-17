// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "Stats/Stats.h"
#include "Modules/ModuleInterface.h"

LPREFAB_API DECLARE_LOG_CATEGORY_EXTERN(LPrefab, Log, All);
DECLARE_STATS_GROUP(TEXT("LPrefab"), STATGROUP_LexPrefab, STATCAT_Advanced);

//prevent compile optimization for easier code debug
#define LEXPREFAB_CAN_DISABLE_OPTIMIZATION 0

class FLPrefabModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
