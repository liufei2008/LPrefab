// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

class ULPrefab;

class FAssetTypeActions_LPrefab : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_LPrefab(EAssetTypeCategories::Type InAssetType);

	// FAssetTypeActions_Base overrides
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return FColor::White; }
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions ( const TArray<UObject*>& InObjects ) const override { return true; }
	virtual uint32 GetCategories() override;
	virtual bool CanFilter() override;
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;
private:

	EAssetTypeCategories::Type AssetType;
};
