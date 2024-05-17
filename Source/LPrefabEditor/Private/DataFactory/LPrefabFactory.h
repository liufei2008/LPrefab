// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "LPrefabFactory.generated.h"

UCLASS()
class ULPrefabFactory : public UFactory
{
	GENERATED_BODY()
public:
	ULPrefabFactory();

	class ULPrefab* SourcePrefab = nullptr;
	UClass* RootActorClass = nullptr;
	// UFactory interface
	virtual bool ConfigureProperties() override;
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	// End of UFactory interface
};
