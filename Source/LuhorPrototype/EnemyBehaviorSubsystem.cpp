// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBehaviorSubsystem.h"

#include "EngineUtils.h"
#include "LuhorEnemyCharacter.h"
#include "MeleeAttackerComponent.h"

bool UEnemyBehaviorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}
	
	const UWorld* world{ Cast<UWorld>(Outer) };
	if (!world) return false;

	return world->IsGameWorld();
}

void UEnemyBehaviorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorSpawned)
	);
	GetWorld()->AddOnActorDestroyedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorDestroyed)
	);

	UWorld* world{ GetWorld() };
	check(world);

	world->OnWorldBeginPlay.AddUObject(this, &ThisClass::RegisterInitialEnemies);
}

void UEnemyBehaviorSubsystem::RegisterInitialEnemies()
{
	for (TActorIterator<ALuhorEnemyCharacter> it{ GetWorld() }; it; ++it)
	{
		AddEnemyTracking(*it);
	}
}

void UEnemyBehaviorSubsystem::OnActorSpawned(AActor* SpawnedActor)
{
	if (ALuhorEnemyCharacter* enemy{ Cast<ALuhorEnemyCharacter>(SpawnedActor) }) AddEnemyTracking(enemy);
}

void UEnemyBehaviorSubsystem::OnActorDestroyed(AActor* DestroyedActor)
{
	if (ALuhorEnemyCharacter* enemy{ Cast<ALuhorEnemyCharacter>(DestroyedActor) }) RemoveEnemyTracking(enemy);
}

void UEnemyBehaviorSubsystem::AddEnemyTracking(ALuhorEnemyCharacter* Enemy)
{
	Enemies.Add(Enemy);

	TArray<UMeleeAttackerComponent*> attackers;
	Enemy->GetComponents<UMeleeAttackerComponent>(attackers);
	for (UMeleeAttackerComponent* attacker : attackers)
	{
		attacker->OnMeleeAttackStarted.AddDynamic(this, &ThisClass::IncrementOngoingMeleeAttacks);
		attacker->OnMeleeAttackDone.AddDynamic(this, &ThisClass::DecrementOngoingMeleeAttacks);
		attacker->OnMeleeAttackCancelled.AddDynamic(this, &ThisClass::DecrementOngoingMeleeAttacks);
	}
}

void UEnemyBehaviorSubsystem::RemoveEnemyTracking(ALuhorEnemyCharacter* Enemy)
{
	Enemies.Remove(Enemy);
}

void UEnemyBehaviorSubsystem::IncrementOngoingMeleeAttacks()
{
	++OngoingEnemyMeleeAttacks;
}

void UEnemyBehaviorSubsystem::DecrementOngoingMeleeAttacks()
{
	--OngoingEnemyMeleeAttacks;
}