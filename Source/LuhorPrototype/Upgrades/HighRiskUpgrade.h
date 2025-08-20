// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUpgrade.h"
#include "LuhorPrototype/AmbrosiaHealthComponent.h"
#include "HighRiskUpgrade.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UHighRiskUpgrade : public UBaseUpgrade
{
	GENERATED_BODY()
	
	UHighRiskUpgrade();

	virtual void Init() override;
	virtual void DeInit() override;
	UPROPERTY()
	UAmbrosiaHealthComponent* PlayerHealthComp{nullptr};
	UPROPERTY(EditDefaultsOnly)
	float HealthActivationCutoff{0.5f};
	UFUNCTION()
	void OnHealthChanged();
};

