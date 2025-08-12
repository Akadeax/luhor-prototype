// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpgradesComponent.generated.h"

class UBaseUpgrade;

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeAttackModifier = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MeleeAttackMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RangedAttackModifier = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RangedAttackMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AmbrosiaDrainMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SiphonSpeedMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefenseMultiplier = 1;
	
	FStatModifier& operator+=(const FStatModifier& other) {
		MeleeAttackModifier += other.MeleeAttackModifier;
		MeleeAttackMultiplier *= other.MeleeAttackMultiplier;
		RangedAttackModifier += other.RangedAttackModifier;
		RangedAttackMultiplier *= other.RangedAttackMultiplier;
		AmbrosiaDrainMultiplier *= other.AmbrosiaDrainMultiplier;
		SiphonSpeedMultiplier *= other.SiphonSpeedMultiplier;
		DefenseMultiplier *= other.DefenseMultiplier;
		return *this;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LUHORPROTOTYPE_API UUpgradesComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	UUpgradesComponent();
	// Called every frame
	void RecalculateModifier();
	FStatModifier GetCurrentModifier();
	
	UFUNCTION(BlueprintCallable)
	void AddUpgrade(TSubclassOf<UBaseUpgrade> upgradeClass);
	UFUNCTION(BlueprintCallable)
	void RemoveUpgrade(TSubclassOf<UBaseUpgrade> upgradeClass);
protected:
	// Called when the game starts

	FStatModifier CurrentStats{};
	
	TArray<UBaseUpgrade*> Upgrades;
};
