// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies.h"
#include "SMStateComponent.h"
#include "Components/SceneComponent.h"
#include "StateMachineComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class LUHORPROTOTYPE_API UStateMachineComponent : public USceneComponent, public IComponentDependencies
{
	GENERATED_BODY()
	COMPDEP_DECL()
	
public:
	UStateMachineComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ChangeState(FName StateName);
	USMStateComponent* GetCurrentState();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName StartingStateName{ NAME_None };
	
	UPROPERTY() TMap<FName, USMStateComponent*> States{};
	FName CurrentStateName{};
};
