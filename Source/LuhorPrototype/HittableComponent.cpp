// Fill out your copyright notice in the Description page of Project Settings.

#include "HittableComponent.h"

#include "HealthComponent.h"
#include "Util/ComponentUtil.h"
#include "Components/ShapeComponent.h"
#include "LuhorMovementComponent.h"
#include "Util/FDebugUtil.h"
#include "DrawDebugHelpers.h"

COMPDEP_IMPL_START(UHittableComponent)
	COMPDEP_DEP_ChildRequired(UShapeComponent)
	COMPDEP_DEP_AnyOnActorOptional(UHealthComponent)
	COMPDEP_DEP_AnyOnActorOptional(ULuhorMovementComponent)
COMPDEP_IMPL_END

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

	HealthComponent = FComponentUtil::GetFirstComponentOfClass<UHealthComponent>(GetOwner());
	MovementComponent = FComponentUtil::GetFirstComponentOfClass<ULuhorMovementComponent>(GetOwner());
}

void UHittableComponent::HitStun()
{
	CurrentHitStunTimeLeft = HitStunTime;
	OnHitStun.Broadcast();
}

void UHittableComponent::TickInvulnerability(float DeltaTime)
{
	if (CurrentInvulnerabilityTimeLeft <= 0.f) return;
	
	CurrentInvulnerabilityTimeLeft -= DeltaTime;
	if (CurrentInvulnerabilityTimeLeft <= 0.f)
	{
		OnInvulnerableEnd.Broadcast();
	}
}

void UHittableComponent::TickHitStun(float DeltaTime)
{
	if (CurrentHitStunTimeLeft <= 0.f) return;
	
	CurrentHitStunTimeLeft -= DeltaTime;
	if (CurrentHitStunTimeLeft <= 0.f)
	{
		OnHitStunEnd.Broadcast();
	}
}

void UHittableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickInvulnerability(DeltaTime);
	TickHitStun(DeltaTime);
}

void UHittableComponent::Hit(FHittableHitData HitData)
{
	if (IsInvulnerable()) return;
	
	OnHit.Broadcast(HitData);
	
	MakeInvulnerable(InvulnerabilityOnHitTime);
	HitStun();
	
	if (HealthComponent)
	{
		HealthComponent->Damage(HitData.Damage);
	}
	if (MovementComponent)
	{
		FVector dir{ HitData.Source->GetActorForwardVector() };
		dir.Z = 0.f;
		
		MovementComponent->DoCurvedLaunch(dir, LaunchOnHitData);
	}
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
