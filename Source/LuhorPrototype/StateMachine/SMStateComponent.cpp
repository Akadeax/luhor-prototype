// Fill out your copyright notice in the Description page of Project Settings.

#include "SMStateComponent.h"

#include "StateMachineComponent.h"

USMStateComponent::USMStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USMStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsStateActive) OnUpdateState();
}

void USMStateComponent::EnterState()
{
	OnEnterState();
}

void USMStateComponent::ExitState()
{
	OnExitState();
}

UStateMachineComponent* USMStateComponent::GetStateMachine() const
{
	return Cast<UStateMachineComponent>(GetOwner());
}

void USMStateComponent::OnEnterState_Implementation()
{
}

void USMStateComponent::OnUpdateState_Implementation()
{
}

void USMStateComponent::OnExitState_Implementation()
{
}
