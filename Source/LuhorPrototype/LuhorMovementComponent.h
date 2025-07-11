// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LuhorMovementComponent.generated.h"

USTRUCT(BlueprintType)
struct LUHORPROTOTYPE_API FCurvedLaunchData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Time{ 0.3f };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Force{ 50.f };
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* ForceMultiplierCurve{};
};

class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API ULuhorMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLaunchStart, FCurvedLaunchData, Data);
	UPROPERTY(BlueprintAssignable) FOnLaunchStart OnLaunchStart;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLaunchEnd, FCurvedLaunchData, Data);
	UPROPERTY(BlueprintAssignable) FOnLaunchEnd OnLaunchEnd;

	
	UFUNCTION(BlueprintCallable)
	void DoCurvedLaunch(FVector Direction, FCurvedLaunchData Data);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick) override;
	
private:
	FVector CurrentDirection{};
	FCurvedLaunchData CurrentLaunchData;
	
	float CurrentLaunchTimeLeft{};

	static constexpr float STATIC_LAUNCH_FORCE_MULTIPLIER{ 100000.f };
};
