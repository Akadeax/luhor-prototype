// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UpgradesComponent.h"
#include "LuhorPrototype/Util/FDebugUtil.h"

/**
 *
 */
class LUHORPROTOTYPE_API BaseUpgrade
{
public:
	BaseUpgrade();
	~BaseUpgrade();
	
	StatModifier GetStatModifier() const {return StatModifier();}
	void SetUpgradesComponent(UUpgradesComponent* Component){UpgradesComponent = Component;}
protected:
	StatModifier Modifier{};
	UUpgradesComponent* UpgradesComponent{nullptr};
	
	virtual void Trigger()
	{
		FDebugUtil::QuitCheckf(UpgradesComponent, TEXT("Upgrade doesn't have a pointer to the players upgradesComponent"));
		UpgradesComponent->SetChangedFlag();
	}
};
