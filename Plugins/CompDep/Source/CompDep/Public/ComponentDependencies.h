#pragma once

#include "ComponentDependencies.generated.h"

UENUM(BlueprintType)
enum class EComponentDependencyType : uint8
{
	AnyOnActor,
	AnyOnActorWithTag,
	Child,
	ChildWithTag,
};

UINTERFACE(MinimalAPI, Blueprintable)
class UComponentDependencies : public UInterface
{
	GENERATED_BODY()
};

class COMPDEP_API IComponentDependencies
{
	GENERATED_BODY()
	
public:
	// TODO This should should be a USTRUCT for blueprint support (BlueprintType)
	typedef TTuple<EComponentDependencyType, UClass*, FName> Dependency;
	typedef TArray<Dependency> Dependencies;
	// TODO This should be a BlueprintNativeEvent for blueprint support; implement with _Implementation
	virtual Dependencies GetDependencies() const = 0;
};
