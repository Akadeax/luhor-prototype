#include "FDependencyUtils.h"

#include "Engine/InheritableComponentHandler.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"

FDependencyFulfilledResult FDependencyUtils::CheckDependency(
	const UActorComponent* SourceComponent,
	const FComponentDependency& Dependency)
{
	FDependencyFulfilledResult result{ GetDependencyResult(SourceComponent, Dependency) };
	
	if (Dependency.Type == EComponentDependencyType::Optional && !result.IsFulfilled)
	{
		result.OutputString = "Not Present; Optional";
	}

	return result;
}

FDependencyFulfilledResult FDependencyUtils::GetDependencyResult(
	const UActorComponent* SourceComponent,
	const FComponentDependency& Dependency)
{
	if (Dependency.Class == nullptr) return { false, false, "No class given!" };
	
	UObject* outer{ SourceComponent->GetOuter() };
	UClass* outerClass{ outer->GetClass() };
	
	UBlueprintGeneratedClass* outerCast{ Cast<UBlueprintGeneratedClass>(outer) };
	UBlueprintGeneratedClass* classCast{ Cast<UBlueprintGeneratedClass>(outerClass) };
	if (!(outerCast || classCast)) return { false, false, "Cannot find outer of class! Are you editing a blueprint?" };

	// If SourceComponent is a component added in C++, then it's GetOuter() is the CDO;
	// If SourceComponent is a component added in the Blueprint, then GetOuter() is the BlueprintGeneratedClass;
	// Choose whichever is non-null (normally, it's always 1 of those 2 cases)
	UBlueprintGeneratedClass* blueprintGC{ outerCast ? outerCast : classCast };


	if (!blueprintGC) return { false, true, "Generated Class not found! Are you editing a blueprint?" };
	const AActor* actorCDO{ blueprintGC->GetDefaultObject<AActor>() };
	if (!actorCDO) return { false, true, "Actor CDO not found! Are you editing a blueprint?" };
	const USimpleConstructionScript* constructionScript{ blueprintGC->SimpleConstructionScript };
	if (!constructionScript) return { false, true, "Construction Script not found! Are you editing a blueprint?" };

	USCS_Node* componentSCSNode{};
	TMap<FName, USCS_Node*> nodes{};
	// Only locate the construction script node for SceneComponents
	if (SourceComponent->IsA<USceneComponent>())
	{
		for (UClass* curClass{ blueprintGC }; curClass; curClass = curClass->GetSuperClass())
		{
			UBlueprintGeneratedClass* bpClass{ Cast<UBlueprintGeneratedClass>(curClass) };
			if (!bpClass) continue;

			const USimpleConstructionScript* scs{ bpClass->SimpleConstructionScript };
			if (!scs) continue;

			for (USCS_Node* node : scs->GetAllNodes())
			{
				FName currentName{ node->ComponentTemplate->GetFName() };
				if (!nodes.Contains(currentName)) nodes.Add(currentName, node);
				
				if (currentName != SourceComponent->GetFName()) continue;

				componentSCSNode = node;
			}
		}

		if (!componentSCSNode)
		{
			return { false, true, TEXT("Are you editing a blueprint?") };
		}
	}
	
	const bool dependencyNeedsTree{ 
		Dependency.Position == EComponentDependencyPosition::Child || Dependency.Position == EComponentDependencyPosition::ChildWithTag
	};
	if (dependencyNeedsTree && !componentSCSNode)
	{
		return {
			false, true,
			"Cannot have child dependency on an ActorComponent! Use a SceneComponent instead."
		};
	}
	
	TArray<UActorComponent*> componentList{ FDependencyUtils::GetAllBlueprintComponentsByName(blueprintGC) };
	
	if (Dependency.Position == EComponentDependencyPosition::AnyOnActor)
	{
		for (const TObjectPtr<UActorComponent>& innerComp : componentList)
		{
			if (!innerComp->IsA(Dependency.Class)) continue;
			
			return { true, true, "Present" };
		}
		
		return { false, true, "Not Present" };
	}
	if (Dependency.Position == EComponentDependencyPosition::AnyOnActorWithTag)
	{
		bool containsAnyWithTag{ false };
		bool containsAnyWithType{ false };
		bool containsCorrect{ false };
		
		for (const TObjectPtr<UActorComponent>& innerComp : componentList)
		{
			const bool isType{ innerComp->IsA(Dependency.Class) };
			const bool hasTag{ innerComp->ComponentHasTag(Dependency.Meta) };
	
			if (isType) containsAnyWithType = true;
			if (hasTag) containsAnyWithTag = true;
			if (isType && hasTag)
			{
				containsCorrect = true;
				break;
			}
		}
	
		if (containsCorrect) return { true, true, "Present with Tag" };
	
		if (containsAnyWithType) return { false, true, "Correct Type, Wrong Tag" };
		if (containsAnyWithTag)  return { false, true, "Wrong Type, Correct Tag" };
	
		return { false, true, "Not Present" };
	}
	if (Dependency.Position == EComponentDependencyPosition::Child)
	{
		if (!componentSCSNode) return { false, true, "Couldn't locate SCS node!" };

		bool anyOfType{ false };
		for (const auto& [name, node] : nodes)
		{
			if (!node) continue;
			
			if (!node->ComponentTemplate->IsA(Dependency.Class)) continue;
			anyOfType = true;
			
			if (!node->IsChildOf(componentSCSNode)) continue;

			return { true, true, "Present as Child" };
		}

		if (anyOfType) return { false, true, "Exists, not as child" };
		return{ false, true, "Not Present" };
	}
	if (Dependency.Position == EComponentDependencyPosition::ChildWithTag)
	{
		if (!componentSCSNode) return { false, true, "Couldn't locate SCS node!" };

		bool containsAnyWithTagAndType{ false };
		bool containsChildWithCorrectType{ false };
		
		for (const auto& [name, node] : nodes)
		{
			if (!node) continue;
			if (!node->ComponentTemplate) continue;
			
			const bool correctType{ node->ComponentTemplate->IsA(Dependency.Class) };
			const bool correctTag{ node->ComponentTemplate->ComponentHasTag(Dependency.Meta) };
			const bool isChild{ node->IsChildOf(componentSCSNode) };

			if (correctType && correctTag) containsAnyWithTagAndType = true;
			if (correctType && isChild) containsChildWithCorrectType = true;
			
			if (correctType && correctTag && isChild)
			{
				return { true, true, "Present as Child" };
			}
		}

		if (containsAnyWithTagAndType)
		{
			return { false, true, "Exists, Not as Child" };
		}
		if (containsChildWithCorrectType)
		{
			return { false, true, "Child Exists, Wrong Tag" };
		}
		return{ false, true, "Not Present" };
	}
	
	return { false, true, "Unknown error! Is your dependency type valid?" };
}



