// SPinVarPanel.cpp
#include "SPinVarPanel.h"

#include "ISinglePropertyView.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "SSearchableComboBox.h"
#include "Widgets/Input/SSuggestionTextBox.h"

#include "Widgets/Input/SSegmentedControl.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

#include "PinVarSubsystem.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Kismet2/KismetEditorUtilities.h"

#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/ActorComponent.h"
#include "UObject/UObjectIterator.h"
#include "Blueprint/BlueprintSupport.h"
#include "Engine/BlueprintGeneratedClass.h"

static UObject* FindComponentTemplate(UClass* Class, FName TemplateName)
{
	if (!Class || TemplateName.IsNone()) return nullptr;

	const FName Alt(*(TemplateName.ToString() + TEXT("_GEN_VARIABLE")));

	// Walk this class and all supers
	for (UClass* C = Class; C; C = C->GetSuperClass())
	{
		if (UObject* CDO = C->GetDefaultObject(true))
		{
			if (UObject* T = CDO->GetDefaultSubobjectByName(TemplateName)) return T;
			if (UObject* T2 = CDO->GetDefaultSubobjectByName(Alt))         return T2;

			TArray<UObject*> Subs;
			GetObjectsWithOuter(CDO, Subs, true);
			for (UObject* O : Subs)
			{
				if (!O) continue;
				const FName N = O->GetFName();
				if (N == TemplateName || N == Alt)
					return O;
			}
		}
	}

	// Last‑ditch global sweep (editor only): archetype components under any CDO in the chain
	for (TObjectIterator<UActorComponent> It; It; ++It)
	{
		UActorComponent* Comp = *It;
		if (!Comp || !Comp->HasAnyFlags(RF_ArchetypeObject)) continue;

		// Check whether this component ultimately lives under one of our class CDOs
		UObject* Outer = Comp->GetOuter();
		while (Outer && !Outer->IsA<UClass>())
		{
			if (Outer->GetFName() == TemplateName || Outer->GetFName() == Alt) return Comp;
			Outer = Outer->GetOuter();
		}
	}
	return nullptr;
}

static void BuildComponentOptions(UBlueprint* BP, UClass* Class, TArray<TSharedPtr<SPinVarPanel::FCompOption>>& Out)
{
	Out.Reset();
	if (!Class) return;

	TSet<FName> Seen;

	// Pass 1: collect all component templates from CDOs up the chain
	for (UClass* C = Class; C; C = C->GetSuperClass())
	{
		if (UObject* CDO = C->GetDefaultObject(true))
		{
			TArray<UObject*> Subs;
			GetObjectsWithOuter(CDO, Subs, false);

			for (UObject* O : Subs)
			{
				UActorComponent* Comp = Cast<UActorComponent>(O);
				if (!Comp) continue;

				const FName TmplName = Comp->GetFName();
				if (Seen.Contains(TmplName)) continue;

				Seen.Add(TmplName);

				auto Opt = MakeShared<SPinVarPanel::FCompOption>();
				Opt->Label        = TmplName;   // provisional label
				Opt->TemplateName = TmplName;   // stable key
				Opt->Template     = Comp;       // keep the actual template (weak)
				Out.Add(Opt);
			}
		}
	}

	// Small helper: upsert from an SCS node
	auto UpsertFromNode = [&Out, &Seen](UClass* OwningClass, USCS_Node* Node)
	{
		if (!Node || !OwningClass) return;

		UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(OwningClass);
		UActorComponent* ActualTemplate = BPGC ? Node->GetActualComponentTemplate(BPGC) : nullptr;
		if (!ActualTemplate)
		{
			ActualTemplate = Node->ComponentTemplate;
		}

		const FName Pretty = Node->GetVariableName();
		FName TemplateKey = ActualTemplate ? ActualTemplate->GetFName()
		                                   : FName(*(Pretty.ToString() + TEXT("_GEN_VARIABLE")));

		// If exists, improve label and fill template
		for (auto& Opt : Out)
		{
			if (Opt->TemplateName == TemplateKey)
			{
				Opt->Label = Pretty;
				if (ActualTemplate) { Opt->Template = ActualTemplate; }
				return;
			}
		}

		// Otherwise add it now
		if (!Seen.Contains(TemplateKey))
		{
			Seen.Add(TemplateKey);
			auto Opt = MakeShared<SPinVarPanel::FCompOption>();
			Opt->Label        = Pretty;
			Opt->TemplateName = TemplateKey;
			Opt->Template     = ActualTemplate; // may be null (rare)
			Out.Add(Opt);
		}
	};

	// Pass 2: walk BPGCs and refine labels/templates via SCS
	for (UClass* C = Class; C; C = C->GetSuperClass())
	{
		if (UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(C))
		{
			if (UBlueprint* OwnerBP = Cast<UBlueprint>(BPGC->ClassGeneratedBy))
			{
				if (USimpleConstructionScript* SCS = OwnerBP->SimpleConstructionScript)
				{
					for (USCS_Node* Node : SCS->GetAllNodes())
					{
						UpsertFromNode(C, Node);
					}
				}
			}
		}
	}

	Out.StableSort([](const TSharedPtr<SPinVarPanel::FCompOption>& A,
	                  const TSharedPtr<SPinVarPanel::FCompOption>& B)
	{
		return A->Label.LexicalLess(B->Label);
	});
}

