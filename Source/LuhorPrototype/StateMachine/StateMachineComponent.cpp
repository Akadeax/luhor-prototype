// Fill out your copyright notice in the Description page of Project Settings.

#include "StateMachineComponent.h"

#include "SMStateComponent.h"

COMPDEP_IMPL_START(UStateMachineComponent)
	COMPDEP_DEP_ChildRequired(USMStateComponent)
COMPDEP_IMPL_END

UStateMachineComponent::UStateMachineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStateMachineComponent::BeginPlay()
{
	Super::BeginPlay();

	TArray<USceneComponent*> allChildren;
	GetChildrenComponents(true, allChildren);

	for (USceneComponent* child : allChildren)
	{
		if (child->IsA<USMStateComponent>())
		{
			States.Add(child->GetFName(), Cast<USMStateComponent>(child));
		}
	}

	ChangeState(StartingStateName);	
}

void UStateMachineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStateMachineComponent::ChangeState(FName StateName)
{
	checkf(States.Contains(StateName), TEXT("Trying to enter invalid state!"));
	
	if (CurrentStateName != NAME_None)
	{
		GetCurrentState()->ExitState();
	}
	CurrentStateName = StateName;
	GetCurrentState()->EnterState();
}

USMStateComponent* UStateMachineComponent::GetCurrentState()
{
	return States[CurrentStateName];
}

