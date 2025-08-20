// Fill out your copyright notice in the Description page of Project Settings.


#include "CruelMomentum.h"

UCruelMomentum::UCruelMomentum()
{
	UpgradeName = "Cruel Momentum";
	UpgradeText =
		"Consecutive Hits grant a 10% boost in damage up to a 4X multiplier. Not Hitting any enemy for 5 seconds or taking damage resets the multiplier.";
	IsUpgradeActive = true;
}

void UCruelMomentum::Init()
{
	UAmbrosiaHealthComponent* PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass()));
	PlayerHealthComp->OnDamaged.AddDynamic(this, &UCruelMomentum::OnReset);
	TArray<UMeleeAttackerComponent*> Components;
	PlayerCharacter->GetComponents<UMeleeAttackerComponent>(Components);
	for (UMeleeAttackerComponent* Component : Components)
	{
		Component->OnMeleeAttackHit.AddDynamic(this, &UCruelMomentum::OnEnemyHit);
	}
	URangedAttackerComponent* PlayerRangedAttacker = Cast<URangedAttackerComponent>(PlayerCharacter->GetComponentByClass(URangedAttackerComponent::StaticClass()));
	PlayerRangedAttacker->OnRangedAttackHit.AddDynamic(this, &UCruelMomentum::OnEnemyHit);
}

void UCruelMomentum::DeInit()
{
	if (!PlayerCharacter) return;

	if (UAmbrosiaHealthComponent* PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass())))
	{
		PlayerHealthComp->OnDamaged.RemoveDynamic(this, &UCruelMomentum::OnReset);
	}

	TArray<UMeleeAttackerComponent*> Components;
	PlayerCharacter->GetComponents<UMeleeAttackerComponent>(Components);
	for (UMeleeAttackerComponent* Component : Components)
	{
		if (Component)
		{
			Component->OnMeleeAttackHit.RemoveDynamic(this, &UCruelMomentum::OnEnemyHit);
		}
	}

	if (URangedAttackerComponent* PlayerRangedAttacker = Cast<URangedAttackerComponent>(PlayerCharacter->GetComponentByClass(URangedAttackerComponent::StaticClass())))
	{
		PlayerRangedAttacker->OnRangedAttackHit.RemoveDynamic(this, &UCruelMomentum::OnEnemyHit);
	}
}

void UCruelMomentum::OnEnemyHit(FHittableHitData Data, bool WasLethal)
{
	if (HitStreak == 0)
	{
		HitStreak++; //technically can be a problem if MaxStreakBonus is 0, but then this upgrade would do nothing
		RecalculateModifier();
		LastKillTimeStamp = FPlatformTime::Seconds();
	} else
	{
		double Now = FPlatformTime::Seconds();
		if (Now - LastKillTimeStamp < MaxTimeBetweenKills)
		{
			LastKillTimeStamp = Now;
			HitStreak = FMath::Min(HitStreak + 1, MaxStreakBonus);
			RecalculateModifier();
				
		} else
		{
			OnReset();
		}
	}
}

void UCruelMomentum::OnReset()
{
	HitStreak = 0;
	RecalculateModifier();
}

void UCruelMomentum::RecalculateModifier()
{
	Modifier.MeleeAttackMultiplier = 1 + (HitStreak * BonusPerHit);
	UpgradesComponent->RecalculateModifier();
}
