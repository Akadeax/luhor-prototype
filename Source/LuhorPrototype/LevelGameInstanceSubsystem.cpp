// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGameInstanceSubsystem.h"

#include "Kismet/GameplayStatics.h"

void ULevelGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	RoomData = LoadObject<URoomData>(nullptr, TEXT("/Game/Room/RoomData.RoomData"));
	check(RoomData);
}

void ULevelGameInstanceSubsystem::LoadRandomLevel()
{
	const int index{ FMath::RandRange(0, RoomData->Rooms.Num() - 1) };
	UGameplayStatics::OpenLevel(GetWorld(), RoomData->Rooms[index].Level.GetLongPackageFName());
}
