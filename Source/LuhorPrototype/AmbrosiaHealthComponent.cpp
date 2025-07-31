// Fill out your copyright notice in the Description page of Project Settings.


#include "AmbrosiaHealthComponent.h"


void UAmbrosiaHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = StartingAmbrosia;
}

void UAmbrosiaHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	if (!InLastStand)
	{
		CurrentHealth -= DeltaTime * PassiveAmbrosiaDrain;
		OnDamaged.Broadcast();
		if (CurrentHealth <= 0)
		{
			CurrentHealth = 0;
			InLastStand = true;
			OnLastStandStarted.Broadcast();
		}
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAmbrosiaHealthComponent::Damage(float Amount)
{
	if (Amount < 0) return;
	OnDamaged.Broadcast();
	if (InLastStand)
	{
		OnDeath.Broadcast();
		if (DestroyOnDeath) GetOwner()->Destroy();
	} else
	{
		CurrentHealth -= Amount;
		if (CurrentHealth <= 0)
		{
			CurrentHealth = 0;
			InLastStand = true;
			OnLastStandStarted.Broadcast();
		};
	}
}

void UAmbrosiaHealthComponent::SiphonAmbrosia(float Amount)
{
	float SiphonAmount = Amount * (SiphonPercentage/100);
	if (CurrentHealth / MaxHealth > SpecialChargeCutoff)
	{
		float HalfSiphon = SiphonAmount / 2;
		float HealthRemainder = MaxHealth-CurrentHealth;
		float PoisonedAmbrosiaRemainder = MaxPoisonedAmbrosia - CurrentPoisonedAmbrosia;
		if (HalfSiphon > HealthRemainder)
		{
			int Remainder = HalfSiphon - HealthRemainder;
			CurrentHealth = MaxHealth;
			CurrentPoisonedAmbrosia = FMath::Min(CurrentPoisonedAmbrosia + Remainder + HalfSiphon , MaxPoisonedAmbrosia);
		} else if (HalfSiphon > PoisonedAmbrosiaRemainder)
		{
			int Remainder = HalfSiphon - PoisonedAmbrosiaRemainder;
			CurrentPoisonedAmbrosia = MaxPoisonedAmbrosia;
			CurrentHealth = FMath::Min(CurrentHealth + Remainder + HalfSiphon , MaxHealth);
		} else
		{
			CurrentHealth += HalfSiphon;
			CurrentPoisonedAmbrosia += HalfSiphon;
		}
	} else
	{
		CurrentHealth = FMath::Min(CurrentHealth + SiphonAmount, MaxHealth);
		if (InLastStand && CurrentHealth > LastStandEndCutoff)
		{
			InLastStand = false;
			OnLastStandEnded.Broadcast();
		}
	}
}

bool UAmbrosiaHealthComponent::TrySpendSpecialCharge()
{
	if ( CurrentPoisonedAmbrosia >= PoisonedAmbrosiaPerCharge)
	{
		CurrentPoisonedAmbrosia-= PoisonedAmbrosiaPerCharge;
		return true;
	}
	return false;
}