static FString PrettyBlueprintDisplayName(const UClass* Cls)
{
	if (!Cls) return TEXT("");
	FString N = Cls->GetName();
	N.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);
	return N;
}


void SPinVarPanel::Construct(const FArguments& InArgs)
{
	OnRefreshRequested = InArgs._OnRefreshRequested;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar row
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(SHorizontalBox)

			// Refresh
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton")
				.ToolTipText(FText::FromString(TEXT("Refresh pinned variables list")))
				.OnClicked_Lambda([this]()
				{
					Refresh();
					return FReply::Handled();
				})
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Refresh")))
				]
			]

			// Add Variable (BP / Parent C++ / Component)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton")
				.OnClicked(this, &SPinVarPanel::OnAddBlueprintVariableClicked)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Add Variable")))
				]
			]
		]

		// Scrollable list
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(RootBox, SVerticalBox)
			]
		]
	];

	Refresh();
}

void SPinVarPanel::Refresh()
{
	if (OnRefreshRequested.IsBound())
	{
		OnRefreshRequested.Execute();
	}

	if (GEditor)
	{
		if (UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>())
		{
			Subsystem->RepopulateSessionCacheAll();
		}
	}

	Grouped.Reset();
	Rebuild();
}



void SPinVarPanel::Rebuild()
{
	for (auto& Pair : GroupAreaWidgets)
	{
		const FName GroupName = Pair.Key;
		if (TSharedPtr<SExpandableArea> Area = Pair.Value.Pin())
		{
			GroupExpandedState.Add(GroupName, Area->IsExpanded());
		}
	}
	GroupAreaWidgets.Empty();

	RootBox->ClearChildren();
	GatherPinnedProperties();

	if (Grouped.Num() == 0)
	{
		RootBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("No pinned variables yet. Use “Add Variable” to stage entries.")))
		];

		GroupExpandedState.Empty();
		return;
	}

	for (auto& KV : Grouped)
	{
		const FName Group = KV.Key;
		TArray<FEntry>& Entries = KV.Value;

		TSharedRef<SVerticalBox> ListVB = SNew(SVerticalBox);

		// Create the group area (collapsed by default), then apply remembered state
		TSharedRef<SExpandableArea> Area =
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()[ SNew(STextBlock).Text(FText::FromName(Group)) ]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SSeparator) ]
				+ SVerticalBox::Slot().AutoHeight()[ ListVB ]
			];

		// Restore expansion if we remembered it
		const bool* Remembered = GroupExpandedState.Find(Group);
		if (Remembered && *Remembered)
		{
			Area->SetExpanded(true);
		}

		// Track this area so we can snapshot its state next rebuild
		GroupAreaWidgets.Add(Group, Area);

		RootBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 4.f)
		[
			Area
		];

		// Populate the body with the class sections we already assembled
		for (const FEntry& E : Entries)
		{
			ListVB->AddSlot()
			.AutoHeight()
			.Padding(6.f, 6.f)
			[
				E.Widget
			];
		}
	}
}

bool SPinVarPanel::IsSkelOrReinst(const UClass* C)
{
	if (!C) return false;
	const FString N = C->GetName();
	return N.StartsWith(TEXT("SKEL_")) || N.StartsWith(TEXT("REINST_"));
}

