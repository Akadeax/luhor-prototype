// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradePilon.h"

#include "Components/BoxComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LuhorPrototype/LuhorEnemyCharacter.h"


// Sets default values
AUpgradePilon::AUpgradePilon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Widget
	WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComp"));
	WidgetComp->SetupAttachment(RootComponent);

	// Option 1 (box)
	Option1Shape = CreateDefaultSubobject<UBoxComponent>(TEXT("Option1Shape"));
	Option1Shape->SetupAttachment(RootComponent);

	// Option 2 (box)
	Option2Shape = CreateDefaultSubobject<UBoxComponent>(TEXT("Option2Shape"));
	Option2Shape->SetupAttachment(RootComponent);
}

bool AUpgradePilon::CanInteract_Implementation(AActor* Interactor) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALuhorEnemyCharacter::StaticClass(), FoundActors);
	
	UE_LOG(LogTemp, Log, TEXT("Checked"));
	return FoundActors.Num() == 0 && Upgrades.Num() !=0;
}

void AUpgradePilon::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Log, TEXT("Interacted"));
	WidgetComp->SetVisibility(true);
	
}

FText AUpgradePilon::GetInteractPrompt_Implementation(const AActor* Interactor) const

{
	return FText();
}

// Called when the game starts or when spawned
void AUpgradePilon::BeginPlay()
{
	Super::BeginPlay();
	WidgetComp->SetVisibility(false);
}

// Called every frame
void AUpgradePilon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

