// Fill out your copyright notice in the Description page of Project Settings.

#include "LuhorCharacter.h"

#include "LuhorMovementComponent.h"

ALuhorCharacter::ALuhorCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULuhorMovementComponent>(CharacterMovementComponentName))
{
	LuhorCharacterMovement = Cast<ULuhorMovementComponent>(GetCharacterMovement());
}

void ALuhorCharacter::SetControlRotationFromInputDir(FVector InputDir, float OffsetDeg) const
{
	const double yaw{ FMath::Atan2(-InputDir.Y, InputDir.X) };
	const FRotator rot{ 0.f, FMath::RadiansToDegrees(yaw) + OffsetDeg, 0.f };
	GetController()->SetControlRotation(rot);
}
