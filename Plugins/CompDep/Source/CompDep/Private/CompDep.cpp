#include "CompDep.h"

#include "ComponentDependencies.h"
#include "DependencyDetailCustomization.h"

void FCompDepModule::StartupModule()
{
	FPropertyEditorModule& PEM{ FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor") };

	for (TObjectIterator<UClass> it; it; ++it)
	{
		const UClass* current{ *it };
		check(current);
		if (!current->ImplementsInterface(UComponentDependencies::StaticClass())) continue;

		checkf(
			current->IsChildOf(UActorComponent::StaticClass()),
			TEXT("%s is not an ActorComponent but is trying to implement component dependencies!"),
			*current->GetName()
		);
		
		PEM.RegisterCustomClassLayout(
			current->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FDependencyDetailCustomization::MakeInstance)
		);
		RegisteredClassLayoutNames.Emplace(current->GetFName());
	}
}

void FCompDepModule::ShutdownModule()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor")) return;
	FPropertyEditorModule& PEM{ FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor") };
	
	for (const FName& classLayoutName : RegisteredClassLayoutNames)
	{
		PEM.UnregisterCustomClassLayout(classLayoutName);
	}
}

IMPLEMENT_MODULE(FCompDepModule, CompDep)