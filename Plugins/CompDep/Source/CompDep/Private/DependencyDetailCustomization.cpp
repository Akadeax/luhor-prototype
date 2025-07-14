#include "DependencyDetailCustomization.h"

#include "ComponentDependencies.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"

#define LOCTEXT_NAMESPACE "Details"

void FDependencyDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<>> selectedList;
	DetailLayout.GetObjectsBeingCustomized(selectedList);

	if (selectedList.Num() != 1) return;
	const UObject* selectedObj{ selectedList[0].Get() };
	check(selectedObj);
	
	// Only operate on CDO (Blueprint, etc.)
	if (!selectedObj->IsTemplate()) return;
	
	const UActorComponent* selectedComp{ Cast<UActorComponent>(selectedObj) };
	check(selectedComp);

	const IComponentDependencies* selectedInterface{ Cast<IComponentDependencies>(selectedComp) };
	check(selectedInterface);

	IComponentDependencies::Dependencies dependencies{ selectedInterface->GetDependencies() };
	
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
	
	for (const IComponentDependencies::Dependency& dependency : dependencies)
	{
		auto [fulfilled, resultStr] = CheckDependencyFulfilled(selectedComp, dependency);

		FSlateColor textColor{ fulfilled ? FStyleColors::Success : FStyleColors::Error };
		
		const UClass* classType{ dependency.Get<1>() };
		const FString className{ classType->GetName() };

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
						.Text(LOCTEXT("Dependency", "Dependency:"))
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
					.Text(FText::FromString(resultStr))
					.ColorAndOpacity(textColor)
					.Margin(FMargin(15, 0))
					.AutoWrapText(true)
					.Font(resultTextFont)
				]				
			]
			
		];
	}
	
}

TPair<bool, FString> FDependencyDetailCustomization::CheckDependencyFulfilled(
	const UActorComponent* SourceComponent,
	IComponentDependencies::Dependency Dependency)
{
	const EComponentDependencyType type{ Dependency.Get<0>() };
	const UClass* classType{ Dependency.Get<1>() };
	const FName meta{ Dependency.Get<2>() };
	
	const UBlueprintGeneratedClass* blueprintGC{ Cast<UBlueprintGeneratedClass>(SourceComponent->GetOuter()->GetClass()) };
	if (!blueprintGC) return { false, "Generated Class not found! Are you editing a blueprint?" };
	const AActor* actorCDO{ blueprintGC->GetDefaultObject<AActor>() };
	if (!actorCDO) return { false, "Actor CDO not found! Are you editing a blueprint?" };
	const USimpleConstructionScript* constructionScript{ blueprintGC->SimpleConstructionScript };
	if (!constructionScript) return { false, "Construction Script not found! Are you editing a blueprint?" };

	const USCS_Node* componentSCSNode{};
	// We only need to locate the construction script node if this is a
	// SceneComponent, i.e. component hierarchy is involved
	if (SourceComponent->IsA<USceneComponent>())
	{
		USCS_Node* const* componentSCSNodeIt{ constructionScript->GetAllNodes().FindByPredicate(
			[&SourceComponent](const USCS_Node* innerNode)
			{
				return innerNode->ComponentTemplate.Get() == SourceComponent;
			}
		) };
		if (!componentSCSNodeIt) return { false, "Couldn't locate Scene Component in tree! Are you editing a blueprint?" };
		
		componentSCSNode = *componentSCSNodeIt;
	}

	const bool dependencyNeedsTree{ type == EComponentDependencyType::Child || type == EComponentDependencyType::ChildWithTag };
	if (dependencyNeedsTree && !componentSCSNode)
	{
		return { false, "Cannot have child dependency on an ActorComponent! Use a SceneComponent instead."};
	}


	TArray<UActorComponent*> componentList{};
	// This is only C++ components
	for (UActorComponent* innerComp : actorCDO->GetComponents())
	{
		componentList.Emplace(innerComp);
	}
	// This is only BP components
	for (const USCS_Node* node : constructionScript->GetAllNodes())
	{
		componentList.Emplace(node->ComponentTemplate.Get());
	}
	
	if (type == EComponentDependencyType::AnyOnActor)
	{
		for (const TObjectPtr<UActorComponent>& innerComp : componentList)
		{
			if (!innerComp->IsA(classType)) continue;
			
			return { true, "Present" };
		}
		
		return { false, "Not Present" };
	}
	if (type == EComponentDependencyType::AnyOnActorWithTag)
	{
		bool containsAnyWithTag{ false };
		bool containsAnyWithType{ false };
		bool containsCorrect{ false };
		
		for (const TObjectPtr<UActorComponent>& innerComp : componentList)
		{
			const bool isType{ innerComp->IsA(classType) };
			const bool hasTag{ innerComp->ComponentHasTag(meta) };

			if (isType) containsAnyWithType = true;
			if (hasTag) containsAnyWithTag = true;
			if (isType && hasTag)
			{
				containsCorrect = true;
				break;
			}
		}

		if (containsCorrect) return { true, "Present with Correct Tag" };

		if (containsAnyWithType) return { false, "Correct Type, Wrong Tag" };
		if (containsAnyWithTag)  return { false, "Wrong Type, Correct Tag" };

		return { false, "Not Present" };
	}
	if (type == EComponentDependencyType::Child)
	{
		
	}
	
	return { false, "Unknown error! Is your dependency type valid?" };
}

#undef LOCTEXT_NAMESPACE
