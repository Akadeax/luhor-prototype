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
	UHeadDisplayWidgetComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, FTexture*> HeadDisplayTextures{};
};
