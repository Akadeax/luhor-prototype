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

	UCruelMomentum();

	virtual void Init() override;
	virtual void DeInit() override;


	int HitStreak{0};
	double LastKillTimeStamp{};
	UPROPERTY(EditAnywhere)
	float MaxTimeBetweenKills{5.f};
	UPROPERTY(EditAnywhere)
	int MaxStreakBonus{40};
	UPROPERTY(EditAnywhere)
	float BonusPerHit{0.1f};
	UFUNCTION()
	void OnEnemyHit(FHittableHitData Data, bool WasLethal, UHittableComponent* Hittable);
	UFUNCTION()
	void OnReset();

	void RecalculateModifier();
};
