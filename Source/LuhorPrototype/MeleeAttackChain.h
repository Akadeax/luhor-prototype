// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MeleeAttackChain.generated.h"

USTRUCT(BlueprintType)
struct LUHORPROTOTYPE_API FMeleeAttackData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WindupTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ContactTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RecoveryTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage{ 5.f };
};


UCLASS()
class LUHORPROTOTYPE_API UMeleeAttackChain : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FMeleeAttackData> Attacks{};
};
