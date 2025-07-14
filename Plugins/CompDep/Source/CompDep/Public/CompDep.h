#pragma once

#include "Modules/ModuleManager.h"

class FCompDepModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TArray<FName> RegisteredClassLayoutNames{};
};
