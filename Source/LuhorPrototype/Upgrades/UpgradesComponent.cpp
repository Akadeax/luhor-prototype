// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradesComponent.h"
#include "BaseUpgrade.h"
#include "LuhorPrototype/LevelGameInstanceSubsystem.h"
// Sets default values for this component's properties
UUpgradesComponent::UUpgradesComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

FStatModifier UUpgradesComponent::GetCurrentModifier()
{
	return CurrentStats;
}

void UUpgradesComponent::AddUpgrade(TSubclassOf<UBaseUpgrade> upgradeClass)
{
	if (!*upgradeClass) return;
	
	UBaseUpgrade* instance = NewObject<UBaseUpgrade>(this, upgradeClass);
	instance->SetUpgradesComponent(this);
	UE_LOG(LogTemp, Display, TEXT("Adding Upgrade: %s"),*instance->GetTitle());
	Upgrades.Add(instance);
	RecalculateModifier();
}

void UUpgradesComponent::RemoveUpgrade(TSubclassOf<UBaseUpgrade> upgradeClass)
{
	if (!*upgradeClass) return;

	for (int32 i = Upgrades.Num() - 1; i >= 0; --i)
	{
		if (Upgrades[i] && Upgrades[i]->IsA(upgradeClass))
		{
			Upgrades.RemoveAt(i);
		}
	}

	RecalculateModifier();
}

void UUpgradesComponent::ClearUpgrades()
{
	Upgrades.Empty();
}


void UUpgradesComponent::BeginPlay()
{
	Super::BeginPlay();
	const ULevelGameInstanceSubsystem* sub{ GetWorld()->GetGameInstance()->GetSubsystem<ULevelGameInstanceSubsystem>() };
	check(sub);

	if (sub->PlayerSaveData.Health == -1) return;

	for (TSubclassOf<UBaseUpgrade> upgrade : sub->PlayerSaveData.Upgrades)
	{
		AddUpgrade(upgrade);
	}
}

TArray<TSubclassOf<UBaseUpgrade>> UUpgradesComponent::GetUpgrades() const
{
	TArray<TSubclassOf<UBaseUpgrade>> result;
	for (auto& upgrade : Upgrades)
	{
		result.Add(upgrade->GetClass());
	}
	return result;
}


void UUpgradesComponent::RecalculateModifier()
{
	CurrentStats = {};
	for (UBaseUpgrade* upgrade : Upgrades)
	{
		if (upgrade->IsUpgradeActive)
		{
			CurrentStats += upgrade->GetStatModifier();
		}
	}
}



