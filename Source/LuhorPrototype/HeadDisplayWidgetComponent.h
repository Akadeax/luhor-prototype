// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HeadDisplayWidgetComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UHeadDisplayWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeadDisplayChanged, UTexture2D*, Texture, float, Time);
	UPROPERTY(BlueprintAssignable) FOnHeadDisplayChanged OnHeadDisplayChanged;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeadDisplayActivated, UTexture2D*, Texture);
	UPROPERTY(BlueprintAssignable) FOnHeadDisplayActivated OnHeadDisplayActivated;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHeadDisplayDeactivated);
	UPROPERTY(BlueprintAssignable) FOnHeadDisplayDeactivated OnHeadDisplayDeactivated;
	
	UFUNCTION(BlueprintCallable)
	void Display(FName Display, float Time);
	UFUNCTION(blueprintCallable)
	void IndefiniteDisplay(FName Display);
	UFUNCTION(blueprintCallable)
	void DeactivateDisplay();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, UTexture2D*> HeadDisplays{};
};
