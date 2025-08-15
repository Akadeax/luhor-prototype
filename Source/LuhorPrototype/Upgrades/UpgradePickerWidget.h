// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradeView.h"
#include "Blueprint/UserWidget.h"
#include "UpgradePickerWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API UUpgradePickerWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// These will bind to widgets placed in the BP that derive from UUpgradeView
	UPROPERTY(meta=(BindWidget))
	UUpgradeView* LeftView;

	UPROPERTY(meta=(BindWidget))
	UUpgradeView* RightView;

public:
	// Optional helpers
	UFUNCTION(BlueprintCallable, Category="Upgrade")
	void SetLeftText(const FText& Title, const FText& Desc)
	{
		if (LeftView) { LeftView->SetText(Title, Desc); }
	}

	UFUNCTION(BlueprintCallable, Category="Upgrade")
	void SetRightText(const FText& Title, const FText& Desc)
	{
		if (RightView) { RightView->SetText(Title, Desc); }
	}
};