FText FDependencyUtils::GetDependencyDescriptionText(const FComponentDependency& Dependency)
{
	const bool isOptional{ Dependency.Type == EComponentDependencyType::Optional };
	const bool isChild{
		Dependency.Position == EComponentDependencyPosition::Child ||
		Dependency.Position == EComponentDependencyPosition::ChildWithTag
	};
	const bool needsTag{
		Dependency.Position == EComponentDependencyPosition::AnyOnActorWithTag ||
		Dependency.Position == EComponentDependencyPosition::ChildWithTag
	};
	
	FString optionalText{ TEXT("Optional ") };
	FString childText{ TEXT("Child ") };
	FString tagText{ FString::Printf(TEXT(" (Tag '%s')"), *Dependency.Meta.ToString()) };
	FText dependencyText{ FText::Format(NSLOCTEXT("CompDep", "DependencyText", "{0}{1}Dependency{2}"),{
		FText::FromString(isOptional ? optionalText : ""),
		FText::FromString(isChild ? childText : ""),
		FText::FromString(needsTag ? tagText : ""),
	} )};

	return dependencyText;
}

TArray<UActorComponent*> FDependencyUtils::GetAllBlueprintComponentsByName(const UBlueprintGeneratedClass* BlueprintClass)
{
    if (!BlueprintClass) return {};
    
    TMap<FName, UActorComponent*> compMap;
    
    auto tryAddComp{ [&compMap](UActorComponent* InnerComp)
    {
        if (UActorComponent* TypedComponent{ Cast<UActorComponent>(InnerComp) })
        {
            const FName compName{ InnerComp->GetFName() };
            if (!compMap.Contains(compName)) 
            {
                compMap.Add(compName, TypedComponent);
            }
        }
    } };

    TArray<const UBlueprintGeneratedClass*> blueprintClassHierarchy;
    UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(BlueprintClass, blueprintClassHierarchy);

	// Process from most derived to least derived
    for (const UBlueprintGeneratedClass* currentClass : blueprintClassHierarchy)
    {
    	// Yes, this should be illegal, but for some reason GetInheritableComponentHandler
    	// is not const, so we *do* need to cast it away :(
    	UBlueprintGeneratedClass* currentClassMutable{ const_cast<UBlueprintGeneratedClass*>(currentClass) };
    	
        // Get C++ native components from CDO
        if (const AActor* actorCDO{ currentClass->GetDefaultObject<AActor>() })
        {
            TArray<UActorComponent*> nativeComps;
            actorCDO->GetComponents<UActorComponent>(nativeComps);
            
            for (UActorComponent* innerComp : nativeComps)
            {
                tryAddComp(innerComp);
            }
        }
        
        // Get Blueprint components from SimpleConstructionScript
        if (currentClass->SimpleConstructionScript)
        {
            for (const USCS_Node* node : currentClass->SimpleConstructionScript->GetAllNodes())
            {
                if (UActorComponent* compTemplate{ node->GetActualComponentTemplate(
                    const_cast<UBlueprintGeneratedClass*>(currentClass)) })
                {
                    tryAddComp(compTemplate);
                }
            }
        }
        
        // Get inherited/overridden components from this specific class
        if (const UInheritableComponentHandler* handler{ currentClassMutable->GetInheritableComponentHandler(false) })
        {
            TArray<UActorComponent*> inherited;
            handler->GetAllTemplates(inherited);
            
            for (UActorComponent* innerComp : inherited)
            {
                tryAddComp(innerComp);
            }
        }
    }
    
    // Convert map to array
    TArray<UActorComponent*> result;
    compMap.GenerateValueArray(result);
    
    return result;
}