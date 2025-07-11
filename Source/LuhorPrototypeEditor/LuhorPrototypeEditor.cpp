#include "LuhorPrototypeEditor.h"

#include "Details.h"
#include "LuhorPrototype/HittableComponent.h"
#include "LuhorPrototype/MeleeAttackChain.h"
#define LOCTEXT_NAMESPACE "FLuhorPrototypeEditorModule"

void FLuhorPrototypeEditorModule::StartupModule()
{
    FPropertyEditorModule& PEM{ FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor") };
    PEM.RegisterCustomClassLayout(
        UHittableComponent::StaticClass()->GetFName(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FHittableComponentDetail::MakeInstance)
    );
    PEM.NotifyCustomizationModuleChanged();
}

void FLuhorPrototypeEditorModule::ShutdownModule()
{
    if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor")) return;

    FPropertyEditorModule& PEM{ FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor") };
    PEM.UnregisterCustomClassLayout(UHittableComponent::StaticClass()->GetFName());
    PEM.NotifyCustomizationModuleChanged();
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FLuhorPrototypeEditorModule, LuhorPrototypeEditor)