// Fill out your copyright notice in the Description page of Project Settings.

#include "LuhorMovementComponent.h"

void ULuhorMovementComponent::DoCurvedLaunch(FVector Direction, FCurvedLaunchData Data)
{
	CurrentDirection = Direction;
	CurrentDirection.Normalize();
	
	CurrentLaunchData = Data;
	
	CurrentLaunchTimeLeft = Data.Time;

	OnLaunchStart.Broadcast(CurrentLaunchData);
}

void ULuhorMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick)
{
	Super::TickComponent(DeltaTime, TickType, Tick);

	if (CurrentLaunchTimeLeft <= 0.f) return;

	CurrentLaunchTimeLeft -= DeltaTime;

	// Inversion so time goes from 0 to 1
	const float currentTimeProgress01{ 1 - (CurrentLaunchTimeLeft / CurrentLaunchData.Time) };

	const float curveFactor{
		CurrentLaunchData.ForceMultiplierCurve ?
		CurrentLaunchData.ForceMultiplierCurve->GetFloatValue(currentTimeProgress01) :
		1.f - currentTimeProgress01 // Budget linear curve; inverted so force is back to 1 to 0
	};
	
	const float currentForce{
		curveFactor *
		CurrentLaunchData.Force *
		STATIC_LAUNCH_FORCE_MULTIPLIER
	};

	AddForce(currentForce * CurrentDirection);

	if (CurrentLaunchTimeLeft <= 0.f)
	{
		OnLaunchEnd.Broadcast(CurrentLaunchData);
	}
}
