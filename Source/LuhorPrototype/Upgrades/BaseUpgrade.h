// BaseUpgrade.h
#pragma once

#include "CoreMinimal.h"
#include "GameDelegates.h"
#include "UObject/Object.h"
#include "UpgradesComponent.h"
#include "LuhorPrototype/LuhorCharacter.h"
#include "LuhorPrototype/LuhorPlayerCharacter.h"
#include "LuhorPrototype/Util/FDebugUtil.h"
#include "Navigation/PathFollowingComponent.h"
#include "BaseUpgrade.generated.h"

UCLASS(BlueprintType, Blueprintable)
class LUHORPROTOTYPE_API UBaseUpgrade : public UObject
{
	GENERATED_BODY()

public:
	UBaseUpgrade() = default;
	virtual ~UBaseUpgrade() override = default;
	virtual void Init (){}
	virtual void DeInit(){}
	virtual void Tick(float DeltaTime){};

	UFUNCTION(BlueprintCallable)
	void SetStatModifier(const FStatModifier& Mod) { Modifier = Mod; }

	UFUNCTION(BlueprintPure)
	FStatModifier GetStatModifier() const { return IsUpgradeActive ? Modifier : FStatModifier{}; }

	FString GetTitle() const { return UpgradeName; }
	FString GetDescription() const { return UpgradeText; }
	UFUNCTION(BlueprintCallable)
	void SetUpgradesComponent(UUpgradesComponent* Component);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsUpgradeActive{ true };
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FStatModifier Modifier{};
	
	UPROPERTY(BlueprintReadOnly) ALuhorPlayerCharacter* PlayerCharacter{ nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeText{TEXT("Upgrade does stuff")};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UpgradeName{TEXT("Upgrade")};
	
	UPROPERTY(BlueprintReadOnly)
	UUpgradesComponent* UpgradesComponent{ nullptr };
};

