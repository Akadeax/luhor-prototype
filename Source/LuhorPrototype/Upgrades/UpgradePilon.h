// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
