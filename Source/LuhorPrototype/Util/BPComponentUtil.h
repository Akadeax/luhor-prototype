// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BPComponentUtil.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UBPComponentUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=Class))
	static UObject* GetComponentOfClassWithTag(AActor* Actor, TSubclassOf<UObject> Class, FName Tag);
};
