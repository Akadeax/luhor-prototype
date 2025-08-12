// Interactable.h
#pragma once
#include "UObject/Interface.h"
#include "Interactable.generated.h"

UINTERFACE(BlueprintType)
class LUHORPROTOTYPE_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

class LUHORPROTOTYPE_API IInteractable
{
	GENERATED_BODY()

public:
	/** Can the interactor use this right now? */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interact")
	bool CanInteract(AActor* Interactor) const;

	/** Perform the interaction */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interact")
	void Interact(AActor* Interactor);

	/** What prompt should we show (“Open”, “Pick up”, …) */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interact")
	FText GetInteractPrompt(const AActor* Interactor) const;
};
