// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradePilon.h"

#include "UpgradePickerWidget.h"
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
	LeftShape = CreateDefaultSubobject<UBoxComponent>(TEXT("Option1Shape"));
	LeftShape->SetupAttachment(RootComponent);

	// Option 2 (box)
	RightShape = CreateDefaultSubobject<UBoxComponent>(TEXT("Option2Shape"));
	RightShape->SetupAttachment(RootComponent);
}

bool AUpgradePilon::CanInteract_Implementation(AActor* Interactor) const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALuhorEnemyCharacter::StaticClass(), FoundActors);
	
	UE_LOG(LogTemp, Log, TEXT("Checked"));
	return FoundActors.Num() == 0 && Upgrades.Num() !=0 && PilonState != PilonState::Used;
}

void AUpgradePilon::Interact_Implementation(AActor* Interactor)
{
	if (!PlayerUpgradeComp)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALuhorCharacter::StaticClass(), FoundActors);
		PlayerUpgradeComp = Cast<UUpgradesComponent>(Cast<ALuhorCharacter>(FoundActors[0])->GetComponentByClass(UUpgradesComponent::StaticClass()));
	}
	if (!PlayerUpgradeComp) return;
	
	UE_LOG(LogTemp, Log, TEXT("Interacted"));
	switch (PilonState)
	{
	case PilonState::Inactive:
		RefreshUpgradeOptions();
		if (Upgrades.Num() >= 2)
		{
			int32 FirstIndex = FMath::RandRange(0, Upgrades.Num() - 1);
			int32 SecondIndex;
			do
			{
				SecondIndex = FMath::RandRange(0, Upgrades.Num() - 1);
			}
			while (SecondIndex == FirstIndex);

			UpgradeOption1 = Upgrades[FirstIndex];
			UpgradeOption2 = Upgrades[SecondIndex];
		}
		else if (Upgrades.Num() == 1)
		{
			UpgradeOption1 = Upgrades[0];
			UpgradeOption2 = Upgrades[0];
		}
		if (UUpgradePickerWidget* Widget = Cast<UUpgradePickerWidget>(WidgetComp->GetUserWidgetObject()))
		{
			UBaseUpgrade* upgrade1 = UpgradeOption1->GetDefaultObject<UBaseUpgrade>();
			UBaseUpgrade* upgrade2 = UpgradeOption2->GetDefaultObject<UBaseUpgrade>();
			Widget->SetLeftText(FText::FromString(upgrade1->GetTitle()),FText::FromString(upgrade1->GetDescription()));
			Widget->SetRightText(FText::FromString(upgrade2->GetTitle()),FText::FromString(upgrade2->GetDescription()));
		}
		WidgetComp->SetVisibility(true);
		PilonState = PilonState::Active;
		break;
		case PilonState::Active:
			if (LeftShape && RightShape)
			{
				TArray<AActor*> OverlappingActors;
				LeftShape->GetOverlappingActors(OverlappingActors, ALuhorCharacter::StaticClass());
				bool IsPlayerInLeft = OverlappingActors.Num() > 0;
				RightShape->GetOverlappingActors(OverlappingActors, ALuhorCharacter::StaticClass());
				bool IsPlayerInRight = OverlappingActors.Num() > 1;
				if (IsPlayerInLeft && IsPlayerInRight)
				{
					GEngine->AddOnScreenDebugMessage(
					-1,                   // key (-1 = always add a new line)
					1.0f,                 // time in seconds
					FColor::Green,        // text color
					TEXT("player either not in one of the choice boxes or in both at once"));
				}else
				{
					if (!IsPlayerInLeft && IsPlayerInRight)
					{
						PlayerUpgradeComp->AddUpgrade(UpgradeOption2);					
					} else {
						
						PlayerUpgradeComp->AddUpgrade(UpgradeOption1);					
					}
					PilonState = PilonState::Used;
					WidgetComp->SetVisibility(false);
				}
			}
			break;
	}
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
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALuhorCharacter::StaticClass(), FoundActors);
	PlayerUpgradeComp = Cast<UUpgradesComponent>(Cast<ALuhorCharacter>(FoundActors[0])->GetComponentByClass(UUpgradesComponent::StaticClass()));
}

void AUpgradePilon::RefreshUpgradeOptions()
{
	if (!PlayerUpgradeComp) return;

	// Build a set of classes the player already owns

	TArray<TSubclassOf<UBaseUpgrade>> PlayerUpgrades{PlayerUpgradeComp->GetUpgrades()};
	for (int j=Upgrades.Num()-1; j >= 0; j--)
	{
		for (int i = 0; i < PlayerUpgrades.Num(); i++)
		{
			if (Upgrades[j] == PlayerUpgrades[i])
			{
				Upgrades.RemoveAt(j);
				
			}
		}
	}
	UE_LOG(LogTemp, Log , TEXT("%i"), Upgrades.Num());
}

// Called every frame
void AUpgradePilon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


