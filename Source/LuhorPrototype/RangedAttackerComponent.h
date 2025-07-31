// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackData.h"
#include "BaseAttackerComponent.h"
#include "HittableComponent.h"
#include "RangedAttackerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API URangedAttackerComponent : public UBaseAttackerComponent, public IComponentDependencies
{
	GENERATED_BODY()
	COMPDEP_DECL()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRangedAttackCancelled);
	UPROPERTY(BlueprintAssignable) FOnRangedAttackCancelled OnRangedAttackCancelled;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRangedAttackHit, FHittableHitData, Data);
	UPROPERTY(BlueprintAssignable) FOnRangedAttackHit OnRangedAttackHit;

	UFUNCTION(BlueprintCallable)
	bool TryAttack();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MainSkeletalMeshComponentTag{ "main_skeletal_mesh" };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<URangedAttack> RangedAttack;

	UPROPERTY() TObjectPtr<USkeletalMeshComponent> MainSkeletalMesh{};
	UPROPERTY() TObjectPtr<ULuhorMovementComponent> MovementComponent{};

	virtual void BeginPlay() override;

	void DoWindup();
	void DoContact();
	void DoRecovery();
	void EndAttack();

	void SpawnProjectile();
};
