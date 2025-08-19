// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseUpgrade.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "LuhorPrototype/Util/Interactable.h"
#include "UpgradePilon.generated.h"

UCLASS()
class LUHORPROTOTYPE_API AUpgradePilon : public AActor , public IInteractable
{
	GENERATED_BODY()
	

public:
	// Sets default values for this actor's properties
	AUpgradePilon();

	UFUNCTION(BlueprintCallable, Category="Interact")
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	UFUNCTION(BlueprintCallable, Category="Interact")
	virtual void Interact_Implementation(AActor* Interactor) override;
	UFUNCTION(BlueprintCallable, Category="Interact")
	virtual FText GetInteractPrompt_Implementation(const AActor* Interactor) const override;
	UFUNCTION(BlueprintImplementableEvent, Category="Interact")
	void InteractCalled();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	UWidgetComponent* WidgetComp{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	UShapeComponent* LeftShape{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact")
	UShapeComponent* RightShape{};
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UBaseUpgrade>> Upgrades{};
	UPROPERTY()
	UUpgradesComponent* PlayerUpgradeComp{};
	enum class PilonState
	{
		Inactive,
		Active,
		Used
	};
	PilonState PilonState{PilonState::Inactive};

	UPROPERTY()
	TSubclassOf<UBaseUpgrade> UpgradeOption1;
	UPROPERTY()
	TSubclassOf<UBaseUpgrade> UpgradeOption2;
	
	virtual void BeginPlay() override;
	void RefreshUpgradeOptions();
		

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
