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
	UUpgradesComponent();
	void RecalculateModifier();
	UFUNCTION(BlueprintCallable)
	FStatModifier GetCurrentModifier();
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgradeAdded);
	UPROPERTY(BlueprintAssignable) FOnUpgradeAdded OnUpgradeAdded;
	
	UFUNCTION(BlueprintCallable)
	void AddUpgrade(TSubclassOf<UBaseUpgrade> UpgradeClass);
	UFUNCTION(BlueprintCallable)
	void RemoveUpgrade(TSubclassOf<UBaseUpgrade> UpgradeClass);
	UFUNCTION(BlueprintCallable)
	void ClearUpgrades();
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable)
	TArray<TSubclassOf<UBaseUpgrade>> GetUpgrades() const;

	UFUNCTION(BlueprintCallable)
	bool HasUpgrade(TSubclassOf<UBaseUpgrade> UpgradeClass) const;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	FStatModifier CurrentStats{};
	UPROPERTY()
	TArray<TObjectPtr<UBaseUpgrade>> Upgrades;
};
