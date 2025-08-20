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
	TargetHealth = StartingAmbrosia;
	UpgradesComponent = Cast<UUpgradesComponent>(GetOwner()->GetComponentByClass(UUpgradesComponent::StaticClass()));
	FDebugUtil::QuitCheckf(UpgradesComponent, TEXT("Couldn't find the UpgradesComponent"));
	MaxPoisonedAmbrosia = PoisonedAmbrosiaPerCharge * StartingCharges;
	TargetPoisonedAmbrosia = CurrentPoisonedAmbrosia;
	const ULevelGameInstanceSubsystem* sub{ GetWorld()->GetGameInstance()->GetSubsystem<ULevelGameInstanceSubsystem>() };
	check(sub);

	if (sub->PlayerSaveData.Health == -1) return;
	
	CurrentHealth = sub->PlayerSaveData.Health;
	TargetHealth = CurrentHealth;
	CurrentPoisonedAmbrosia = sub->PlayerSaveData.PoisonedAmbrosia;
	TargetPoisonedAmbrosia = CurrentPoisonedAmbrosia;
}

void UAmbrosiaHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!InLastStand && DoDrain)
	{
		TargetHealth -= DeltaTime * PassiveAmbrosiaDrain * UpgradesComponent->GetCurrentModifier().
		                                                                       AmbrosiaDrainMultiplier;
		if (CurrentHealth < 0)
		{
			CurrentHealth = 0;
			InLastStand = true;
			OnLastStandStarted.Broadcast();
		}
	}

	CurrentHealth = FMath::FInterpTo(
		CurrentHealth,
		TargetHealth,
		DeltaTime,
		CurrentHealth >TargetHealth ? AmbrosiaFillSpeed*4 : AmbrosiaFillSpeed //drain much faster than you fill
		);
	CurrentPoisonedAmbrosia = FMath::FInterpTo(CurrentPoisonedAmbrosia, TargetPoisonedAmbrosia, DeltaTime, PoisonedAmbrosiaFillSpeed);
	if (CurrentHealth < 0)
	{
		CurrentHealth = 0;
		TargetHealth = 0;
		InLastStand = true;
		OnLastStandStarted.Broadcast();
		UE_LOG(LogTemp,Log,TEXT("Last stand started"));
	};
}

void UAmbrosiaHealthComponent::Damage(float Amount)
{
	if (Amount < 0) return;
	OnDamaged.Broadcast();
	if (InLastStand)
	{ 
		OnDeath.Broadcast();
		HasDied = true;
		if (DestroyOnDeath) GetOwner()->Destroy();
	}
	else
	{
		float damage = Amount / UpgradesComponent->GetCurrentModifier().DefenseMultiplier;
		TargetHealth -= damage;
	}
}

void UAmbrosiaHealthComponent::Heal(float Amount)
{
	Super::Heal(Amount);
	TargetHealth = CurrentHealth;
}

void UAmbrosiaHealthComponent::SiphonAmbrosia(float Amount)
{
	float SiphonAmount = Amount * (SiphonPercentage / 100) * UpgradesComponent->GetCurrentModifier().
		SiphonSpeedMultiplier;

	if (CurrentHealth / MaxHealth > SpecialChargeCutoff)
	{
		float HalfSiphon = SiphonAmount / 2;
		float HealthRemainder = MaxHealth - TargetHealth;
		float PoisonedAmbrosiaRemainder = MaxPoisonedAmbrosia - TargetPoisonedAmbrosia;
		if (HalfSiphon > HealthRemainder)
		{
			int Remainder = HalfSiphon - HealthRemainder;
			TargetHealth = MaxHealth;
			TargetPoisonedAmbrosia = FMath::Min(CurrentPoisonedAmbrosia + Remainder + HalfSiphon, MaxPoisonedAmbrosia);
		}
		else if (HalfSiphon > PoisonedAmbrosiaRemainder)
		{
			int Remainder = HalfSiphon - PoisonedAmbrosiaRemainder;
			TargetPoisonedAmbrosia = MaxPoisonedAmbrosia;
			TargetHealth = FMath::Min(TargetHealth + Remainder + HalfSiphon, MaxHealth);
		}
		else
		{
			TargetHealth += HalfSiphon;
			TargetPoisonedAmbrosia += HalfSiphon;
		}
	}
	else
	{
		TargetHealth = FMath::Min(TargetHealth + SiphonAmount, MaxHealth);
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
		TargetPoisonedAmbrosia -= PoisonedAmbrosiaPerCharge;
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

void UAmbrosiaHealthComponent::ResetBools()
{
	HasDied = false;
	InLastStand = false;
	DoDrain = false;
}
