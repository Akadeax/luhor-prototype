// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedAttackProjectile.h"

#include "AmbrosiaHealthComponent.h"
#include "AttackData.h"
#include "HittableComponent.h"
#include "LuhorPlayerCharacter.h"
#include "Components/BoxComponent.h"

ARangedAttackProjectile::ARangedAttackProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Collision = CreateDefaultSubobject<UBoxComponent>("Collision");
	RootComponent = Collision;
	
	constexpr ECollisionChannel HITBOX_CHANNEL{ ECC_GameTraceChannel1 };
	constexpr ECollisionChannel ATTACKBOX_CHANNEL{ ECC_GameTraceChannel2 };

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Collision->SetCollisionObjectType(ATTACKBOX_CHANNEL);
	Collision->SetCollisionResponseToChannel(HITBOX_CHANNEL, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ATTACKBOX_CHANNEL, ECR_Ignore);
}

void ARangedAttackProjectile::InitializeProjectile(const FProjectileData& Data)
{
	ProjectileData = Data;
	SetActorRotation(Data.Direction);

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCollisionBeginOverlap);
}

void ARangedAttackProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector offsetPerSec{ ProjectileData.Direction.Vector() * ProjectileData.RangedAttack->AttackData.ProjectileSpeed };
	AddActorWorldOffset(offsetPerSec * DeltaSeconds, true);
}

void ARangedAttackProjectile::OnCollisionBeginOverlap(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32,
	bool,
	const FHitResult&)
{
	UHittableComponent* hittable{ Cast<UHittableComponent>(OtherComp->GetAttachParent()) };
	if (!hittable)
	{
		Destroy();
		return;
	}
	
	if (hittable->GetFaction() == ProjectileData.SourceFaction) return;
	
	const FHittableHitData data{ ProjectileData.RangedAttack->AttackData.Damage, ProjectileData.Source, ProjectileData.SourceFaction };

	hittable->Hit(data);
	bool wasLethal {true};
	if (UHealthComponent* HealthComp{ Cast<UHealthComponent>(OtherActor->GetComponentByClass(UHealthComponent::StaticClass()))})
	{
		wasLethal = HealthComp->GetCurrentHealth() < 0;
		if (wasLethal)
		{
			if (ALuhorPlayerCharacter* Character{Cast<ALuhorPlayerCharacter>(OtherActor)})
			{
				wasLethal = Cast<UAmbrosiaHealthComponent>(HealthComp)->IsDead();
			}	
		}
	}
	OnProjectileHit.Broadcast(data,wasLethal);

	Destroy();
}
