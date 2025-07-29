// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBehaviorSubsystem.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "LuhorEnemyCharacter.h"
#include "LuhorPlayerCharacter.h"
#include "MeleeAttackerComponent.h"
#include "Algo/Count.h"
#include "BehaviorTree/BlackboardComponent.h"

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

	world->OnWorldBeginPlay.AddUObject(this, &ThisClass::RegisterInitial);
}

void UEnemyBehaviorSubsystem::RegisterInitial()
{
	for (TActorIterator<ALuhorEnemyCharacter> it{ GetWorld() }; it; ++it)
	{
		AddEnemyTracking(*it);
	}

	for (TActorIterator<ALuhorPlayerCharacter> it{ GetWorld() }; it; ++it)
	{
		Player = *it;
	}
	
	UpdateEngagedEnemies();
}

void UEnemyBehaviorSubsystem::OnActorSpawned(AActor* SpawnedActor)
{
	if (ALuhorEnemyCharacter* enemy{ Cast<ALuhorEnemyCharacter>(SpawnedActor) }) AddEnemyTracking(enemy);
	if (ALuhorPlayerCharacter* player{ Cast<ALuhorPlayerCharacter>(SpawnedActor) }) Player = player;
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
	UpdateEngagedEnemies();
}

void UEnemyBehaviorSubsystem::UpdateEngagedEnemies()
{
	TArray<ALuhorEnemyCharacter*> unengagedEnemies;
	unengagedEnemies.Reserve(Enemies.Num());

	int engagedCount{ 0 };
	
	for (ALuhorEnemyCharacter* inner : Enemies)
	{
		const UBlackboardComponent* blackboard{ GetBlackboard(inner) };
		const bool engaged{ blackboard->GetValueAsBool("Engaged") };

		if (engaged) continue;

		++engagedCount;
		unengagedEnemies.Add(inner);
	}

	int amountToEngage{ FMath::Clamp(DesiredEngagedCount - engagedCount, 0, DesiredEngagedCount) };

	unengagedEnemies.Sort([this](const ALuhorEnemyCharacter& a, const ALuhorEnemyCharacter& b)
	{
		const double aDist{ FVector::DistSquared(a.GetActorLocation(), Player->GetActorLocation()) };
		const double bDist{ FVector::DistSquared(b.GetActorLocation(), Player->GetActorLocation()) };
		return aDist < bDist;
	});

	for (const ALuhorEnemyCharacter* unengaged : unengagedEnemies)
	{
		DrawDebugString(GetWorld(), unengaged->GetActorLocation(), TEXT("Test"), nullptr, FColor::White, 1.f);
	}

}

UBlackboardComponent* UEnemyBehaviorSubsystem::GetBlackboard(const ACharacter* Char)
{
	return Cast<AAIController>(Char->GetController())->GetBlackboardComponent();
}

void UEnemyBehaviorSubsystem::IncrementOngoingMeleeAttacks()
{
	++OngoingEnemyMeleeAttacks;
}

void UEnemyBehaviorSubsystem::DecrementOngoingMeleeAttacks()
{
	--OngoingEnemyMeleeAttacks;
}