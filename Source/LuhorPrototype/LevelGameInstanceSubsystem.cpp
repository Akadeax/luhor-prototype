// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGameInstanceSubsystem.h"

#include "AmbrosiaHealthComponent.h"
#include "Kismet/GameplayStatics.h"

void ULevelGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RoomData = LoadObject<URoomData>(nullptr, TEXT("/Game/Room/RoomData.RoomData"));
	check(RoomData);
}

void ULevelGameInstanceSubsystem::LoadRandomLevel()
{
	SavePlayerData();
	
	if (RoomsLeft.Num() == 0) RefillRoomsLeft();
	
	const int index{ FMath::RandRange(0, RoomsLeft.Num() - 1) };
	UGameplayStatics::OpenLevel(GetWorld(), RoomsLeft[index].Level.GetLongPackageFName());

	RoomsLeft.RemoveAt(index);
}

void ULevelGameInstanceSubsystem::RefillRoomsLeft()
{
	RoomsLeft = RoomData->Rooms;
}

void ULevelGameInstanceSubsystem::SavePlayerData()
{
	const APlayerController* controller{ UGameplayStatics::GetPlayerController(this, 0) };
	const UAmbrosiaHealthComponent* comp{ controller->GetPawn()->FindComponentByClass<UAmbrosiaHealthComponent>() };
	PlayerSaveData = {
		comp->GetCurrentHealth(),
		comp->GetCurrentPoisonedAmbrosia()
	};
}
