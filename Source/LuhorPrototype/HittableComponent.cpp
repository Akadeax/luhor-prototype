// Fill out your copyright notice in the Description page of Project Settings.

#include "HittableComponent.h"
#include "Util/ComponentUtil.h"
#include "Components/ShapeComponent.h"
#include "Util/FDebugUtil.h"

UHittableComponent::UHittableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHittableComponent::BeginPlay()
{
	Super::BeginPlay();

	HitBox = FComponentUtil::GetChildComponentOfClass<UShapeComponent>(this);
	FDebugUtil::QuitCheckf(HitBox, TEXT("Actor %s does not have a HitBox!"), *GetOwner()->GetName());
	
	constexpr ECollisionChannel HITBOX_CHANNEL{ ECC_GameTraceChannel1 };
	constexpr ECollisionChannel ATTACKBOX_CHANNEL{ ECC_GameTraceChannel2 };
	HitBox->SetCollisionObjectType(HITBOX_CHANNEL);
	HitBox->SetCollisionResponseToChannel(HITBOX_CHANNEL, ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ATTACKBOX_CHANNEL, ECR_Overlap);
}

void UHittableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentInvulnerabilityTimeLeft <= 0.f) return;
	
	CurrentInvulnerabilityTimeLeft -= DeltaTime;
	if (CurrentInvulnerabilityTimeLeft <= 0.f)
	{
		OnInvulnerableEnd.Broadcast();
	}
}

void UHittableComponent::Hit(FHittableHitData HitData)
{
	OnHit.Broadcast(HitData);
}

void UHittableComponent::MakeInvulnerable(float Time, MakeInvulnerableMode Mode)
{
	if (Mode == MakeInvulnerableMode::TimeAdditive)
	{
		CurrentInvulnerabilityTimeLeft += Time;
		OnInvulnerable.Broadcast();
		return;
	}

	if (Mode == MakeInvulnerableMode::IfNotInvulnerableAlready && !IsInvulnerable())
	{
		CurrentInvulnerabilityTimeLeft = Time;
		OnInvulnerable.Broadcast();
		return;
	}

	if (Mode == MakeInvulnerableMode::SetTimeIfLonger && Time > CurrentInvulnerabilityTimeLeft)
	{
		CurrentInvulnerabilityTimeLeft = Time;
		OnInvulnerable.Broadcast();
	}
}
