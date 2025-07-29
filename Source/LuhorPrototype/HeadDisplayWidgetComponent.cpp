// Fill out your copyright notice in the Description page of Project Settings.

#include "HeadDisplayWidgetComponent.h"

void UHeadDisplayWidgetComponent::Display(FName Display, float Time)
{
	UTexture2D** texture{ HeadDisplays.Find(Display) };
	if (texture == nullptr) return;
	
	OnHeadDisplayChanged.Broadcast(*texture, Time);
}
