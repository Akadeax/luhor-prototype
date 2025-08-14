// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float Health{ -1.f };

	UPROPERTY(BlueprintReadOnly)
	float PoisonedAmbrosia{ -1.f };
};