bool SPinVarPanel::IsEditableProperty(const FProperty* P)
{
	if (!P) return false;

	// Editable on class defaults if it has CPF_Edit (covers EditAnywhere + EditDefaultsOnly + EditInstanceOnly)
	const bool bHasEdit = P->HasAnyPropertyFlags(CPF_Edit);

	// Hide if read-only in editor or not editable on templates (CDOs)
	const bool bReadOnlyInEditor  = P->HasAnyPropertyFlags(CPF_EditConst);
	const bool bHiddenOnTemplates = P->HasAnyPropertyFlags(CPF_DisableEditOnTemplate);

	// We don't expose transient or delegate properties
	const bool bTransient = P->HasAnyPropertyFlags(CPF_Transient);
	const bool bIsDelegate =
		P->IsA(FMulticastDelegateProperty::StaticClass()) ||
		P->IsA(FDelegateProperty::StaticClass());

	return bHasEdit && !bReadOnlyInEditor && !bHiddenOnTemplates && !bTransient && !bIsDelegate;
}

void SPinVarPanel::GetAllGroups(TSharedRef<FState> S)
{
	TSet<FString> Unique;

	if (GEditor)
	{
		if (UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>())
		{
			// Pull from pinned mirror
			for (const auto& Pair : Subsystem->PinnedGroups)                          // Map<FName ClassName, TArray<FPinnedVariable>>
			{
				for (const FPinnedVariable& E : Pair.Value)
				{
					if (!E.GroupName.IsNone())
					{
						Unique.Add(E.GroupName.ToString());
					}
				}
			}

			// (Optional) also include staged-only entries, if any
			for (const auto& Pair : Subsystem->StagedPinnedGroups)
			{
				for (const FPinnedVariable& E : Pair.Value)
				{
					if (!E.GroupName.IsNone())
					{
						Unique.Add(E.GroupName.ToString());
					}
				}
			}
		}
	}

	S->AllGroups.Reserve(Unique.Num());
	for (const FString& G : Unique) { S->AllGroups.Add(G); }
	S->AllGroups.Sort();
}

