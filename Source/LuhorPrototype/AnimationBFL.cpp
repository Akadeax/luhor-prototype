// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimationBFL.h"

float UAnimationBFL::GetMontageSectionLength(UAnimMontage* Montage, FName SectionName)
{
	check(Montage);
	check(SectionName.GetStringLength() > 0);
	
	const int32 index{ Montage->GetSectionIndex(SectionName) };
	check(index != INDEX_NONE);

	return Montage->GetSectionLength(index);
}
