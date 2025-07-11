// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

class LUHORPROTOTYPE_API FComponentUtil
{
public:
	template<typename TClass>
	static TObjectPtr<TClass> GetChildComponentOfClass(USceneComponent* Parent);

	template<typename TClass>
	static TObjectPtr<TClass> GetFirstComponentOfClass(AActor* Parent);
};

template <typename TClass>
TObjectPtr<TClass> FComponentUtil::GetChildComponentOfClass(USceneComponent* Parent)
{
	const TArray<TObjectPtr<USceneComponent>>& children{ Parent->GetAttachChildren() };
	const TObjectPtr<USceneComponent>* first = children.FindByPredicate([](const TObjectPtr<USceneComponent>& comp)
	{
		return comp->IsA<TClass>();
	});

	if (first == nullptr) return nullptr;
	return Cast<TClass>(first->Get());
}

template <typename TClass>
TObjectPtr<TClass> FComponentUtil::GetFirstComponentOfClass(AActor* Parent)
{
	for (UActorComponent* comp : Parent->GetComponents())
	{
		if (comp->IsA<TClass>()) return Cast<TClass>(comp);
	}
	
	return nullptr;
}
