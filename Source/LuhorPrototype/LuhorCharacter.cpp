// Fill out your copyright notice in the Description page of Project Settings.

#include "LuhorCharacter.h"

#include "LuhorMovementComponent.h"

ALuhorCharacter::ALuhorCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULuhorMovementComponent>(CharacterMovementComponentName))
{
	LuhorCharacterMovement = Cast<ULuhorMovementComponent>(GetCharacterMovement());
}
