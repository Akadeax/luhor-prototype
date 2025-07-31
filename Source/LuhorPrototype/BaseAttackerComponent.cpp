// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseAttackerComponent.h"

void UBaseAttackerComponent::SetAttackState(EAttackState NewState)
{
	if (CurrentAttackState == NewState) return;
	
	CurrentAttackState = NewState;
	OnAttackStateChanged.Broadcast(NewState);
}

float UBaseAttackerComponent::ConvertPlayRate(float OriginalTime, float DesiredTime)
{
	return OriginalTime / DesiredTime;
}

float UBaseAttackerComponent::GetSectionPlayRate(const UAnimMontage* Montage, FName SectionName, float DesiredTime)
{
	const int32 sectionIndex{ Montage->GetSectionIndex(SectionName) };
	if (!Montage->IsValidSectionIndex(sectionIndex)) return 0.f;
	
	const float sectionLength{ Montage->GetSectionLength(sectionIndex) };
	if (sectionLength == 0.f) return 0.f;

	return ConvertPlayRate(sectionLength, DesiredTime);
}

