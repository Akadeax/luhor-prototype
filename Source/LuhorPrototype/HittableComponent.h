// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HittableComponent.generated.h"

USTRUCT(BlueprintType)
struct FHittableHitData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float Damage{};
};

UENUM(BlueprintType)
enum class MakeInvulnerableMode : uint8
{
	IfNotInvulnerableAlready,
	SetTimeIfLonger,
	TimeAdditive,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UHittableComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHittableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, FHittableHitData, Data);
	UPROPERTY(BlueprintAssignable) FOnHit OnHit;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerable);
	UPROPERTY(BlueprintAssignable) FOnInvulnerable OnInvulnerable;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerableEnd);
	UPROPERTY(BlueprintAssignable) FOnInvulnerableEnd OnInvulnerableEnd;


	void Hit(FHittableHitData HitData);

	UFUNCTION(BlueprintCallable)
	void MakeInvulnerable(float Time, MakeInvulnerableMode Mode = MakeInvulnerableMode::SetTimeIfLonger);
	
	UFUNCTION(BlueprintCallable)
	bool IsInvulnerable() const { return CurrentInvulnerabilityTimeLeft > 0.f; }
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InvulnerabilityOnHitTime{ 0.2f };

	
	UPROPERTY() UShapeComponent* HitBox{};
	float CurrentInvulnerabilityTimeLeft{};
};
