// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAttackerComponent.h"

#include "HittableComponent.h"
#include "LuhorMovementComponent.h"
#include "Util/ComponentUtil.h"
#include "Components/ShapeComponent.h"
#include "Util/FDebugUtil.h"


UMeleeAttackerComponent::UMeleeAttackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


bool UMeleeAttackerComponent::TryAttack()
{
	if (CurrentAttackState != EMeleeAttackState::None)
	{
		if (!AttackQueued)
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
	DisableContactCollision();

	constexpr ECollisionChannel HITBOX_CHANNEL{ ECC_GameTraceChannel1 };
	constexpr ECollisionChannel ATTACKBOX_CHANNEL{ ECC_GameTraceChannel2 };
	ContactCollision->SetCollisionObjectType(ATTACKBOX_CHANNEL);
	ContactCollision->SetCollisionResponseToChannel(HITBOX_CHANNEL, ECR_Overlap);
	ContactCollision->SetCollisionResponseToChannel(ATTACKBOX_CHANNEL, ECR_Ignore);

	MovementComponent = FComponentUtil::GetFirstComponentOfClass<ULuhorMovementComponent>(GetOwner());
}

void UMeleeAttackerComponent::DoWindup()
{
	SetMeleeAttackState(EMeleeAttackState::Windup);
	const FMeleeAttackData& data{ GetCurrentAttack() };
	
	OnMeleeAttackStarted.Broadcast();

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
	SetMeleeAttackState(EMeleeAttackState::Contact);
	const FMeleeAttackData& data{ GetCurrentAttack() };

	EnableContactCollision();

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
	SetMeleeAttackState(EMeleeAttackState::Recovery);
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
	SetMeleeAttackState(EMeleeAttackState::None);

	if (CurrentChainIndex + 1 >= MeleeAttackChain->Attacks.Num())
	{
		AttackQueued = false;
		EndChainLeniency();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			ChainLeniencyTimer, this, &ThisClass::EndChainLeniency, MeleeAttackChain->ChainLeniencyTime
		);
	}

	if (AttackQueued) TryAttack();
	
	OnMeleeAttackDone.Broadcast();
}

void UMeleeAttackerComponent::EndChainLeniency()
{
	CurrentChainIndex = 0;
}

void UMeleeAttackerComponent::EnableContactCollision()
{
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UMeleeAttackerComponent::DisableContactCollision()
{
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UMeleeAttackerComponent::OnContactCollisionBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UHittableComponent* hittable{ Cast<UHittableComponent>(OtherComp->GetAttachParent()) };
	if (!hittable) return;

	const FMeleeAttackData& data{ GetCurrentAttack() };
	hittable->Hit({ data.Damage });
}

void UMeleeAttackerComponent::SetMeleeAttackState(EMeleeAttackState NewState)
{
	if (CurrentAttackState == NewState) return;
	
	CurrentAttackState = NewState;
	OnMeleeAttackStateChanged.Broadcast(NewState);
}

const FMeleeAttackData& UMeleeAttackerComponent::GetCurrentAttack() const
{
	return MeleeAttackChain->Attacks[CurrentChainIndex];
}

float UMeleeAttackerComponent::ConvertPlayRate(float OriginalTime, float DesiredTime) const
{
	return OriginalTime / DesiredTime;
}

float UMeleeAttackerComponent::GetSectionPlayRate(const UAnimMontage* Montage, FName SectionName, float DesiredTime) const
{
	const int32 sectionIndex{ Montage->GetSectionIndex(SectionName) };
	if (!Montage->IsValidSectionIndex(sectionIndex)) return 0.f;
	
	const float sectionLength{ Montage->GetSectionLength(sectionIndex) };
	if (sectionLength == 0.f) return 0.f;

	return ConvertPlayRate(sectionLength, DesiredTime);
}

void UMeleeAttackerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

