// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUpgrade.h"
#include "Tickable.h"
#include "LuhorPrototype/HittableComponent.h"
#include "GhostUpgrade.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UGhostUpgrade : public UBaseUpgrade
{
	GENERATED_BODY()

	virtual void Tick(float DeltaTime) override;

	
	UGhostUpgrade();
	virtual void Init() override;
	virtual void DeInit() override;

	UFUNCTION()
	void OnDashEnded();
	UFUNCTION()
	void OnDashStarted();
	void OnTimerEnded();

	UPROPERTY(EditAnywhere)
	float MaxTimerLength{2.f};
	float CurrentTimerLength{0};
	UHittableComponent* PlayerHittableComp{nullptr};
	
};
