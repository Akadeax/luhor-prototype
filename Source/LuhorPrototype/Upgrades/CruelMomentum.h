// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUpgrade.h"
#include "LuhorPrototype/AmbrosiaHealthComponent.h"
#include "LuhorPrototype/HittableComponent.h"
#include "LuhorPrototype/MeleeAttackerComponent.h"
#include "LuhorPrototype/RangedAttackerComponent.h"
#include "CruelMomentum.generated.h"


UCLASS()
class LUHORPROTOTYPE_API UCruelMomentum : public UBaseUpgrade
{
	GENERATED_BODY()

	UCruelMomentum()
	{
		UpgradeName = "Cruel Momentum";
		UpgradeText = "Consecutive Kills grant a 50% boost in damage up to a 4X multiplier. Not killing any enemy for 10 seconds or taking damage resets the multiplier.";
		IsUpgradeActive = true;
	}
	virtual void Init() override
	{
		UAmbrosiaHealthComponent* PlayerHealthComp = Cast<UAmbrosiaHealthComponent>(PlayerCharacter->GetComponentByClass(UAmbrosiaHealthComponent::StaticClass()));
		PlayerHealthComp->OnDamaged.AddDynamic(this, &UCruelMomentum::OnReset);
		UMeleeAttackerComponent* PlayerMeleeAttacker = Cast<UMeleeAttackerComponent>(PlayerCharacter->GetComponentByClass(UMeleeAttackerComponent::StaticClass()));
		//PlayerMeleeAttacker->OnMeleeAttackHit.AddDynamic(this, &UCruelMomentum::OnEnemyHit);
		URangedAttackerComponent* PlayerRangedAttacker = Cast<URangedAttackerComponent>(PlayerCharacter->GetComponentByClass(URangedAttackerComponent::StaticClass()));
		//PlayerRangedAttacker->OnRangedAttackHit.AddDynamic(this, &UCruelMomentum::OnEnemyHit);
	}
	
	
	int KillStreak{0};
	double LastKillTimeStamp{};
	UPROPERTY(EditAnywhere)
	float MaxTimeBetweenKills{10.f};
	UPROPERTY(EditAnywhere)
	int MaxStreakBonus;
	UPROPERTY(EditAnywhere)
	float BonusPerKill{0.5f};
	UFUNCTION()
	void OnEnemyHit(FHittableHitData Data, bool WasLethal)
	{
		if (WasLethal)
		{
			if (KillStreak == 0)
			{
				KillStreak++; //technically can be a problem if MaxStreakBonus is 0, but then this upgrade would do nothing
				RecalculateModifier();
			} else
			{
				double Now = FPlatformTime::Seconds();
				if (Now - LastKillTimeStamp < MaxTimeBetweenKills)
				{
					LastKillTimeStamp = Now;
					KillStreak = FMath::Min(KillStreak +1, MaxStreakBonus);
					RecalculateModifier();
					
				} else
				{
					OnReset();
				}
			}
		}
	}
	UFUNCTION()
	void OnReset()
	{
		KillStreak = 0;
		RecalculateModifier();
	}
	void RecalculateModifier()
	{
		Modifier.MeleeAttackMultiplier = 1 + (KillStreak * BonusPerKill);
		UE_LOG(LogTemp, Warning, TEXT("KillStreak: %d"), KillStreak);
		UpgradesComponent->RecalculateModifier();
	}
};
