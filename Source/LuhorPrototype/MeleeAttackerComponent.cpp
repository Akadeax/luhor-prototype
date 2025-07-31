// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAttackerComponent.h"

#include "HittableComponent.h"
#include "LuhorMovementComponent.h"
#include "Util/ComponentUtil.h"
#include "Components/ShapeComponent.h"
#include "Util/FDebugUtil.h"

COMPDEP_IMPL_START(UMeleeAttackerComponent)
	COMPDEP_DEP_AnyOnActorWithTagRequired(USkeletalMeshComponent, MainSkeletalMeshComponentTag)
	COMPDEP_DEP_AnyOnActorOptional(ULuhorMovementComponent)
	COMPDEP_DEP_ChildRequired(UShapeComponent)
COMPDEP_IMPL_END

void UMeleeAttackerComponent::BeginPlay()
{
	Super::BeginPlay();

	FDebugUtil::QuitCheckf(MeleeAttackChain, TEXT("No attack chain assigned to %s on %s!"), *GetName(), *GetOwner()->GetName());
	for (const FMeleeAttackData& data : MeleeAttackChain->Attacks)
	{
		FDebugUtil::QuitCheckf(data.Montage, TEXT("Melee Attack Chain %s doesn't have a montage assigned on all attacks!"), *MeleeAttackChain.GetName());
	}
	
	MainSkeletalMesh = GetOwner()->FindComponentByTag<USkeletalMeshComponent>(MainSkeletalMeshComponentTag);
	FDebugUtil::QuitCheckf(MainSkeletalMesh, TEXT("Actor %s has no skeletal mesh with tag %s!"), *GetOwner()->GetName(), *MainSkeletalMeshComponentTag.ToString());

	ContactCollision = FComponentUtil::GetChildComponentOfClass<UShapeComponent>(this);
	FDebugUtil::QuitCheckf(ContactCollision, TEXT("Component %s on actor %s has no shape component as a child!"), *GetName(), *GetOwner()->GetName());

	// Initialize contact collision
	ContactCollision->SetGenerateOverlapEvents(true);
	ContactCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnContactCollisionBeginOverlap);
	BaseCollisionTransform = ContactCollision->GetComponentTransform();
	DisableContactCollision();

	constexpr ECollisionChannel HITBOX_CHANNEL{ ECC_GameTraceChannel1 };
	constexpr ECollisionChannel ATTACKBOX_CHANNEL{ ECC_GameTraceChannel2 };
	ContactCollision->SetCollisionObjectType(ATTACKBOX_CHANNEL);
	ContactCollision->SetCollisionResponseToChannel(HITBOX_CHANNEL, ECR_Overlap);
	ContactCollision->SetCollisionResponseToChannel(ATTACKBOX_CHANNEL, ECR_Ignore);
	MovementComponent = FComponentUtil::GetFirstComponentOfClass<ULuhorMovementComponent>(GetOwner());
}

bool UMeleeAttackerComponent::TryAttack()
{
	if (CurrentAttackState != EAttackState::None)
	{
		if (!AttackQueued && CurrentChainIndex != MeleeAttackChain->Attacks.Num() - 1)
		{
			AttackQueued = true;
		}
		
		return false;
	}
	
	if (GetWorld()->GetTimerManager().IsTimerActive(ChainLeniencyTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(ChainLeniencyTimer);
		++CurrentChainIndex;
	}

	AttackQueued = false;
	DoWindup();
	return true;
}

void UMeleeAttackerComponent::CancelAttack()
{
	if (CurrentAttackState == EAttackState::None) return;
	
	for (FMeleeAttackData& data : MeleeAttackChain->Attacks)
	{
		MainSkeletalMesh->GetAnimInstance()->Montage_Stop(0.1f, data.Montage);
	}

	AttackQueued = false;
	CurrentChainIndex = 0;
	CurrentAttackState = EAttackState::None;

	DisableContactCollision();
	
	GetWorld()->GetTimerManager().ClearTimer(CurrentAttackStateTimer);
	GetWorld()->GetTimerManager().ClearTimer(ChainLeniencyTimer);
	
	OnMeleeAttackCancelled.Broadcast();
}



void UMeleeAttackerComponent::DoWindup()
{
	SetAttackState(EAttackState::Windup);
	const FMeleeAttackData& data{ GetCurrentAttack() };
	
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

void UMeleeAttackerComponent::DoContact()
{
	SetAttackState(EAttackState::Contact);
	const FMeleeAttackData& data{ GetCurrentAttack() };
	
	EnableContactCollision(data);

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

void UMeleeAttackerComponent::DoRecovery()
{
	SetAttackState(EAttackState::Recovery);
	const FMeleeAttackData& data{ GetCurrentAttack() };

	DisableContactCollision();

	const float playRate{ GetSectionPlayRate(data.Montage, "recovery", data.RecoveryTime) };
	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
	GetWorld()->GetTimerManager().SetTimer(
		CurrentAttackStateTimer, this, &ThisClass::EndAttack, data.RecoveryTime
	);
}

void UMeleeAttackerComponent::EndAttack()
{
	SetAttackState(EAttackState::None);

	if (CurrentChainIndex >= MeleeAttackChain->Attacks.Num() - 1)
	{
		AttackQueued = false;
		EndChainLeniency();
		
		OnAttackDone.Broadcast();
		OnMeleeAttackChainDone.Broadcast();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			ChainLeniencyTimer, this, &ThisClass::EndChainLeniency, MeleeAttackChain->ChainLeniencyTime
		);

		OnAttackDone.Broadcast();
	}
	
	if (AttackQueued) TryAttack();
}

void UMeleeAttackerComponent::EndChainLeniency()
{
	CurrentChainIndex = 0;
}

void UMeleeAttackerComponent::EnableContactCollision(const FMeleeAttackData& data)
{
	if (data.HitBoxTransform.Equals(FTransform::Identity))
	{
		ContactCollision->SetRelativeTransform(BaseCollisionTransform);
	}
	else
	{
		ContactCollision->SetRelativeTransform(data.HitBoxTransform);
	}
	
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UMeleeAttackerComponent::DisableContactCollision()
{
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UMeleeAttackerComponent::OnContactCollisionBeginOverlap(
	UPrimitiveComponent*,
	AActor*,
	UPrimitiveComponent* OtherComp,
	int32,
	bool,
	const FHitResult&)
{
	UHittableComponent* hittable{ Cast<UHittableComponent>(OtherComp->GetAttachParent()) };
	if (!hittable) return;

	const FMeleeAttackData& data{ GetCurrentAttack() };

	const FHittableHitData hitData{ data.Damage, GetOwner(), Faction };
	
	hittable->Hit(hitData);
	OnMeleeAttackHit.Broadcast(hitData);
}

const FMeleeAttackData& UMeleeAttackerComponent::GetCurrentAttack() const
{
	return MeleeAttackChain->Attacks[CurrentChainIndex];
}


