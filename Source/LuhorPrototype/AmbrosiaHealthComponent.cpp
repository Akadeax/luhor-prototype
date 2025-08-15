// Fill out your copyright notice in the Description page of Project Settings.


#include "AmbrosiaHealthComponent.h"

#include "LevelGameInstanceSubsystem.h"
#include "Upgrades/UpgradesComponent.h"
#include "Util/FDebugUtil.h"

COMPDEP_IMPL_START(UAmbrosiaHealthComponent)
	COMPDEP_DEP_AnyOnActorOptional(UUpgradesComponent)
COMPDEP_IMPL_END

void UAmbrosiaHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = StartingAmbrosia;
	UpgradesComponent = Cast<UUpgradesComponent>(GetOwner()->GetComponentByClass(UUpgradesComponent::StaticClass()));
	FDebugUtil::QuitCheckf(UpgradesComponent, TEXT("Couldn't find the UpgradesComponent"));

	const ULevelGameInstanceSubsystem* sub{ GetWorld()->GetGameInstance()->GetSubsystem<ULevelGameInstanceSubsystem>() };
	check(sub);

	if (sub->PlayerSaveData.Health == -1) return;
	
	CurrentHealth = sub->PlayerSaveData.Health;
	CurrentPoisonedAmbrosia = sub->PlayerSaveData.PoisonedAmbrosia;
}

void UAmbrosiaHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!InLastStand && DoDrain)
	{
		CurrentHealth -= DeltaTime * PassiveAmbrosiaDrain * UpgradesComponent->GetCurrentModifier().
		                                                                       AmbrosiaDrainMultiplier;
		
		if (UUpgradesComponent* Upgrades = Cast<UUpgradesComponent>(
			GetOwner()->GetComponentByClass(UUpgradesComponent::StaticClass())))
		{
			FStatModifier modifier{Upgrades->GetCurrentModifier()};
			CurrentHealth -= DeltaTime * PassiveAmbrosiaDrain * modifier.AmbrosiaDrainMultiplier;
		}

		OnDamaged.Broadcast();
		if (CurrentHealth <= 0)
		{
			CurrentHealth = 0;
			InLastStand = true;
			OnLastStandStarted.Broadcast();
		}
	}
}

void UAmbrosiaHealthComponent::Damage(float Amount)
{
	if (Amount < 0) return;
	OnDamaged.Broadcast();
	if (InLastStand)
	{
		OnDeath.Broadcast();
		if (DestroyOnDeath) GetOwner()->Destroy();
	}
	else
	{
		CurrentHealth -= Amount / UpgradesComponent->GetCurrentModifier().DefenseMultiplier;
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
	float SiphonAmount = Amount * (SiphonPercentage / 100) * UpgradesComponent->GetCurrentModifier().
		SiphonSpeedMultiplier;

	if (CurrentHealth / MaxHealth > SpecialChargeCutoff)
	{
		float HalfSiphon = SiphonAmount / 2;
		float HealthRemainder = MaxHealth - CurrentHealth;
		float PoisonedAmbrosiaRemainder = MaxPoisonedAmbrosia - CurrentPoisonedAmbrosia;
		if (HalfSiphon > HealthRemainder)
		{
			int Remainder = HalfSiphon - HealthRemainder;
			CurrentHealth = MaxHealth;
			CurrentPoisonedAmbrosia = FMath::Min(CurrentPoisonedAmbrosia + Remainder + HalfSiphon, MaxPoisonedAmbrosia);
		}
		else if (HalfSiphon > PoisonedAmbrosiaRemainder)
		{
			int Remainder = HalfSiphon - PoisonedAmbrosiaRemainder;
			CurrentPoisonedAmbrosia = MaxPoisonedAmbrosia;
			CurrentHealth = FMath::Min(CurrentHealth + Remainder + HalfSiphon, MaxHealth);
		}
		else
		{
			CurrentHealth += HalfSiphon;
			CurrentPoisonedAmbrosia += HalfSiphon;
		}
	}
	else
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
	if (CurrentPoisonedAmbrosia >= PoisonedAmbrosiaPerCharge)
	{
		CurrentPoisonedAmbrosia -= PoisonedAmbrosiaPerCharge;
		return true;
	}
	return false;
}

void UAmbrosiaHealthComponent::AddSpecialChargeSlots(const int Amount)
{
	MaxPoisonedAmbrosia += PoisonedAmbrosiaPerCharge * Amount;
}

float UAmbrosiaHealthComponent::GetCurrentPoisonedAmbrosia() const
{
	return CurrentPoisonedAmbrosia;
}

float UAmbrosiaHealthComponent::GetCurrentPoisonedAmbrosiaPercentage() const
{
	return CurrentPoisonedAmbrosia / MaxPoisonedAmbrosia;
}

int UAmbrosiaHealthComponent::GetSpecialAttackCharges() const
{
	return FMath::TruncToInt(CurrentPoisonedAmbrosia / PoisonedAmbrosiaPerCharge);
}
