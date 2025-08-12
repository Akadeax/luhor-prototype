// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradePilon.h"


// Sets default values
AUpgradePilon::AUpgradePilon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AUpgradePilon::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
	
}

void AUpgradePilon::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("Interacted"));
}

FText AUpgradePilon::GetInteractPrompt_Implementation(const AActor* Interactor) const

{
	return FText();
}

// Called when the game starts or when spawned
void AUpgradePilon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUpgradePilon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

