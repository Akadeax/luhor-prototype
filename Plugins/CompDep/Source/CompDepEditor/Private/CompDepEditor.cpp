#include "CompDepEditor.h"

#include "DependencyDetailCustomization.h"
#include "Editor.h"
#include "EditorUtilitySubsystem.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "CompDep/Public/ComponentDependencies.h"

#define LOCTEXT_NAMESPACE "Details"

void FCompDepEditorModule::StartupModule()
{
	FDependencyDetailCustomization::RegisterCustomizations();
	InitializeMenu();
	InitializeReloadHooks();
}

void FCompDepEditorModule::ShutdownModule()
{
	FDependencyDetailCustomization::UnregisterCustomizations();

	if (GEditor)
	{
		GEditor->OnBlueprintPreCompile().Remove(BlueprintPreCompileDelegateHandle);
	}
	
	UToolMenus::UnregisterOwner(FToolMenuOwner(this));
	UToolMenus::UnRegisterStartupCallback(this);
}

void FCompDepEditorModule::InitializeMenu()
{
	UToolMenus::RegisterStartupCallback(FSimpleDelegate::CreateRaw(this, &FCompDepEditorModule::RegisterMenus));
}

void FCompDepEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped(this);

	UToolMenu* menu{ UToolMenus::Get()->ExtendMenu("MainFrame.MainMenu") };
	FToolMenuSection& section{ menu->FindOrAddSection(
		"comp_dep_menu",
		FText::FromString("CompDep"),
		FToolMenuInsert("comp_dep", EToolMenuInsertType::Last))
	};
	
	section.AddSubMenu(
		"comp_dep_menu",
		FText::FromString("CompDep"),
		FText::FromString(""),
		FNewToolMenuDelegate::CreateRaw(this, &FCompDepEditorModule::PopulateSubMenu),
		true,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Toolbar.Play")
	);
	

}

void FCompDepEditorModule::PopulateSubMenu(UToolMenu* Menu)
{
	FToolMenuSection& section{ Menu->FindOrAddSection("comp_dep", FText::FromString("CompDep")) };
	
	section.AddMenuEntry(
		"reload_component_dependencies",
		FText::FromString("Reload Component Dependencies"),
		FText::FromString(""),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Play"),
		FExecuteAction::CreateStatic(&FDependencyDetailCustomization::ReloadCustomizations)
	);
	
	section.AddMenuEntry(
		"open_dependency_viewer",
		FText::FromString("Open Dependency Viewer"),
		FText::FromString(""),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Fullscreen"),
		FExecuteAction::CreateRaw(this, &FCompDepEditorModule::OpenDependencyViewer)
	);
}

void FCompDepEditorModule::OpenDependencyViewer()
{
	if (!GEditor) return;

	UEditorUtilitySubsystem* editorUtility{
		GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>()
	};

	checkf(editorUtility, TEXT("Editor utility subsystem couldn't be loaded."));

	const FString widgetClassPath{ TEXT("/CompDep/Widgets/EUW_DependencyViewer.EUW_DependencyViewer") };
	UEditorUtilityWidgetBlueprint* widgetClass{
		LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, *widgetClassPath)
	};

	checkf(widgetClass, TEXT("Couldn't load dependency viewer! make sure the path is correct."));

	editorUtility->SpawnAndRegisterTab(widgetClass);
}

void FCompDepEditorModule::InitializeReloadHooks()
{
	// This only works because our loading phase is PostEngineInit; otherwise GEditor would be null
	if (GEditor)
	{
		// Any time a blueprint that implements "UComponentDependencies" is compiled, reload our
		// Detail customization; this is so blueprint-defined dependencies get reloaded properly.
		// Any C++-defined ones get reloaded on engine restart anyway
		BlueprintPreCompileDelegateHandle = GEditor->OnBlueprintPreCompile().AddLambda([](const UBlueprint* Blueprint)
		{
			if (!Blueprint) return;

			const UClass* generated{ Blueprint->GeneratedClass };
			if (!generated) return;
			
			if (!generated->ImplementsInterface(UComponentDependencies::StaticClass())) return;
			
			FDependencyDetailCustomization::ReloadCustomizations();
		});
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FCompDepEditorModule, CompDepEditor)
