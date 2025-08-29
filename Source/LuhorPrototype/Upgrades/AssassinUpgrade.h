// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LuhorPrototype/Upgrades/BaseUpgrade.h"
#include "AssassinUpgrade.generated.h"

class UAmbrosiaHealthComponent;
class UHittableComponent;
struct FHittableHitData;
class UMeleeAttackerComponent;
/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UAssassinUpgrade : public UBaseUpgrade
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void DeInit() override;

private:
	UPROPERTY() UMeleeAttackerComponent* DashAttacker;
	UPROPERTY() UAmbrosiaHealthComponent* Health;
	
	UFUNCTION() void OnDashAttackHit(FHittableHitData Data, bool WasLethal, UHittableComponent* Hittable);

	UPROPERTY(EditDefaultsOnly)
	float UpgradedSiphonPercent{ 120.f };
	
	UPROPERTY(EditDefaultsOnly)
	float UpgradedPoisonSiphonPercent{ 0.4f };
	
	float OldSiphonPercent;
	float OldPoisonSiphonPercent;
};
