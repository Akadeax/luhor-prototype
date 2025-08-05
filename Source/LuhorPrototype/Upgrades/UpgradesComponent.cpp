// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradesComponent.h"
#include "BaseUpgrade.h"
// Sets default values for this component's properties
UUpgradesComponent::UUpgradesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

StatModifier UUpgradesComponent::GetCurrentModifier()
{
	if (Changed)
	{
		CalculateModifier();
	}
	return CurrentStats;
}

void UUpgradesComponent::AddUpgrade(BaseUpgrade* upgrade)
{
	Upgrades.Add(upgrade);
	upgrade->SetUpgradesComponent(this);
	SetChangedFlag();
}

void UUpgradesComponent::CalculateModifier()
{
	CurrentStats = {};
	for (BaseUpgrade* upgrade : Upgrades)
	{
		CurrentStats += upgrade->GetStatModifier();
	}
	Changed = false;
}



