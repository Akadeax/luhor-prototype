#pragma once

#include "CoreMinimal.h"
#include "CompDep/Public/ComponentDependencyStructs.h"
#include "FDependencyUtils.generated.h"

USTRUCT(BlueprintType)
struct FDependencyFulfilledResult
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsFulfilled{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ShowDependency{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OutputString{};
};

class FDependencyUtils
{
public:
	static FDependencyFulfilledResult CheckDependency(
		const UActorComponent* SourceComponent,
		const FComponentDependency& Dependency
	);

	static FText GetDependencyDescriptionText(const FComponentDependency& Dependency);
	static TArray<UActorComponent*> GetAllBlueprintComponentsByName(const UBlueprintGeneratedClass* BlueprintClass);

private:
	static FDependencyFulfilledResult GetDependencyResult(
		const UActorComponent* SourceComponent,
		const FComponentDependency& Dependency
	);
};
