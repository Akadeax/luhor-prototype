// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDamaged);
	UPROPERTY(BlueprintAssignable) FOnDamaged OnDamaged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealed);
	UPROPERTY(BlueprintAssignable) FOnHealed OnHealed;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
	UPROPERTY(BlueprintAssignable) FOnDeath OnDeath;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BLueprintCallable)
	void Damage(float Amount);

	UFUNCTION(BLueprintCallable)
	void Heal(float Amount);

	UFUNCTION(BLueprintCallable)
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BLueprintCallable)
	float GetCurrentHealth() const { return CurrentHealth; }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth{ 100.f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool DestroyOnDeath{ true };

	float CurrentHealth{ 0.f };
};
