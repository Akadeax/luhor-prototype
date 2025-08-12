// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedAttackerComponent.h"

#include "Upgrades/UpgradesComponent.h"
#include "Util/ComponentUtil.h"
#include "Util/FDebugUtil.h"

COMPDEP_IMPL_START(URangedAttackerComponent)
	COMPDEP_DEP_AnyOnActorWithTagRequired(USkeletalMeshComponent, MainSkeletalMeshComponentTag)
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

void URangedAttackerComponent::CancelAttack()
{
	if (CurrentAttackState == EAttackState::None) return;
	
	MainSkeletalMesh->GetAnimInstance()->Montage_Stop(0.1f, RangedAttack->AttackData.Montage);
	SetAttackState(EAttackState::None);
	GetWorld()->GetTimerManager().ClearTimer(CurrentAttackStateTimer);
	OnRangedAttackCancelled.Broadcast();
}

void URangedAttackerComponent::BeginPlay()
{
	Super::BeginPlay();

	FDebugUtil::QuitCheckf(RangedAttack, TEXT("No attack assigned to %s on %s!"), *GetName(), *GetOwner()->GetName());

	MainSkeletalMesh = GetOwner()->FindComponentByTag<USkeletalMeshComponent>(MainSkeletalMeshComponentTag);
	FDebugUtil::QuitCheckf(MainSkeletalMesh, TEXT("Actor %s has no skeletal mesh with tag %s!"), *GetOwner()->GetName(), *MainSkeletalMeshComponentTag.ToString());
	
	MovementComponent = FComponentUtil::GetFirstComponentOfClass<ULuhorMovementComponent>(GetOwner());
}

void URangedAttackerComponent::DoWindup()
{
	SetAttackState(EAttackState::Windup);
	const FRangedAttackData& data{ RangedAttack->AttackData };
	
	OnAttackStarted.Broadcast();

	const float playRate{ GetSectionPlayRate(data.Montage, "windup", data.WindupTime) };
	const bool playSuccess{ MainSkeletalMesh->GetAnimInstance()->Montage_Play(data.Montage, playRate) != 0.f };
	if (!playSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Montage_Play failed!"));
	}

	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
	GetWorld()->GetTimerManager().SetTimer(
		CurrentAttackStateTimer, this, &ThisClass::DoContact, data.WindupTime
	);
}

void URangedAttackerComponent::DoContact()
{
	SetAttackState(EAttackState::Contact);
	const FRangedAttackData& data{ RangedAttack->AttackData };

	SpawnProjectile();

	if (MovementComponent)
	{
		FVector launchDir{ GetForwardVector() };
		launchDir.Z = 0;
		MovementComponent->DoCurvedLaunch(launchDir, data.CurvedLaunchData);
	}

	const float playRate{ GetSectionPlayRate(data.Montage, "contact", data.ContactTime) };
	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
	GetWorld()->GetTimerManager().SetTimer(
		CurrentAttackStateTimer, this, &ThisClass::DoRecovery, data.ContactTime
	);
}

void URangedAttackerComponent::DoRecovery()
{
	SetAttackState(EAttackState::Recovery);
	const FRangedAttackData& data{ RangedAttack->AttackData };

	const float playRate{ GetSectionPlayRate(data.Montage, "recovery", data.RecoveryTime) };
	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
	GetWorld()->GetTimerManager().SetTimer(
		CurrentAttackStateTimer, this, &ThisClass::EndAttack, data.RecoveryTime
	);
}

void URangedAttackerComponent::EndAttack()
{
	SetAttackState(EAttackState::None);
	OnAttackDone.Broadcast();
}

void URangedAttackerComponent::SpawnProjectile()
{
	FRotator rot{ GetForwardVector().Rotation() };
	rot.Pitch = 0;
	rot.Roll = 0;
	
	ARangedAttackProjectile* proj{ GetWorld()->SpawnActor<ARangedAttackProjectile>(
		RangedAttack->AttackData.ProjectileClass, GetOwner()->GetActorLocation(), rot
	) };
	check(proj);
	if (UUpgradesComponent* Upgrades = Cast<UUpgradesComponent>(GetOwner()->GetComponentByClass(UUpgradesComponent::StaticClass())))
	{
		FStatModifier modifiers{Upgrades->GetCurrentModifier()};
		RangedAttack->AttackData.Damage += modifiers.RangedAttackModifier;
		RangedAttack->AttackData.Damage *= modifiers.RangedAttackMultiplier;
	}

	proj->InitializeProjectile({ RangedAttack, Faction, GetOwner(), GetForwardVector().Rotation() });

	proj->OnProjectileHit.AddDynamic(this, &ThisClass::OnProjectileHit);
}

void URangedAttackerComponent::OnProjectileHit(const FHittableHitData& Data)
{
	OnRangedAttackHit.Broadcast(Data);
}
