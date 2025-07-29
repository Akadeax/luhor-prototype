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
	
	UFUNCTION(BlueprintCallable)
	void Display(FName Display, float Time);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, UTexture2D*> HeadDisplays{};
};
