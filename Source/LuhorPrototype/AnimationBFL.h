// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AnimationBFL.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UAnimationBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AnimationBFL")
	static float GetMontageSectionLength(UAnimMontage* Montage, FName SectionName);
};
