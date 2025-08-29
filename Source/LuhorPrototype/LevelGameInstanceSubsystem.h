// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerSaveData.h"
#include "RoomData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelGameInstanceSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LUHORPROTOTYPE_API ULevelGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void LoadRandomLevel();
	
	UPROPERTY(BlueprintReadOnly) FPlayerSaveData PlayerSaveData;
	UFUNCTION(BlueprintCallable)
	void RefillRoomsLeft();
	UFUNCTION(BlueprintCallable)
	int GetRoomsCompleted(){return RoomsCompleted;}
private:
	void SavePlayerData();
	
	UPROPERTY() TObjectPtr<URoomData> RoomData;
	TArray<FRoom> RoomsLeft;
	int RoomsCompleted{ 0 };
};
