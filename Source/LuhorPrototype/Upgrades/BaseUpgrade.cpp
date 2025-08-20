#include "BaseUpgrade.h"

void UBaseUpgrade::SetUpgradesComponent(UUpgradesComponent* Component)
{
	UpgradesComponent = Component;
	AActor* Actor = UpgradesComponent->GetOwner();
	if (Actor != nullptr)
	{
		PlayerCharacter = Cast<ALuhorPlayerCharacter>(Actor);
		Init();
	}
}
