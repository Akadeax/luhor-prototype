#include "HighRiskUpgrade.h"

UHighRiskUpgrade::UHighRiskUpgrade()
{
	Modifier.MeleeAttackMultiplier = 2;
	Modifier.AmbrosiaDrainMultiplier = 1.25;
	Modifier.SiphonSpeedMultiplier = 0.75;
		
	UpgradeName = "High Risk High Reward";
	UpgradeText = "When you are below 50% ambrosia your damage Triples, but your ambrosia drains 25% faster and you siphon 25% slower";
	IsUpgradeActive = false;
}

void UHighRiskUpgrade::Init()
{
	PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass()));
	PlayerHealthComp->OnHealed.AddDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
	PlayerHealthComp->OnDamaged.AddDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
}

void UHighRiskUpgrade::DeInit()
{
	PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass()));
	PlayerHealthComp->OnHealed.RemoveDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
	PlayerHealthComp->OnDamaged.RemoveDynamic(this, &UHighRiskUpgrade::OnHealthChanged);
}

void UHighRiskUpgrade::OnHealthChanged()
{
	float healthPercentage = PlayerHealthComp->GetCurrentHealth() / PlayerHealthComp->GetMaxHealth();

	bool shouldBeActive = healthPercentage < HealthActivationCutoff;
	if (IsUpgradeActive != shouldBeActive)
	{
		IsUpgradeActive = shouldBeActive;
		UpgradesComponent->RecalculateModifier();
	}
}
