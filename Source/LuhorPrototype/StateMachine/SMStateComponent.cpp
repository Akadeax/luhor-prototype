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
	IsStateActive = true;
	OnEnterState();
}

void USMStateComponent::ExitState()
{
	IsStateActive = false;
	OnExitState();
}

bool USMStateComponent::CanEnterState_Implementation()
{
	return true;
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

UStateMachineComponent* USMStateComponent::GetStateMachine() const
{
	return Cast<UStateMachineComponent>(GetAttachParent());
}

// Pass-by-const-ref disables blueprint inline syntax
// ReSharper disable once CppPassValueParameterByConstReference
UObject* USMStateComponent::GetChildComponentByClass(TSubclassOf<USceneComponent> Class) const
{
	TArray<USceneComponent*> children;
	GetChildrenComponents(true, children);
	for (USceneComponent* child : children)
	{
		if (child->IsA(Class)) return child;
	}

	return nullptr;
}
