#pragma once

#include "IDetailCustomization.h"

class FDependencyDetailCustomization final : public IDetailCustomization
{
public:
	static void ReloadCustomizations();
	static void RegisterCustomizations();
	static void UnregisterCustomizations();
	
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FDependencyDetailCustomization>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
	
	static TArray<FName> RegisteredClassLayoutNames;
};
