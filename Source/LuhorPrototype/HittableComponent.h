// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ComponentDependencies.h"
#include "LuhorMovementComponent.h"
#include "HittableComponent.generated.h"

class ULuhorMovementComponent;
class UHealthComponent;
class UFactionAssociation;

UENUM(BlueprintType)
enum class HitType : uint8
{
	Melee, Ranged
};

USTRUCT(BlueprintType)
struct FHittableHitData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	float Damage{};
	
	UPROPERTY(BlueprintReadWrite)
	AActor* Source{};

	UPROPERTY(BlueprintReadWrite)
	FVector TargetLocation{};

	UPROPERTY(BlueprintReadWrite)
	UFactionAssociation* SourceFaction{};

	UPROPERTY(BlueprintReadWrite)
	HitType Type{};
};

UENUM(BlueprintType)
enum class MakeInvulnerableMode : uint8
{
	IfNotInvulnerableAlready,
	SetTimeIfLonger,
	TimeAdditive,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UHittableComponent : public USceneComponent, public IComponentDependencies
{
	GENERATED_BODY()
	COMPDEP_DECL()

public:
	UHittableComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, FHittableHitData, Data);
	UPROPERTY(BlueprintAssignable) FOnHit OnHit;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerable);
	UPROPERTY(BlueprintAssignable) FOnInvulnerable OnInvulnerable;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerableEnd);
	UPROPERTY(BlueprintAssignable) FOnInvulnerableEnd OnInvulnerableEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitStun);
	UPROPERTY(BlueprintAssignable) FOnHitStun OnHitStun;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitStunEnd);
	UPROPERTY(BlueprintAssignable) FOnHitStunEnd OnHitStunEnd;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMarked);
	UPROPERTY(BlueprintAssignable) FMarked OnMarked;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMarkConsumed);
	UPROPERTY(BlueprintAssignable) FMarkConsumed OnMarkConsumed;

	
	void Hit(const FHittableHitData& HitData);
	
	UFUNCTION(BlueprintCallable)
	void CustomHitStun(float Time);
	
	UFUNCTION(BlueprintCallable)
	void MakeInvulnerable(float Time, MakeInvulnerableMode Mode = MakeInvulnerableMode::SetTimeIfLonger);
	
	UFUNCTION(BlueprintCallable)
	bool IsInvulnerable() const { return CurrentInvulnerabilityTimeLeft > 0.f; }

	UFUNCTION()
	void MakeVulnerable();
	
	UFUNCTION(BlueprintCallable)
	bool IsHitStunned() const { return CurrentHitStunTimeLeft > 0.f; }	

	UFUNCTION(BlueprintCallable)
	float GetHitStunTimeLeft() const { return CurrentHitStunTimeLeft; }

	UFUNCTION(BlueprintCallable)
	UFactionAssociation* GetFaction() const { return Faction; }

	UFUNCTION(BlueprintCallable) void SetMarked();
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UFactionAssociation* Faction{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InvulnerabilityOnHitTime{ 0.2f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float HitStunTime{ 0.2f };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool LaunchOnHit{ true };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FCurvedLaunchData LaunchOnHitData{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MarkedDamageMulti{ 2.f };
	
	UPROPERTY() UShapeComponent* HitBox{};
	UPROPERTY() UHealthComponent* HealthComponent{};
	UPROPERTY() ULuhorMovementComponent* MovementComponent{};
	
	float CurrentInvulnerabilityTimeLeft{};
	float CurrentHitStunTimeLeft{};

	void HitStun();

	void TickInvulnerability(float DeltaTime);
	void TickHitStun(float DeltaTime);

	bool Marked{ false };
};
