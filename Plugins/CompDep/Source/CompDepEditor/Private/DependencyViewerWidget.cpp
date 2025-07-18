// Fill out your copyright notice in the Description page of Project Settings.

#include "DependencyViewerWidget.h"

#include "ComponentDependencies.h"
#include "FDependencyUtils.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/KismetSystemLibrary.h"

TArray<FQueryDependencyBlueprintResult> UDependencyViewerWidget::QueryBlueprints(FString Filter, bool showOnlyUnfulfilled)
{
	TArray<FQueryDependencyBlueprintResult> queryResult{ QueryBlueprintsInternal() };

	if (!Filter.IsEmpty())
	{
		queryResult = queryResult.FilterByPredicate([&Filter](const FQueryDependencyBlueprintResult& Result)
		{
			return UKismetSystemLibrary::GetClassDisplayName(Result.BlueprintGC).Contains(Filter);
		});
	}
	
	if (showOnlyUnfulfilled)
	{
		queryResult = queryResult.FilterByPredicate([&Filter](const FQueryDependencyBlueprintResult& Result)
		{
			return Result.GetUnfulfilledDependencyCount() > 0;
		});
		
		queryResult.Sort([](const FQueryDependencyBlueprintResult& A, const FQueryDependencyBlueprintResult& B)
		{
			return A.GetUnfulfilledDependencyCount() > B.GetUnfulfilledDependencyCount();
		});
	}

	return queryResult;
}

TArray<FQueryDependencyBlueprintResult> UDependencyViewerWidget::QueryBlueprintsInternal()
{
	const FAssetRegistryModule& assetRegistry{ FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry") };

	TArray<FAssetData> assetDataList;
	assetRegistry.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), assetDataList);
	
	TArray<FQueryDependencyBlueprintResult> blueprintResults{};

	for (const FAssetData& assetData : assetDataList)
	{
		const UBlueprint* blueprintAsset{ Cast<UBlueprint>(assetData.GetAsset()) };
		if (!blueprintAsset) continue;
		
		UBlueprintGeneratedClass* blueprintGC{ Cast<UBlueprintGeneratedClass>(blueprintAsset->GeneratedClass) };
		if (!blueprintGC) continue;

		if (!blueprintGC->IsChildOf(AActor::StaticClass())) continue;
	
		TArray<FQueryDependencyComponentResult> componentResults{};
		
		TArray<UActorComponent*> components{ FDependencyUtils::GetAllBlueprintComponentsByName(blueprintGC) };
		for (UActorComponent* innerComp : components)
		{
			if (!innerComp->GetClass()->ImplementsInterface(UComponentDependencies::StaticClass())) continue;
			TArray<FQueryDependencyResult> dependencyResults{};
			
			TArray<FComponentDependency> dependencies{ IComponentDependencies::Execute_GetDependencies(innerComp) };
			for (FComponentDependency& dependency : dependencies)
			{
				dependencyResults.Emplace(
					dependency,
					FDependencyUtils::CheckDependency(innerComp, dependency)
				);
			}

			if (dependencyResults.IsEmpty()) continue;
			componentResults.Emplace(
				innerComp,
				MoveTemp(dependencyResults)
			);
		}

		if (componentResults.IsEmpty()) continue;

		blueprintResults.Emplace(blueprintGC, MoveTemp(componentResults));
	}

	return blueprintResults;
}

UTexture2D* UDependencyViewerWidget::GetThumbnail(const UBlueprintGeneratedClass* BlueprintGC, int Size)
{
	UBlueprint* blueprint{ BlueprintGC->ClassGeneratedBy };
	if (!blueprint) return nullptr;

	FObjectThumbnail thumbnail;
	ThumbnailTools::RenderThumbnail(
		blueprint,
		Size,
		Size,
		ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush,
		nullptr,
		&thumbnail
	);

	const int32 width{ thumbnail.GetImageWidth() };
	const int32 height{ thumbnail.GetImageHeight() };
	const TArray<uint8>& bytes{ thumbnail.GetUncompressedImageData() };

	TArray<FColor> RawColors;
	RawColors.AddUninitialized(width * height);
	for (int32 i{}; i < width * height; i++)
	{
		const int32 bIndex{ i*4 };
		// Convert BGRA bytes to FColor
		RawColors[i] = FColor(bytes[bIndex+2], bytes[bIndex+1], bytes[bIndex+0], bytes[bIndex+3]);
	}
	
	UTexture2D* tex{ UTexture2D::CreateTransient(width, height, PF_B8G8R8A8, NAME_None, bytes) };
	return tex;
}
