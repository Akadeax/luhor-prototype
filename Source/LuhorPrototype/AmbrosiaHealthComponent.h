// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HealthComponent.h"
#include "AmbrosiaHealthComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UAmbrosiaHealthComponent final : public UHealthComponent
{
	GENERATED_BODY()
public:
	UAmbrosiaHealthComponent()
	{
		PrimaryComponentTick.bCanEverTick = true;
	};
	virtual void BeginPlay() override;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLastStandStarted);
	UPROPERTY(BlueprintAssignable) FOnLastStandStarted OnLastStandStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLastStandEnded);
	UPROPERTY(BlueprintAssignable) FOnLastStandStarted OnLastStandEnded;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void Damage (float Amount) override;

	UFUNCTION(BlueprintCallable)
	void SiphonAmbrosia(float Amount);

	UFUNCTION(BlueprintCallable)
	bool TrySpendSpecialCharge();
	UFUNCTION(BlueprintCallable)
	void AddSpecialChargeSlots(const int Amount = 1){MaxPoisonedAmbrosia+= PoisonedAmbrosiaPerCharge*Amount;}

	UFUNCTION(BlueprintCallable)
	float GetCurrentPoisonedAmbrosiaPercentage() const {return CurrentPoisonedAmbrosia/MaxPoisonedAmbrosia;}
	UFUNCTION(BlueprintCallable)
	int GetSpecialAttackCharges() const {return FMath::TruncToInt(CurrentPoisonedAmbrosia/PoisonedAmbrosiaPerCharge);}
	
	protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ambrosia|Siphon")
	float SiphonPercentage{5.f};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ambrosia|Special Attack")
	float PoisonedAmbrosiaPerCharge{ 10.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ambrosia|Special Attack")
	float MaxPoisonedAmbrosia{PoisonedAmbrosiaPerCharge};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ambrosia|Special Attack")
	float CurrentPoisonedAmbrosia{0.f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Ambrosia|Special Attack", meta = (ToolTip = "The percentage of ambrosia the player needs to have before putting a part of its ambrosia gain towards filling the special attack meter"))
	float SpecialChargeCutoff{0.66f};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Ambrosia")
	float PassiveAmbrosiaDrain{1.25f};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Ambrosia")
	float StartingAmbrosia{50};
	

	bool InLastStand{false};
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Ambrosia" , meta = (ToolTip = "How much ambrosia the player needs to gather to get out of the last stand mode"))
	float LastStandEndCutoff{5};
};
