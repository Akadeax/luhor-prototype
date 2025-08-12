// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUpgrade.h"
#include "LuhorPrototype/AmbrosiaHealthComponent.h"
#include "HighRiskUpgrade.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UHighRiskUpgrade : public UBaseUpgrade
{
	GENERATED_BODY()
	
	UHighRiskUpgrade()
	{
		Modifier.MeleeAttackMultiplier = 2;
		Modifier.AmbrosiaDrainMultiplier = 1.25;
		Modifier.SiphonSpeedMultiplier = 0.75;
		
		UpgradeName = "High Risk High Reward";
		UpgradeText = "When you are below 50% ambrosia your damage Tripples, but your ambrosia drains 25% faster and you siphon 25% slower";
		IsUpgradeActive = false;
	}
	virtual void Init() override
	{
		PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass()));
		PlayerHealthComp->OnHealed.AddDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
		PlayerHealthComp->OnDamaged.AddDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
	}
	UPROPERTY()
	UAmbrosiaHealthComponent* PlayerHealthComp{nullptr};
	UPROPERTY(EditDefaultsOnly)
	float HealthActivationCutoff{0.5f};
	UFUNCTION()
	void OnHealthChanged()
	{
		float healthPercentage = PlayerHealthComp->GetCurrentHealth() / PlayerHealthComp->GetMaxHealth();

		bool shouldBeActive = healthPercentage < HealthActivationCutoff;
		if (IsUpgradeActive != shouldBeActive)
		{
			IsUpgradeActive = shouldBeActive;
			UpgradesComponent->RecalculateModifier();
		}
	}
};

