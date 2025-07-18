#pragma once

class FCompDepEditorModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void InitializeMenu();
	
	void RegisterMenus();
	void PopulateSubMenu(UToolMenu* Menu);
	void OpenDependencyViewer(); 
	
	void InitializeReloadHooks();
	FDelegateHandle BlueprintPreCompileDelegateHandle;
};