void SPinVarPanel::GatherPinnedProperties()
{
	if (!GEditor) return;
	UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>();
	if (!Subsystem) return;

	FPropertyEditorModule& PropEd = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// ------- Buckets: Group -> Class -> {BP, Native, Components} -------
	struct FClassBuckets
	{
		FName   ClassName;
		FText   ClassLabel;

		TArray<FName> BPVars;
		TArray<FName> NativeVars;

		TMap<FName, TArray<FName>> ComponentVarsByName;          // CompLabel -> Var list
		TMap<FName, TWeakObjectPtr<UObject>> ComponentTemplates; // CompLabel -> template obj
	};

	TMap<FName, TMap<FName, FClassBuckets>> Build; // Group -> (ClassName -> Buckets)

	// Helpers to classify where a property was declared
	auto IsBPDeclared = [](const FProperty* P)
	{
		const UClass* OwnerClass = Cast<UClass>(P ? P->GetOwnerStruct() : nullptr);
		return (OwnerClass && OwnerClass->ClassGeneratedBy != nullptr);
	};
	auto IsNativeDeclared = [](const FProperty* P)
	{
		const UClass* OwnerClass = Cast<UClass>(P ? P->GetOwnerStruct() : nullptr);
		return (OwnerClass && OwnerClass->ClassGeneratedBy == nullptr);
	};

	// -------- Collect --------
	for (const TPair<FName, TArray<FPinnedVariable>>& Pair : Subsystem->PinnedGroups)
	{
		const FName ClassName = Pair.Key;
		UClass* Cls = FindFirstObjectSafe<UClass>(*ClassName.ToString());
		if (!Cls || IsSkelOrReinst(Cls)) continue;

		UObject* CDO = Cls->GetDefaultObject(true);
		if (!CDO) continue;

		const FName ClassFName = Cls->GetFName();
		const FText ClassLabel = FText::FromString(PrettyBlueprintDisplayName(Cls)); // display-only prettified name. 

		for (const FPinnedVariable& Pinned : Pair.Value)
		{
			// Resolve target object (class defaults or component template)
			UObject* TargetObj = CDO;
			const bool bIsComponent = !Pinned.ComponentTemplateName.IsNone();
			if (bIsComponent)
			{
				TargetObj = Pinned.ResolvedTemplate.IsValid()
					? Pinned.ResolvedTemplate.Get()
					: FindComponentTemplate(Cls, Pinned.ComponentTemplateName);

				if (!TargetObj)
				{
					UE_LOG(LogTemp, Warning, TEXT("PinVar.Panel: component template '%s' not resolved on %s"),
						*Pinned.ComponentTemplateName.ToString(), *CDO->GetName());
					continue;
				}
			}

			// Check property exists and is editable
			FProperty* Found = FindFProperty<FProperty>(TargetObj->GetClass(), Pinned.VariableName);
			if (!Found || !IsEditableProperty(Found)) continue;  // uses your IsEditableProperty() 

			// Groups can be split by ','
			TArray<FString> Tokens;
			const FString GroupStr = Pinned.GroupName.ToString();
			GroupStr.ParseIntoArray(Tokens, TEXT(","), /*CullEmpty*/true);
			if (Tokens.Num() == 0) Tokens.Add(GroupStr);

			for (const FString& Tok : Tokens)
			{
				const FName GroupName(*Tok.TrimStartAndEnd());
				if (GroupName.IsNone()) continue;

				FClassBuckets& B = Build.FindOrAdd(GroupName).FindOrAdd(ClassFName);
				B.ClassName  = ClassFName;
				B.ClassLabel = ClassLabel;

				if (!bIsComponent)
				{
					// Class defaults: BP vs Native by owner
					if (IsBPDeclared(Found))          { B.BPVars.Add(Pinned.VariableName); }
					else if (IsNativeDeclared(Found)) { B.NativeVars.Add(Pinned.VariableName); }
					else                               { B.BPVars.Add(Pinned.VariableName); } // fallback
				}
				else
				{
					// Component: show pretty label if present, else template name
					const FName CompLabel = !Pinned.ComponentVariablePrettyName.IsNone()
						? Pinned.ComponentVariablePrettyName
						: Pinned.ComponentTemplateName;

					B.ComponentVarsByName.FindOrAdd(CompLabel).Add(Pinned.VariableName);
					if (!B.ComponentTemplates.Contains(CompLabel))
					{
						B.ComponentTemplates.Add(CompLabel, TargetObj);
					}
				}
			}
		}
	}

	// -------- Emit UI into Grouped (ONE widget per class) --------
	for (auto& GroupKV : Build)
	{
		const FName Group = GroupKV.Key;
		TMap<FName, FClassBuckets>& Classes = GroupKV.Value;

		// Sort classes by their label (case-insensitive)
		TArray<FName> ClassOrder;
		Classes.GenerateKeyArray(ClassOrder);
		ClassOrder.Sort([&Classes](const FName& A, const FName& B)
		{
			return Classes[A].ClassLabel.ToString().Compare(
				Classes[B].ClassLabel.ToString(), ESearchCase::IgnoreCase) < 0;
		});

		for (const FName& CN : ClassOrder)
		{
			FClassBuckets& B = Classes[CN];

			// Sort buckets
			B.BPVars.Sort(FNameLexicalLess());
			B.NativeVars.Sort(FNameLexicalLess());

			TArray<FName> CompNames;
			B.ComponentVarsByName.GenerateKeyArray(CompNames);
			CompNames.Sort(FNameLexicalLess());
			for (auto& CKV : B.ComponentVarsByName) { CKV.Value.Sort(FNameLexicalLess()); }

			// Build a single widget for this class section
			TSharedRef<SVerticalBox> ClassVB = SNew(SVerticalBox);

			// Class header (bigger + bold)
			ClassVB->AddSlot().AutoHeight().Padding(6.f, 8.f, 6.f, 4.f)
			[
				SNew(STextBlock)
				.Text(B.ClassLabel)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			];

			// Helpers: plain prop widget + prop widget with Delete button (no lambdas)
			auto EmitPropOnly = [&](UObject* Target, const FName Var) -> TSharedRef<SWidget>
			{
				FSinglePropertyParams Params;
				TSharedPtr<ISinglePropertyView> View = PropEd.CreateSingleProperty(Target, Var, Params);
				return View.IsValid()
					? StaticCastSharedRef<SWidget>(View.ToSharedRef())
					: SNew(STextBlock).Text(FText::FromString(Var.ToString()));
			};

			auto EmitPropWithDelete = [&](UObject* Target,
			                              const FName Var,
			                              const FName GroupName,
			                              const FName ClassName,
			                              const FName CompNameForRemoval) -> TSharedRef<SWidget>
			{
				return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				[
					EmitPropOnly(Target, Var)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(6.f, 0.f, 0.f, 0.f)
				[
					SNew(SButton)
					.ButtonStyle(FAppStyle::Get(), "FlatButton")
					.ContentPadding(FMargin(4.f, 2.f))
					.ToolTipText(FText::FromString(TEXT("Remove this variable from the list")))
					.OnClicked(this, &SPinVarPanel::OnRemovePinned, ClassName, Var, GroupName, CompNameForRemoval)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("X")))
						.ColorAndOpacity(FLinearColor::Red)
					]
				];
			};

			// Class defaults (BP then Native)
			if (B.BPVars.Num() || B.NativeVars.Num())
			{
				if (UClass* ClsForCDO = FindFirstObjectSafe<UClass>(*B.ClassName.ToString()))
				{
					if (UObject* CDO = ClsForCDO->GetDefaultObject(true))
					{
						for (const FName& Var : B.BPVars)
						{
							ClassVB->AddSlot().AutoHeight().Padding(16.f, 2.f)
							[ EmitPropWithDelete(CDO, Var, Group, B.ClassName, NAME_None) ];
						}
						for (const FName& Var : B.NativeVars)
						{
							ClassVB->AddSlot().AutoHeight().Padding(16.f, 2.f)
							[ EmitPropWithDelete(CDO, Var, Group, B.ClassName, NAME_None) ];
						}
					}
				}
			}

			// Components
			for (const FName& CompLabel : CompNames)
			{
				ClassVB->AddSlot().AutoHeight().Padding(10.f, 8.f, 6.f, 2.f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("Component: %s"), *CompLabel.ToString())))
					.ColorAndOpacity(FLinearColor(0.8f,0.8f,0.8f,1))
				];

				UObject* Tmpl = B.ComponentTemplates.FindRef(CompLabel).Get();
				if (!Tmpl) continue;

				const FName CompNameForRemoval = Tmpl->GetFName(); // remove by template key

				for (const FName& Var : B.ComponentVarsByName[CompLabel])
				{
					ClassVB->AddSlot().AutoHeight().Padding(16.f, 2.f)
					[
						EmitPropWithDelete(Tmpl, Var, Group, B.ClassName, CompNameForRemoval)
					];
				}
			}

			// One entry per class for this group
			Grouped.FindOrAdd(Group).Add({ Group, ClassVB });
		}
	}
}


