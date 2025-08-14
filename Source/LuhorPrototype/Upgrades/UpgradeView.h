// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "UpgradeView.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LUHORPROTOTYPE_API UUpgradeView : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called when the widget is constructed (like BeginPlay for widgets)
	virtual void NativeConstruct() override;

	void SetText(FText UpgradeTitle, FText UpgradeDescription);
	// Called every frame while the widget is active
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DescriptionText;

	
};
