// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FDependencyUtils.h"
#include "Editor/Blutility/Classes/EditorUtilityWidget.h"
#include "DependencyViewerWidget.generated.h"

USTRUCT(BlueprintType)
struct FQueryDependencyResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FComponentDependency Dependency;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDependencyFulfilledResult FulfilledResult;
};

USTRUCT(BlueprintType)
struct FQueryDependencyComponentResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UActorComponent* Component;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQueryDependencyResult> QueriedDependencies;

	int GetUnfulfilledDependencyCount() const
	{
		int count{};
		for (const FQueryDependencyResult res : QueriedDependencies)
		{
			if (res.Dependency.Type == EComponentDependencyType::Optional) continue;
			
			if (!res.FulfilledResult.IsFulfilled) ++count;
		}
		return count;
	}
};

USTRUCT(BlueprintType)
struct FQueryDependencyBlueprintResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlueprintGeneratedClass* BlueprintGC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQueryDependencyComponentResult> QueriedComponents;

	int GetUnfulfilledDependencyCount() const
	{
		int count{};
		for (const FQueryDependencyComponentResult& res : QueriedComponents)
		{
			count += res.GetUnfulfilledDependencyCount();
		}
		return count;
	}
};

UCLASS()
class COMPDEPEDITOR_API UDependencyViewerWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintCallable)
	static TArray<FQueryDependencyBlueprintResult> QueryBlueprints(FString Filter = "", bool showOnlyUnfulfilled = false);
	static TArray<FQueryDependencyBlueprintResult> QueryBlueprintsInternal();

	UFUNCTION(BlueprintCallable)
	static UTexture2D* GetThumbnail(const UBlueprintGeneratedClass* BlueprintGC, int Size = 64);
};
