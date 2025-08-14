// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeView.h"

#include "Navigation/PathFollowingComponent.h"


void UUpgradeView::NativeConstruct()
{
	Super::NativeConstruct();
	SetText(FText::FromString(TEXT("beep")), FText::FromString(TEXT("No Upgrade passed")));
}

void UUpgradeView::SetText(FText UpgradeTitle, FText UpgradeDescription)
{
	TitleText->SetText(UpgradeTitle);
	DescriptionText->SetText(UpgradeDescription);
}

void UUpgradeView::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}
