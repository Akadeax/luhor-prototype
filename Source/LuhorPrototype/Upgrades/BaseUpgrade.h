// BaseUpgrade.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UpgradesComponent.h"
#include "LuhorPrototype/LuhorCharacter.h"
#include "LuhorPrototype/LuhorPlayerCharacter.h"
#include "LuhorPrototype/Util/FDebugUtil.h"
#include "BaseUpgrade.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LUHORPROTOTYPE_API UBaseUpgrade : public UObject
{
	GENERATED_BODY()

public:
	UBaseUpgrade() = default;
	virtual ~UBaseUpgrade() override = default;
	virtual void Init (){}

	UFUNCTION(BlueprintCallable)
	void SetStatModifier(const FStatModifier& Mod) { Modifier = Mod; }

	UFUNCTION(BlueprintPure)
	FStatModifier GetStatModifier() const { return IsUpgradeActive ? Modifier : FStatModifier{}; }

	UFUNCTION(BlueprintCallable)
	void SetUpgradesComponent(UUpgradesComponent* Component)
	{
		UpgradesComponent = Component;
		PlayerCharacter = Cast<ALuhorPlayerCharacter>(UpgradesComponent->GetOwner());
		Init();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsUpgradeActive{ true };
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStatModifier Modifier{};
	
	ALuhorPlayerCharacter* PlayerCharacter{ nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeText{TEXT("Upgrade does stuff")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeName{TEXT("Upgrade")};

	UPROPERTY() 
	UUpgradesComponent* UpgradesComponent{ nullptr };
};

