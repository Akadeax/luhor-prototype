#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencyStructs.generated.h"

UENUM(BlueprintType)
enum class EComponentDependencyType : uint8
{
	Required UMETA(ToolTip="The dependency is required; absence will be displayed as a warning"),
	Optional UMETA(ToolTip="The dependency is optional; absence will be displayed purely as information"),
};

UENUM(BlueprintType)
enum class EComponentDependencyPosition : uint8
{
	AnyOnActor UMETA(ToolTip="Fulfilled if any component of the given type exists on the actor at all"),
	AnyOnActorWithTag UMETA(ToolTip="Fulfilled if any component of the given type with the given tag exists on the actor"),
	Child UMETA(ToolTip="Fulfilled if the SceneComponent has a child of the given type"),
	ChildWithTag UMETA(ToolTip="Fulfilled if the SceneComponent has a child of the given type with the given tag"),
};

USTRUCT(BlueprintType)
struct FComponentDependency
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EComponentDependencyPosition Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UActorComponent> Class;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Meta{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EComponentDependencyType Type{ EComponentDependencyType::Required };
};