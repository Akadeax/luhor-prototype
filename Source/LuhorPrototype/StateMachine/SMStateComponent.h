// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SMStateComponent.generated.h"

class UStateMachineComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class LUHORPROTOTYPE_API USMStateComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USMStateComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EnterState();
	void ExitState();
	
protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnEnterState();
	UFUNCTION(BlueprintNativeEvent)
	void OnUpdateState();
	UFUNCTION(BlueprintNativeEvent)
	void OnExitState();

	UPROPERTY(BlueprintReadOnly)
	bool IsStateActive{ false };

	UFUNCTION(BlueprintPure)
	UStateMachineComponent* GetStateMachine() const;
};
