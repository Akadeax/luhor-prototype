// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthComponent.h"

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UHealthComponent::Damage(float Amount)
{
	CurrentHealth -= Amount;
	OnDamaged.Broadcast();
	
	if (CurrentHealth <= 0)
	{
		CurrentHealth = 0;
		OnDeath.Broadcast();
		if (DestroyOnDeath) GetOwner()->Destroy();
	}
}

void UHealthComponent::Heal(float Amount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0, MaxHealth);
	OnHealed.Broadcast();
}

