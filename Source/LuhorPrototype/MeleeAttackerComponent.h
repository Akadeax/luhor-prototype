// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeAttackChain.h"
#include "Components/ActorComponent.h"
#include "MeleeAttackerComponent.generated.h"

class UCapsuleComponent;

UENUM(BlueprintType)
enum class EMeleeAttackState : uint8
{
	None, Windup, Contact, Recovery
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UMeleeAttackerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMeleeAttackerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeAttackStarted);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackStarted OnMeleeAttackStarted;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeleeAttackDone);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackDone OnMeleeAttackDone;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeAttackStateChanged, EMeleeAttackState, NewState);
	UPROPERTY(BlueprintAssignable) FOnMeleeAttackStateChanged OnMeleeAttackStateChanged;
	
	UFUNCTION(BlueprintCallable)
	bool TryAttack();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MainSkeletalMeshComponentTag{ "main_skeletal_mesh" };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMeleeAttackChain> MeleeAttackChain;

	
	UPROPERTY() TObjectPtr<USkeletalMeshComponent> MainSkeletalMesh{};
	UPROPERTY() TObjectPtr<UShapeComponent> ContactCollision{};

	EMeleeAttackState CurrentAttackState{ EMeleeAttackState::None };
	FTimerHandle CurrentAttackStateTimer;

	int CurrentChainComboIndex{ 0 };
	
	virtual void BeginPlay() override;

	void DoWindup();
	void DoContact();
	void DoRecovery();
	void EndAttack();
	
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

	void SetMeleeAttackState(EMeleeAttackState NewState);
	const FMeleeAttackData& GetCurrentAttack() const;
	
	// Calculate a play rate that makes an `originalTime` second section take `desiredTime` seconds 
	float ConvertPlayRate(float OriginalTime, float DesiredTime) const;
	float GetSectionPlayRate(const UAnimMontage* Montage, FName SectionName, float DesiredTime) const;
};
