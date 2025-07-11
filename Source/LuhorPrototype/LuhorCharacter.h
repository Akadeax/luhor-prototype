// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LuhorMovementComponent.h"
#include "GameFramework/Character.h"
#include "LuhorCharacter.generated.h"

UCLASS()
class LUHORPROTOTYPE_API ALuhorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	explicit ALuhorCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULuhorMovementComponent> LuhorCharacterMovement;
};
