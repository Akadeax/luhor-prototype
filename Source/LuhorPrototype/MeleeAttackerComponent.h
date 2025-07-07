// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackerComponent.generated.h"

UENUM(BlueprintType)
enum class EMeleeAttackState : uint8
{
	None, Windup, Contact, Recovery
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UMeleeAttackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

public:
};
