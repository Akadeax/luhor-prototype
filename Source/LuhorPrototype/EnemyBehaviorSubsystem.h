// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyBehaviorSubsystem.generated.h"

class ALuhorEnemyCharacter;

UCLASS()
class LUHORPROTOTYPE_API UEnemyBehaviorSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	UFUNCTION(BlueprintPure)
	bool AnyOngoingEnemyMeleeAttacks() { return OngoingEnemyMeleeAttacks != 0; }
	
	UFUNCTION(BlueprintPure)
	int GetOngoingEnemyMeleeAttacksCount() { return OngoingEnemyMeleeAttacks; }

	UFUNCTION(BlueprintPure)
	int GetSpawnedEnemiesCount() { return Enemies.Num(); }

private:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void RegisterInitialEnemies();
	
	void OnActorSpawned(AActor* SpawnedActor);
	void OnActorDestroyed(AActor* DestroyedActor);

	void AddEnemyTracking(ALuhorEnemyCharacter* Enemy);
	void RemoveEnemyTracking(ALuhorEnemyCharacter* Enemy);

	UFUNCTION() void IncrementOngoingMeleeAttacks();
	UFUNCTION() void DecrementOngoingMeleeAttacks();

	TArray<ALuhorEnemyCharacter*> Enemies;

	int OngoingEnemyMeleeAttacks{ 0 };
};
