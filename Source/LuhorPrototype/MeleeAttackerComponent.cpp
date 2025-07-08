// Fill out your copyright notice in the Description page of Project Settings.

#include "MeleeAttackerComponent.h"

#include "Components/ShapeComponent.h"


UMeleeAttackerComponent::UMeleeAttackerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


bool UMeleeAttackerComponent::TryAttack()
{
	if (CurrentAttackState != EMeleeAttackState::None) return false;

	DoWindup();
	return true;
}

void UMeleeAttackerComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MeleeAttackChain, TEXT("No attack chain assigned to %s on %s!"), *GetName(), *GetOwner()->GetName());

	for (FMeleeAttackData& data : MeleeAttackChain->Attacks)
	{
		checkf(data.Montage, TEXT("Melee Attack Chain %s doesn't have a montage assigned on all attacks!"), *MeleeAttackChain.GetName());
	}
	
	// Fetch and verify skeletal mesh
	MainSkeletalMesh = GetOwner()->FindComponentByTag<USkeletalMeshComponent>(MainSkeletalMeshComponentTag);
	checkf(MainSkeletalMesh, TEXT("Actor %s has no skeletal mesh with tag %s!"), *GetOwner()->GetName(), *MainSkeletalMeshComponentTag.ToString());

	// Fetch and verify contact collision
	const TArray<TObjectPtr<USceneComponent>>& children{ GetAttachChildren() };
	const TObjectPtr<USceneComponent>* first{};
	first = children.FindByPredicate([](const TObjectPtr<USceneComponent>& comp)
	{
		return comp->IsA<UShapeComponent>();
	});

	checkf(first, TEXT("No shape component found as child of %s on %s!"), *GetName(), *GetOwner()->GetName());
	ContactCollision = Cast<UShapeComponent>(*first);
	check(ContactCollision);

	// Initialize contact collision
	ContactCollision->SetGenerateOverlapEvents(true);
	ContactCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnContactCollisionBeginOverlap);
	DisableContactCollision();

}

void UMeleeAttackerComponent::DoWindup()
{
	OnMeleeAttackStarted.Broadcast();
	SetMeleeAttackState(EMeleeAttackState::Windup);
	
	const FMeleeAttackData& data{ GetCurrentAttack() };

	const float playRate{ GetSectionPlayRate(data.Montage, "windup", data.WindupTime) };
	const bool playSuccess{ MainSkeletalMesh->GetAnimInstance()->Montage_Play(data.Montage, playRate) != 0.f };
	if (!playSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Montage_Play failed!"));
	}

	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
	
	GetWorld()->GetTimerManager().SetTimer(CurrentAttackStateTimer, this, &ThisClass::DoContact, data.WindupTime);
}

void UMeleeAttackerComponent::DoContact()
{
	SetMeleeAttackState(EMeleeAttackState::Contact);

	const FMeleeAttackData& data{ GetCurrentAttack() };
	GetWorld()->GetTimerManager().SetTimer(CurrentAttackStateTimer, this, &ThisClass::DoRecovery, data.ContactTime);

	EnableContactCollision();

	const float playRate{ GetSectionPlayRate(data.Montage, "contact", data.ContactTime) };
	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
}

void UMeleeAttackerComponent::DoRecovery()
{
	SetMeleeAttackState(EMeleeAttackState::Recovery);

	const FMeleeAttackData& data{ GetCurrentAttack() };
	GetWorld()->GetTimerManager().SetTimer(CurrentAttackStateTimer, this, &ThisClass::EndAttack, data.RecoveryTime);

	DisableContactCollision();

	const float playRate{ GetSectionPlayRate(data.Montage, "recovery", data.RecoveryTime) };
	MainSkeletalMesh->GetAnimInstance()->Montage_SetPlayRate(data.Montage, playRate);
	
}

void UMeleeAttackerComponent::EndAttack()
{
	OnMeleeAttackDone.Broadcast();
	SetMeleeAttackState(EMeleeAttackState::None);
}

void UMeleeAttackerComponent::EnableContactCollision()
{
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void UMeleeAttackerComponent::DisableContactCollision()
{
	ContactCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UMeleeAttackerComponent::OnContactCollisionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("CONTACT!"));
}

void UMeleeAttackerComponent::SetMeleeAttackState(EMeleeAttackState NewState)
{
	if (CurrentAttackState == NewState) return;
	
	CurrentAttackState = NewState;
	OnMeleeAttackStateChanged.Broadcast(NewState);
}

const FMeleeAttackData& UMeleeAttackerComponent::GetCurrentAttack() const
{
	return MeleeAttackChain->Attacks[CurrentChainComboIndex];
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

