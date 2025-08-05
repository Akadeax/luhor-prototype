// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpgradesComponent.generated.h"

class BaseUpgrade;

struct StatModifier
{
	float AttackModifier = 0;
	float AttackMultiplier = 1;
	float AmbrosiaDrainMultiplier = 1;
	float DefenseMultiplier = 1;
	
	StatModifier& operator+=(const StatModifier& other) {
		AttackModifier += other.AttackModifier;
		AttackMultiplier *= other.AttackMultiplier;
		AmbrosiaDrainMultiplier *= other.AmbrosiaDrainMultiplier;
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
	void SetChangedFlag() {Changed = true;}
	StatModifier GetCurrentModifier();
	void AddUpgrade(BaseUpgrade* upgrade);
protected:
	// Called when the game starts

	StatModifier CurrentStats{};
	
	void CalculateModifier();
	TArray<BaseUpgrade*> Upgrades;
	bool Changed{true};
};
