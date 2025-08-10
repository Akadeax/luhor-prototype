// PinVarSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "PinVarSubsystem.generated.h"

// Simple POD used in maps; JSON persistence is manual (no reflection needed)
struct FPinnedVariable
{
    FPinnedVariable() = default;
    FPinnedVariable(FName InVar, FName InGroup, FName InComp = NAME_None)
        : VariableName(InVar), GroupName(InGroup), ComponentTemplateName(InComp) {}

    FName VariableName{ NAME_None };
    FName GroupName{ NAME_None };
    FName ComponentTemplateName{ NAME_None }; // optional: component template on CDO
};

UCLASS()
class UPinVarSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

public:
    // UEditorSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // In‑memory state for the panel (mirrors staged)
    TMap<FName, TArray<FPinnedVariable>> PinnedGroups;

    // Source of truth (what’s saved to disk)
    TMap<FName, TArray<FPinnedVariable>> StagedPinnedGroups;

    // Add to the live view (not persisted)
    void PinVariable(FName ClassName, FName VariableName, FName GroupName, FName ComponentTemplateName = NAME_None);

    // Add/remove to the staged set (and persist)
    void StagePinVariable(FName ClassName, FName VariableName, FName GroupName, FName ComponentTemplateName = NAME_None);
    bool UnstagePinVariable(FName ClassName, FName VariableName, FName GroupName, FName ComponentTemplateName = NAME_None);

    // Read staged
    void GetAllStaged(TArray<TTuple<FName,FName,FName>>& OutTriples) const;
    void GetAllStagedWithComp(TArray<TTuple<FName,FName,FName,FName>>& OutQuads) const;

    // Sync staged -> pinned for display
    void MergeStagedIntoPinned();

    // Query
    const TArray<FPinnedVariable>* GetPinnedVariables(FName ClassName) const;

    // Persistence
    bool    SaveToDisk() const;
    bool    LoadFromDisk();
    FString GetPinsFilePath() const;
};

