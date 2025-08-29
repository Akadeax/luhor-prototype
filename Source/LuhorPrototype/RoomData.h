// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomData.generated.h"

USTRUCT(BlueprintType)
struct LUHORPROTOTYPE_API FRoom
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath Level;
};

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API URoomData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoom> Rooms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRoom UpgradeRoom;
};
