// Fill out your copyright notice in the Description page of Project Settings.

#include "HeadDisplayWidgetComponent.h"

void UHeadDisplayWidgetComponent::Display(FName Display, float Time)
{
	UTexture2D** texture{ HeadDisplays.Find(Display) };
	if (texture == nullptr) return;
	
	OnHeadDisplayChanged.Broadcast(*texture, Time);
}

void UHeadDisplayWidgetComponent::IndefiniteDisplay(FName Display)
{
	UTexture2D** texture{ HeadDisplays.Find(Display) };
	if (texture == nullptr) return;
	OnHeadDisplayActivated.Broadcast(*texture);
}

void UHeadDisplayWidgetComponent::DeactivateDisplay()
{
	OnHeadDisplayDeactivated.Broadcast();
}