FReply SPinVarPanel::OnRemovePinned(FName ClassName, FName VarName, FName GroupName, FName CompName)
{
	if (GEditor)
	{
		if (UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>())
		{
			Subsystem->UnstagePinVariable(ClassName, VarName, GroupName, CompName);
			Refresh(); // rebuild UI
		}
	}
	return FReply::Handled();
}


FReply SPinVarPanel::OnAddBlueprintVariableClicked()
{
	if (TSharedPtr<SWindow> W = SelectBlueprintWindow.Pin()) { W->RequestDestroyWindow(); }
	if (TSharedPtr<SWindow> W = AddVariableWindow.Pin())     { W->RequestDestroyWindow(); }
	FAssetPickerConfig PickerConfig;
	PickerConfig.Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	PickerConfig.SelectionMode = ESelectionMode::Single;
	PickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SPinVarPanel::OnBlueprintPicked);

	TSharedRef<SWindow> PickerWindow = SNew(SWindow)
		.Title(FText::FromString("Select Blueprint"))
		.ClientSize(FVector2D(600, 400))
		[
			FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser")
			.Get()
			.CreateAssetPicker(PickerConfig)
		];

	SelectBlueprintWindow = PickerWindow;
	FSlateApplication::Get().AddWindow(PickerWindow);
	return FReply::Handled();
}

void SPinVarPanel::OnBlueprintPicked(const FAssetData& AssetData)
{
	if (TSharedPtr<SWindow> W = AddVariableWindow.Pin())     { W->RequestDestroyWindow(); }
	if (!AssetData.IsValid())
		return;

	UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset());
	if (!BP)
		return;

	ShowAddDialog(BP);
}


