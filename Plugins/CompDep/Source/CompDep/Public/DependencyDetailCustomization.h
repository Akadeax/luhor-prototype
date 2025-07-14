#pragma once

#include "ComponentDependencies.h"
#include "IDetailCustomization.h"

class FDependencyDetailCustomization final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FDependencyDetailCustomization>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
	TPair<bool, FString> CheckDependencyFulfilled(const UActorComponent* SourceComponent, IComponentDependencies::Dependency Dependency);
};
