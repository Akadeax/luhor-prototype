// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LuhorMovementComponent.h"
#include "RangedAttackProjectile.h"
#include "Engine/DataAsset.h"
#include "AttackData.generated.h"

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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTransform HitBoxTransform{};	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCurvedLaunchData CurvedLaunchData{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UAnimMontage> Montage{};
};


UCLASS()
class LUHORPROTOTYPE_API UMeleeAttackChain : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FMeleeAttackData> Attacks{};
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ChainLeniencyTime{ 0.2f };
};


USTRUCT(BlueprintType)
struct LUHORPROTOTYPE_API FRangedAttackData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<ARangedAttackProjectile> ProjectileClass{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ProjectileSpeed{ 500.f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float WindupTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float ContactTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float RecoveryTime{ 0.2f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage{ 5.f };
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCurvedLaunchData CurvedLaunchData{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UAnimMontage> Montage{};
};


UCLASS()
class LUHORPROTOTYPE_API URangedAttack : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FRangedAttackData AttackData{};
};