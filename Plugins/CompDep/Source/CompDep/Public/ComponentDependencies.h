#pragma once

#include "ComponentDependencyStructs.h"
#include "ComponentDependencies.generated.h"

/*
 * To implement dependencies in C++, form your class declaration like this:
class UMyClass : public UMyComponentParent, public IComponentDependencies
{
	GENERATED_BODY()
	COMPDEP_DECL()
...
 * And put this in your implementation file:

COMPDEP_IMPL_START(ULuhorMovementComponent)
	COMPDEP_DEP_AnyOnActorRequired(UHittableComponent)
	COMPDEP_DEP_ChildWithTagOptional(UMeleeAttackerComponent, "my_tag")
COMPDEP_IMPL_END

 * These will then be displayed in the editor when selecting the component in a blueprint.
 */

#pragma region Helper Macros

#define COMPDEP_DECL() \
virtual TArray<FComponentDependency> GetDependencies_Implementation() const override;

// Only conditionally have content in the implementation; the implementation itself is still needed though.
#if WITH_EDITOR
	#define COMPDEP_IMPL_START(ClassName) \
	TArray<FComponentDependency> ClassName::GetDependencies_Implementation() const { return TArray<FComponentDependency>{
	#define COMPDEP_IMPL_END }; }

	#define COMPDEP_DEP_AnyOnActorRequired(DependencyClass) \
	FComponentDependency{ EComponentDependencyPosition::AnyOnActor, DependencyClass::StaticClass() },

	#define COMPDEP_DEP_AnyOnActorOptional(DependencyClass) \
	FComponentDependency{ EComponentDependencyPosition::AnyOnActor, DependencyClass::StaticClass(), "", EComponentDependencyType::Optional },

	#define COMPDEP_DEP_AnyOnActorWithTagRequired(DependencyClass, Tag) \
	FComponentDependency{ EComponentDependencyPosition::AnyOnActorWithTag, DependencyClass::StaticClass(), Tag },

	#define COMPDEP_DEP_AnyOnActorWithTagOptional(DependencyClass, Tag) \
	FComponentDependency{ EComponentDependencyPosition::AnyOnActorWithTag, DependencyClass::StaticClass(), Tag, EComponentDependencyType::Optional },

	#define COMPDEP_DEP_ChildRequired(DependencyClass) \
	FComponentDependency{ EComponentDependencyPosition::Child, DependencyClass::StaticClass() },

	#define COMPDEP_DEP_ChildOptional(DependencyClass) \
	FComponentDependency{ EComponentDependencyPosition::Child, DependencyClass::StaticClass(), "", EComponentDependencyType::Optional },

	#define COMPDEP_DEP_ChildWithTagRequired(DependencyClass, Tag) \
	FComponentDependency{ EComponentDependencyPosition::Child, DependencyClass::StaticClass(), Tag },

	#define COMPDEP_DEP_ChildWithTagOptional(DependencyClass, Tag) \
	FComponentDependency{ EComponentDependencyPosition::ChildWithTag, DependencyClass::StaticClass(), Tag, EComponentDependencyType::Optional },


#else
	#define COMPDEP_IMPL_START(ClassName, ...) \
	TArray<FComponentDependency> ClassName::GetDependencies_Implementation() const { return TArray<FComponentDependency>{
	#define COMPDEP_IMPL_END }; }
		
	#define COMPDEP_DEP_AnyOnActorRequired(DependencyClass)
	#define COMPDEP_DEP_AnyOnActorOptional(DependencyClass)
	#define COMPDEP_DEP_AnyOnActorWithTagRequired(DependencyClass, Tag) 
	#define COMPDEP_DEP_AnyOnActorWithTagOptional(DependencyClass, Tag) 
	#define COMPDEP_DEP_ChildRequired(DependencyClass) 
	#define COMPDEP_DEP_ChildOptional(DependencyClass) 
	#define COMPDEP_DEP_ChildWithTagRequired(DependencyClass, Tag) 
	#define COMPDEP_DEP_ChildWithTagOptional(DependencyClass, Tag) 
#endif
#pragma endregion


UINTERFACE(MinimalAPI, Blueprintable)
class UComponentDependencies : public UInterface
{
	GENERATED_BODY()
};

class COMPDEP_API IComponentDependencies
{
	GENERATED_BODY()
	
public:
	// It'd be amazing if we could compile this only with WITH_EDITOR, but then blueprints break at runtime
	UFUNCTION(BlueprintNativeEvent)
	TArray<FComponentDependency> GetDependencies() const;
};
