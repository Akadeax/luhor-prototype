// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAttackerComponent.h"


UMeleeAttackerComponent::UMeleeAttackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UMeleeAttackerComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UMeleeAttackerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

