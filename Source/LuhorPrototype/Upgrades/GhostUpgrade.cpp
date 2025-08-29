// Fill out your copyright notice in the Description page of Project Settings.


#include "GhostUpgrade.h"

#include "LuhorPrototype/MeleeAttackerComponent.h"

void UGhostUpgrade::Tick(float DeltaTime)
{
	if (!IsUpgradeActive) return;
	if ( CurrentTimerLength <= 0)
	{
		OnTimerEnded();
	} else
	{
		CurrentTimerLength -= DeltaTime;
	}
}

UGhostUpgrade::UGhostUpgrade()
{
	UpgradeName = "Ghost";
	UpgradeText = "Your dash's cooldown increases by 200%; it goes farther, and during it you are invincible. Dashing doubles your melee attack damage for 2 seconds.";
	Modifier.MeleeAttackMultiplier = 2;
	IsUpgradeActive = false;
	
}

void UGhostUpgrade::OnDashEnded()
{
	IsUpgradeActive = true;
	CurrentTimerLength = MaxTimerLength;
	PlayerHittableComp->MakeVulnerable();
	UpgradesComponent->RecalculateModifier();
	OnEmpowerStart();
}

void UGhostUpgrade::OnDashStarted()
{
	PlayerHittableComp->MakeInvulnerable(50,MakeInvulnerableMode::SetTimeIfLonger);

}

void UGhostUpgrade::OnTimerEnded()
{
	IsUpgradeActive = false;
	UpgradesComponent->RecalculateModifier();
	OnEmpowerEnd();
}

void UGhostUpgrade::Init()
{
	Super::Init();
	PlayerHittableComp = Cast<UHittableComponent>(PlayerCharacter->GetComponentByClass(UHittableComponent::StaticClass()));
	UMeleeAttackerComponent* temp = Cast<UMeleeAttackerComponent>(PlayerCharacter->GetComponentsByTag(UMeleeAttackerComponent::StaticClass(), "Dash")[0]);
	temp->OnMeleeAttackChainDone.AddDynamic(this, &UGhostUpgrade::OnDashEnded);
	temp->OnAttackStarted.AddDynamic(this, &UGhostUpgrade::OnDashStarted);
}

void UGhostUpgrade::DeInit()
{
	Super::DeInit();
	UMeleeAttackerComponent* dashAttacker = Cast<UMeleeAttackerComponent>(PlayerCharacter->GetComponentsByTag(UMeleeAttackerComponent::StaticClass(), "Dash")[0]);
	dashAttacker->OnMeleeAttackChainDone.RemoveDynamic(this, &UGhostUpgrade::OnDashEnded);
}

