// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedAttackerComponent.h"

#include "Util/ComponentUtil.h"
#include "Util/FDebugUtil.h"

COMPDEP_IMPL_START(URangedAttackerComponent)
	COMPDEP_DEP_AnyOnActorRequired(USkeletalMeshComponent, MainSkeletalMeshComponentTag)
	COMPDEP_DEP_AnyOnActorOptional(ULuhorMovementComponent)
COMPDEP_IMPL_END

bool URangedAttackerComponent::TryAttack()
{
	if (CurrentAttackState != EAttackState::None)
	{
		return false;
	}

	DoWindup();
	return true;
}

void URangedAttackerComponent::BeginPlay()
{
	Super::BeginPlay();

	FDebugUtil::QuitCheckf(Attack, TEXT("No attack assigned to %s on %s!"), *GetName(), *GetOwner()->GetName());

	MainSkeletalMesh = GetOwner()->FindComponentByTag<USkeletalMeshComponent>(MainSkeletalMeshComponentTag);
	FDebugUtil::QuitCheckf(MainSkeletalMesh, TEXT("Actor %s has no skeletal mesh with tag %s!"), *GetOwner()->GetName(), *MainSkeletalMeshComponentTag.ToString());
	
	MovementComponent = FComponentUtil::GetFirstComponentOfClass<ULuhorMovementComponent>(GetOwner());
}

void URangedAttackerComponent::DoWindup()
{
}

void URangedAttackerComponent::DoContact()
{
}

void URangedAttackerComponent::DoRecovery()
{
}

void URangedAttackerComponent::EndAttack()
{
}
