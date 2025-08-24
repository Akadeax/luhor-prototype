// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HittableComponent.h"
#include "RangedAttackProjectile.generated.h"

class UBoxComponent;
class UFactionAssociation;
class URangedAttack;

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URangedAttack* RangedAttack{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFactionAssociation* SourceFaction{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Source{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Direction{};
};

UCLASS()
class LUHORPROTOTYPE_API ARangedAttackProjectile : public AActor
{
	GENERATED_BODY()

public:
	ARangedAttackProjectile();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnProjectileHit, const FHittableHitData&, Data, bool, WasLethal, UHittableComponent*, Target);
 	UPROPERTY(BlueprintAssignable) FOnProjectileHit OnProjectileHit;
	
	void InitializeProjectile(const FProjectileData& Data);

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UBoxComponent* Collision;

	FProjectileData ProjectileData;

	UFUNCTION()
	void OnCollisionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
