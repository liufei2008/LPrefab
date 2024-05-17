// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "ShaderCore.h"

#define LOCTEXT_NAMESPACE "FLPrefabModule"
DEFINE_LOG_CATEGORY(LPrefab);

void FLPrefabModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FLPrefabModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLPrefabModule, LPrefab)//if second param is wrong, an error like "EmptyLinkFunctionForStaticInitialization(XXX)" will occor when package project