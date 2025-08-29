// Fill out your copyright notice in the Description page of Project Settings.


#include "LuhorPrototype/Upgrades/AssassinUpgrade.h"

#include "LuhorPrototype/AmbrosiaHealthComponent.h"
#include "LuhorPrototype/MeleeAttackerComponent.h"


void UAssassinUpgrade::Init()
{
	DashAttacker = Cast<UMeleeAttackerComponent>(PlayerCharacter->FindComponentByTag(UMeleeAttackerComponent::StaticClass(), "Dash"));
	DashAttacker->OnMeleeAttackHit.AddDynamic(this, &ThisClass::OnDashAttackHit);

	Health = PlayerCharacter->FindComponentByClass<UAmbrosiaHealthComponent>();

	OldSiphonPercent = Health->GetSiphonPercentage();
	Health->SetSiphonPercentage(UpgradedSiphonPercent);
	
	OldPoisonSiphonPercent = Health->GetPoisonSiphonPercent();
	Health->SetPoisonSiphonPercent(UpgradedPoisonSiphonPercent);
}


void UAssassinUpgrade::DeInit()
{
	DashAttacker->OnMeleeAttackHit.RemoveDynamic(this, &ThisClass::OnDashAttackHit);
	Health->SetPoisonSiphonPercent(OldPoisonSiphonPercent);
	Health->SetSiphonPercentage(OldSiphonPercent);
}


void UAssassinUpgrade::OnDashAttackHit(FHittableHitData Data, bool WasLethal, UHittableComponent* Hittable)
{
	Hittable->SetMarked();
}
