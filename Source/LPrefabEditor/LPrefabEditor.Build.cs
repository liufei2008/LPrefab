// Copyright 2019-Present LexLiu. All Rights Reserved.

using UnrealBuildTool;

public class LPrefabEditor : ModuleRules
{
	public LPrefabEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Slate",
                "SlateCore",
                "Engine",
                "UnrealEd",
                "PropertyEditor",
                "RenderCore",
                "LPrefab",
                "LevelEditor",
                "Projects",
                "EditorWidgets",
                "DesktopPlatform",//file system
                "ImageWrapper",//texture load
                "InputCore",//STableRow
                "AssetTools",//Asset editor
                "ContentBrowser",//LGUI editor
                "SceneOutliner",//LPrefab editor, extend SceneOutliner
                "ApplicationCore",//ClipboardCopy
                "KismetCompiler",
                "AppFramework",
                //"AssetRegistry",
                //"InputCore",
				// ... add other public dependencies that you statically link with here ...
                
                "Kismet",
                "ToolMenus",//PrefabEditor
                "SubobjectEditor",//PrefabEditor, Actor component panel
                "Sequencer",
				"MovieScene",
				"MovieSceneTracks",
				"MovieSceneTools",
                "TypedElementFramework",
                "TypedElementRuntime",
                "EditorFramework",
                "PlacementMode",
                "ClassViewer",
            }
            );
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "EditorStyle",
				// ... add private dependencies that you statically link with here ...	

            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

    }
}
