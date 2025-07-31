// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttackData.h"
#include "BaseAttackerComponent.h"
#include "HittableComponent.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UMeleeAttackerComponent : public UBaseAttackerComponent, public IComponentDependencies
{
	GENERATED_BODY()
	COMPDEP_DECL()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeAttackChainDone);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackChainDone OnMeleeAttackChainDone;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeAttackCancelled);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackCancelled OnMeleeAttackCancelled;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeAttackHit, FHittableHitData, Data);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackHit OnMeleeAttackHit;

	
	UFUNCTION(BlueprintCallable)
	bool TryAttack();

	UFUNCTION(BlueprintCallable)
	bool IsAttackQueued() const { return AttackQueued; }

	UFUNCTION(BlueprintCallable)
	void CancelAttack();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MainSkeletalMeshComponentTag{ "main_skeletal_mesh" };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMeleeAttackChain> MeleeAttackChain;

	UPROPERTY() TObjectPtr<USkeletalMeshComponent> MainSkeletalMesh{};
	UPROPERTY() TObjectPtr<ULuhorMovementComponent> MovementComponent{};
	UPROPERTY() TObjectPtr<UShapeComponent> ContactCollision{};

	int CurrentChainIndex{ 0 };
	FTimerHandle ChainLeniencyTimer;

	bool AttackQueued{ false };
	
	virtual void BeginPlay() override;

	void DoWindup();
	void DoContact();
	void DoRecovery();
	void EndAttack();
	void EndChainLeniency();
	
	void EnableContactCollision();
	void DisableContactCollision();
	
	UFUNCTION()
	void OnContactCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	const FMeleeAttackData& GetCurrentAttack() const;
};
