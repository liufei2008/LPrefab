// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LPrefabEditorStyle.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FLPrefabEditorStyle::StyleInstance = NULL;

void FLPrefabEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLPrefabEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLPrefabEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LPrefabEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FLPrefabEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LPrefabEditorStyle"));
	Style->SetContentRoot((IPluginManager::Get().FindPlugin("LGUI") != nullptr ? IPluginManager::Get().FindPlugin("LGUI") : IPluginManager::Get().FindPlugin("LPrefab"))->GetBaseDir() / TEXT("Resources/Icons"));;

	Style->Set("ClassThumbnail.LPrefab", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LPrefab", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LPrefabLevelManagerActor", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LPrefabLevelManagerActor", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LPrefabHelperObject", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LPrefabHelperObject", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));

	Style->Set("LPrefabEditor.PrefabDataAction", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));

	Style->Set("PrefabMarkWhite", new IMAGE_BRUSH("PrefabMarkWhite_16x", Icon16x16));
	Style->Set("PrefabPlusMarkWhite", new IMAGE_BRUSH("PrefabPlusMarkWhite_16x", Icon16x16));
	Style->Set("PrefabVariantMarkWhite", new IMAGE_BRUSH("PrefabVariantMarkWhite_16x", Icon16x16));
	Style->Set("PrefabMarkBroken", new IMAGE_BRUSH("PrefabMarkBroken_16x", Icon16x16));

	Style->Set("LPrefabEditor.EditorTools", new IMAGE_BRUSH(TEXT("Button_Icon40"), FVector2D(40, 40)));

	FButtonStyle EmptyButton = FButtonStyle()
		.SetNormal(FSlateColorBrush(FColor(0, 39, 131, 0)))
		.SetHovered(FSlateColorBrush(FColor(0, 39, 131, 64)))
		.SetPressed(FSlateColorBrush(FColor(0, 39, 131, 128)));
	Style->Set("EmptyButton", EmptyButton);
	Style->Set("CanvasMark", new IMAGE_BRUSH("CanvasMark_16x", Icon16x16));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FLPrefabEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLPrefabEditorStyle::Get()
{
	return *StyleInstance;
}
