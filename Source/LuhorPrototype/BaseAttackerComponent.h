// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BaseAttackerComponent.generated.h"

class UFactionAssociation;

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	None, Windup, Contact, Recovery
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UBaseAttackerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackStarted);
	UPROPERTY(BlueprintAssignable) FOnAttackStarted OnAttackStarted;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackDone);
	UPROPERTY(BlueprintAssignable) FOnAttackDone OnAttackDone;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackStateChanged, EAttackState, NewState);
	UPROPERTY(BlueprintAssignable) FOnAttackStateChanged OnAttackStateChanged;

	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UFactionAssociation* Faction{};

	EAttackState CurrentAttackState{ EAttackState::None };
	FTimerHandle CurrentAttackStateTimer;

	void SetAttackState(EAttackState NewState);

	// Calculate a play rate that makes an `originalTime` seconds long section take `desiredTime` seconds 
	static float ConvertPlayRate(float OriginalTime, float DesiredTime);
	static float GetSectionPlayRate(const UAnimMontage* Montage, FName SectionName, float DesiredTime);
};