void SPinVarPanel::ShowAddDialog(UBlueprint* BP)
{
	if (!BP) return;

	// Ensure a generated class exists
	if (!BP->GeneratedClass)
	{
		FKismetEditorUtilities::CompileBlueprint(BP);
	}
	UClass* TargetClass = BP->GeneratedClass ? BP->GeneratedClass : BP->SkeletonGeneratedClass;
	if (!TargetClass)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Blueprint has no generated class yet.")));
		return;
	}
	TSharedRef<FState> S = MakeShared<FState>();
	S->BP    = BP;
	S->Class = TargetClass;

	GetAllGroups(S);
	// Local BP variables
	
	TArray<FName> TmpVar;
	GatherLocalVars(BP, TmpVar);
	for (const FName& N : TmpVar) S->LocalVarOpts.Add(MakeShared<FString>(N.ToString()));
	if (S->LocalVarOpts.Num()) S->LocalVarSel = S->LocalVarOpts[0];
	

	// Parent C++ properties
	
	TArray<FName> TmpProp;
	GatherNativeProps(TargetClass, TmpProp);
	for (const FName& N : TmpProp) S->NativePropOpts.Add(MakeShared<FString>(N.ToString()));
	if (S->NativePropOpts.Num()) S->NativePropSel = S->NativePropOpts[0];
	

	// Component options (CDO + SCS, each option carries a weak template ptr)
	BuildComponentOptions(BP, TargetClass, S->CompOpts);
	if (S->CompOpts.Num())
	{
		// Build a label list + map for searchable combo
		for (const auto& Opt : S->CompOpts)
		{
			const FString LabelStr = Opt->Label.ToString();
			S->CompOptLabels.Add(MakeShared<FString>(LabelStr));
			S->LabelToCompOpt.Add(LabelStr, Opt);
		}
		// Default select first
		S->CompSel = S->CompOpts[0];

		// Initial component property list (prefer weak ptr; fallback to FindComponentTemplate)
		if (S->CompSel.IsValid())
		{
			UActorComponent* Template =
				S->CompSel->Template.IsValid()
					? S->CompSel->Template.Get()
					: Cast<UActorComponent>(FindComponentTemplate(S->Class, S->CompSel->TemplateName));

			if (Template)
			{
				TArray<FName> Props;
				GatherComponentPropsByTemplate(Template, Props);
				for (const FName& N : Props) S->CompPropOpts.Add(MakeShared<FString>(N.ToString()));
				if (S->CompPropOpts.Num()) S->CompPropSel = S->CompPropOpts[0];
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PinVar: (init) Template not found for '%s' (key '%s') on %s"),
					*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), *S->Class->GetName());
			}
		}
	}

	// --- Build dialog UI ---

	TSharedRef<SWindow> Dialog = SNew(SWindow)
		.Title(FText::FromString(TEXT("Add Variable to Group")))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.ClientSize(FVector2D(520, 380));

	AddVariableWindow = Dialog;

	// Helper for searchable combos over FString arrays
	auto MakeStringCombo = [](TArray<TSharedPtr<FString>>& Source, TSharedPtr<FString>& Sel, TFunction<void(TSharedPtr<FString>)> OnSel)
	{
		return SNew(SSearchableComboBox)
			.OptionsSource(&Source)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> It)
			{
				return SNew(STextBlock).Text(FText::FromString(It.IsValid() ? *It : TEXT("None")));
			})
			.OnSelectionChanged_Lambda([&Sel, OnSel](TSharedPtr<FString> NewSel, ESelectInfo::Type)
			{
				Sel = NewSel;
				if (OnSel) OnSel(NewSel);
			})
			.InitiallySelectedItem(Sel)
			[
				SNew(STextBlock)
				.Text_Lambda([&Sel]()
				{
					return Sel.IsValid() ? FText::FromString(*Sel) : FText::FromString(TEXT("None"));
				})
			];
	};

	Dialog->SetContent(
		SNew(SVerticalBox)

		// Source segment
		+ SVerticalBox::Slot().AutoHeight().Padding(12,12,12,6)
		[
			SNew(SSegmentedControl<int32>)
			.Value_Lambda([S](){ return S->SourceIndex; })
			.OnValueChanged_Lambda([S](int32 NewIdx){ S->SourceIndex = NewIdx; })
			+ SSegmentedControl<int32>::Slot(0).Text(FText::FromString("Blueprint local"))
			+ SSegmentedControl<int32>::Slot(1).Text(FText::FromString("Parent C++"))
			+ SSegmentedControl<int32>::Slot(2).Text(FText::FromString("Component"))
		]

		// Local BP
		+ SVerticalBox::Slot().AutoHeight().Padding(12,6,12,4)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([S](){ return S->SourceIndex==0 ? EVisibility::Visible : EVisibility::Collapsed; })
			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString("Variable:")) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)
			[
				MakeStringCombo(S->LocalVarOpts, S->LocalVarSel, nullptr)
			]
		]

		// Parent C++
		+ SVerticalBox::Slot().AutoHeight().Padding(12,6,12,4)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([S](){ return S->SourceIndex==1 ? EVisibility::Visible : EVisibility::Collapsed; })
			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString("Property:")) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)
			[
				MakeStringCombo(S->NativePropOpts, S->NativePropSel, nullptr)
			]
		]

		// Component
		+ SVerticalBox::Slot().AutoHeight().Padding(12,6,12,4)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([S](){ return S->SourceIndex==2 ? EVisibility::Visible : EVisibility::Collapsed; })

			+ SVerticalBox::Slot().AutoHeight()[ SNew(STextBlock).Text(FText::FromString("Component:")) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,2,0,4)
			[
				SNew(SSearchableComboBox)
				.OptionsSource(&S->CompOptLabels)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> It)
				{
					return SNew(STextBlock).Text(FText::FromString(It.IsValid() ? *It : TEXT("None")));
				})
				.OnSelectionChanged_Lambda([S,this](TSharedPtr<FString> NewSel, ESelectInfo::Type)
				{
					S->CompSel.Reset();
					S->CompPropOpts.Reset();
					S->CompPropSel.Reset();

					if (!S->Class || !NewSel.IsValid()) { if (S->CompPropCombo.IsValid()) S->CompPropCombo->RefreshOptions(); return; }

					// Resolve label -> option
					if (TSharedPtr<FCompOption>* FoundPtr = S->LabelToCompOpt.Find(*NewSel))
					{
						S->CompSel = *FoundPtr;

						UActorComponent* Tmpl =
							S->CompSel->Template.IsValid()
								? S->CompSel->Template.Get()
								: Cast<UActorComponent>(FindComponentTemplate(S->Class, S->CompSel->TemplateName));

						if (Tmpl)
						{
							TArray<FName> P;
							GatherComponentPropsByTemplate(Tmpl, P);
							for (const FName& N : P) S->CompPropOpts.Add(MakeShared<FString>(N.ToString()));
							if (S->CompPropOpts.Num()) S->CompPropSel = S->CompPropOpts[0];
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("PinVar: Template not found for '%s' (key '%s') on %s"),
								*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), *S->Class->GetName());
						}
					}

					if (S->CompPropCombo.IsValid())
					{
						S->CompPropCombo->RefreshOptions();
						S->CompPropCombo->SetSelectedItem(S->CompPropSel);
					}
				})
				[
					SNew(STextBlock)
					.Text_Lambda([S]()
					{
						return S->CompSel.IsValid()
							? FText::FromName(S->CompSel->Label)
							: FText::FromString(TEXT("None"));
					})
				]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
			[
				SNew(STextBlock).Text(FText::FromString("Property:"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)
			[
				SAssignNew(S->CompPropCombo, SSearchableComboBox)
				.OptionsSource(&S->CompPropOpts)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> It)
				{
					return SNew(STextBlock).Text(FText::FromString(It.IsValid()? *It : TEXT("None")));
				})
				.OnSelectionChanged_Lambda([S](TSharedPtr<FString> NewSel, ESelectInfo::Type)
				{
					S->CompPropSel = NewSel;
				})
				.InitiallySelectedItem(S->CompPropSel)
				[
					SNew(STextBlock)
					.Text_Lambda([S]()
					{
						return S->CompPropSel.IsValid()? FText::FromString(*S->CompPropSel) : FText::FromString(TEXT("None"));
					})
				]
			]
		]

		// Group
		+ SVerticalBox::Slot().AutoHeight().Padding(12,10,12,8)
		[
			SNew(STextBlock).Text(FText::FromString("Group Name (A or A,B):"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(12,0,12,12)
		[
			SAssignNew(S->GroupSuggest, SSuggestionTextBox)
			.Text_Lambda([S]() { return FText::FromString(S->GroupStr); })
			.OnTextChanged_Lambda([S](const FText& T){ S->GroupStr = T.ToString(); })
			.OnTextCommitted_Lambda([S](const FText& T, ETextCommit::Type CommitType)
			{
				if (CommitType != ETextCommit::OnCleared) // <-- prevent blanking on focus loss
				{
					S->GroupStr = T.ToString();
				}
			})
			.OnShowingSuggestions_Lambda([S](const FString& Current, TArray<FString>& Out)
			{
				const int32 LastComma = Current.Find(TEXT(","), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				const FString Prefix = (LastComma != INDEX_NONE) ? Current.Left(LastComma + 1) : FString();
				const FString Needle = (LastComma != INDEX_NONE) ? Current.Mid(LastComma + 1).TrimStart() : Current;

				Out.Reset();
				for (const FString& G : S->AllGroups)
					if (Needle.IsEmpty() || G.StartsWith(Needle, ESearchCase::IgnoreCase))
						Out.Add(Prefix + G);
			})
			.OnShowingHistory_Lambda([S](TArray<FString>& OutHistory){ OutHistory = S->AllGroups; })

		]



		// Buttons
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(12,0,12,12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
				.Text(FText::FromString("Add"))
				.OnClicked_Lambda([this, S, Dialog]()
				{
					if (!GEditor) { Dialog->RequestDestroyWindow(); return FReply::Handled(); }
					if (!S->Class) { Dialog->RequestDestroyWindow(); return FReply::Handled(); }

					// Accept empty group -> "Default"
					FString GroupCsv = S->GroupStr.TrimStartAndEnd();
					if (GroupCsv.IsEmpty())
					{
						GroupCsv = TEXT("Default");
					}

					FName VarName = NAME_None;
					FName CompName = NAME_None;

					switch (S->SourceIndex)
					{
						case 0: // Blueprint local
							if (S->LocalVarSel.IsValid()) VarName = FName(**S->LocalVarSel);
							break;

						case 1: // Parent C++
							if (S->NativePropSel.IsValid()) VarName = FName(**S->NativePropSel);
							break;

						case 2: // Component
							if (S->CompSel.IsValid() && S->CompPropSel.IsValid())
							{
								// Resolve template: weak ptr from option, or fallback by name
								UActorComponent* Tmpl =
									S->CompSel->Template.IsValid()
										? S->CompSel->Template.Get()
										: Cast<UActorComponent>(FindComponentTemplate(S->Class, S->CompSel->TemplateName));

								// We'll persist the template name (use live one if we have it), and cache the object for this session
								const FName TemplateKey = Tmpl ? Tmpl->GetFName() : S->CompSel->TemplateName;
								const FName PrettyVar   = S->CompSel->Label; // SCS variable name (nice label)
								VarName = FName(**S->CompPropSel);

								if (UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>())
								{
									TArray<FString> Groups;
									GroupCsv.ParseIntoArray(Groups, TEXT(","), true);
									for (FString& G : Groups)
									{
										G = G.TrimStartAndEnd();
										if (G.IsEmpty()) continue;
										Subsystem->StagePinVariableWithTemplate(
											S->Class->GetFName(), VarName, FName(*G),
											TemplateKey, Tmpl, PrettyVar);
										GetAllGroups(S);
									}

									Subsystem->SaveToDisk();
									Subsystem->MergeStagedIntoPinned();
								}

								Refresh();
								return FReply::Handled();
							}
							break;
					}

					if (VarName.IsNone())
					{
						UE_LOG(LogTemp, Warning, TEXT("PinVar: Add aborted — no variable selected."));
						Dialog->RequestDestroyWindow();
						return FReply::Handled();
					}

					if (UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>())
					{
						TArray<FString> Groups;
						GroupCsv.ParseIntoArray(Groups, TEXT(","), /*CullEmpty*/true);
						for (FString& G : Groups)
						{
							G = G.TrimStartAndEnd();
							if (!G.IsEmpty())
							{
								Subsystem->StagePinVariable(S->Class->GetFName(), VarName, FName(*G), CompName);
								GetAllGroups(S);
							}
						}
						Subsystem->SaveToDisk();
					}

					Refresh();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Cancel"))
				.OnClicked_Lambda([Dialog]()
				{
					Dialog->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
		]
	);
	FSlateApplication::Get().AddWindow(Dialog);
	
}





void SPinVarPanel::GatherLocalVars(UBlueprint* BP, TArray<FName>& OutVars) const
{
	OutVars.Reset();
	if (!BP) return;
	for (const FBPVariableDescription& Var : BP->NewVariables)
	{
		if (!Var.VarName.IsNone())
		{
			OutVars.Add(Var.VarName);
		}
	}
	OutVars.Sort(FNameLexicalLess());
}

void SPinVarPanel::GatherNativeProps(UClass* Class, TArray<FName>& OutProps) const
{
	OutProps.Reset();
	if (!Class) return;

	for (TFieldIterator<FProperty> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* P = *It;
		if (!IsEditableProperty(P)) continue;
		
		UStruct* OwnerStruct = P->GetOwnerStruct();
		UClass*  OwnerClass  = Cast<UClass>(OwnerStruct);
		
		const bool bDeclaredOnNative = (OwnerClass && OwnerClass->ClassGeneratedBy == nullptr);
		const bool bDeclaredOnThisBP = (OwnerClass == Class);

		if (bDeclaredOnNative && !bDeclaredOnThisBP)
		{
			OutProps.Add(P->GetFName());
		}
	}

	OutProps.Sort(FNameLexicalLess());
}

void SPinVarPanel::GatherComponentPropsByTemplate(UObject* CompTemplate, TArray<FName>& OutProps) const
{
	OutProps.Reset();
	if (!CompTemplate) return;

	for (TFieldIterator<FProperty> It(CompTemplate->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		FProperty* P = *It;
		if (IsEditableProperty(P))
		{
			OutProps.Add(P->GetFName());
		}
	}
	OutProps.Sort(FNameLexicalLess());
}
