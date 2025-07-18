#include "DependencyDetailCustomization.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "FDependencyUtils.h"
#include "CompDep/Public/ComponentDependencies.h"
#include "Engine/BlueprintGeneratedClass.h"

TArray<FName> FDependencyDetailCustomization::RegisteredClassLayoutNames{};

#define LOCTEXT_NAMESPACE "Details"

void FDependencyDetailCustomization::ReloadCustomizations()
{
	UnregisterCustomizations();
	RegisterCustomizations();
}

void FDependencyDetailCustomization::RegisterCustomizations()
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

void FDependencyDetailCustomization::UnregisterCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded("PropertyEditor")) return;
	FPropertyEditorModule& PEM{ FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor") };
	
	for (const FName& classLayoutName : RegisteredClassLayoutNames)
	{
		PEM.UnregisterCustomClassLayout(classLayoutName);
	}
	
	RegisteredClassLayoutNames.Empty();
}

void FDependencyDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<>> selectedList;
	DetailLayout.GetObjectsBeingCustomized(selectedList);

	// Disallow multi-select in editor while viewing dependencies
	if (selectedList.Num() != 1) return;
	
	const UObject* selectedObj{ selectedList[0].Get() };
	check(selectedObj);
	
	// Only operate on CDO (Blueprint, etc.)
	if (!selectedObj->IsTemplate()) return;
	
	const UActorComponent* selectedComp{ Cast<UActorComponent>(selectedObj) };
	// It's assumed this will always be on a component as a detail customization
	check(selectedComp);

	check(selectedComp->GetClass()->ImplementsInterface(UComponentDependencies::StaticClass()));

	// These are the dependencies actually defined in the C++ class or Blueprint implementation
	TArray<FComponentDependency> dependencies{ IComponentDependencies::Execute_GetDependencies(selectedComp) };
	
	IDetailCategoryBuilder& category{ DetailLayout.EditCategory(
		TEXT("Dependencies"),
		FText::GetEmpty(),
		ECategoryPriority::Important)
	};
	
	FSlateFontInfo dependencyTextFont{ IDetailLayoutBuilder::GetDetailFontBold() };
	dependencyTextFont.Size = 7;

	FSlateFontInfo dependencyNameFont{ IDetailLayoutBuilder::GetDetailFontBold() };
	dependencyNameFont.Size = 9;
	
	FSlateFontInfo resultTextFont{ IDetailLayoutBuilder::GetDetailFontBold() };
	resultTextFont.Size = 8;
	
	for (const FComponentDependency& dependency : dependencies)
	{
		if (dependency.Class == nullptr) continue;

		// Calc whether this dependency is fulfilled; if we're viewing in the wrong context, hide it
		FDependencyFulfilledResult result{ FDependencyUtils::CheckDependency(selectedComp, dependency) };
		if (!result.ShowDependency) continue;

		// Get color based on result
		const bool isOptional{ dependency.Type == EComponentDependencyType::Optional };
		FSlateColor textColor{ (result.IsFulfilled || isOptional) ? FStyleColors::Success : FStyleColors::Error };
		
		const FString className{ dependency.Class->GetName() };

		category.AddCustomRow(FText::FromString(className))
		[
			SNew(SBox)
			.Padding(5.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FDependencyUtils::GetDependencyDescriptionText(dependency))
						.Font(dependencyTextFont)
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(className))
						.Font(dependencyNameFont)
						.AutoWrapText(true)
					]
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(result.OutputString))
					.ColorAndOpacity(textColor)
					.Margin(FMargin(15, 0))
					.AutoWrapText(true)
					.Font(resultTextFont)
				]				
			]
		];
	}
}

#undef LOCTEXT_NAMESPACE
