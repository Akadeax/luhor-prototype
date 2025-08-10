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
			// 1) Fast path: direct default subobjects
			if (UObject* T = CDO->GetDefaultSubobjectByName(TemplateName)) return T;
			if (UObject* T2 = CDO->GetDefaultSubobjectByName(Alt))         return T2;

			// 2) Full scan, including nested subobjects (some BP templates are nested)
			TArray<UObject*> Subs;
			GetObjectsWithOuter(CDO, Subs, /*bIncludeNestedObjects*/ true);
			for (UObject* O : Subs)
			{
				if (!O) continue;
				const FName N = O->GetFName();
				if (N == TemplateName || N == Alt)
					return O;
			}
		}
	}

	// 3) Last‑ditch global sweep (editor only): archetype components under any CDO in the chain
	for (TObjectIterator<UActorComponent> It; It; ++It)
	{
		UActorComponent* Comp = *It;
		if (!Comp || !Comp->HasAnyFlags(RF_ArchetypeObject)) continue;

		// Check whether this component ultimately lives under one of our class CDOs
		UObject* Outer = Comp->GetOuter();
		while (Outer && !Outer->IsA<UClass>())
		{
			if (Outer->IsA<UObject>() && Outer->GetFName() == TemplateName) return Comp;
			if (Outer->GetFName() == Alt) return Comp;
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

	for (UClass* C = Class; C; C = C->GetSuperClass())
	{
		if (UObject* CDO = C->GetDefaultObject(true))
		{
			TArray<UObject*> Subs;
			GetObjectsWithOuter(CDO, Subs, false);

			for (UObject* O : Subs)
			{
				if (!O || !O->IsA<UActorComponent>()) continue;

				const FName TmplName = O->GetFName();
				if (Seen.Contains(TmplName)) continue;

				Seen.Add(TmplName);

				auto Opt = MakeShared<SPinVarPanel::FCompOption>();
				Opt->Label        = TmplName;   // provisional: will be prettified by SCS pass
				Opt->TemplateName = TmplName;   // exact binding key
				Out.Add(Opt);
			}
		}
	}

	// Small helper to update/add from an SCS node
	auto UpsertFromNode = [&Out, &Seen](UClass* OwningClass, USCS_Node* Node)
	{
		if (!Node || !OwningClass) return;

		// Resolve the *actual* template for this node on its own BPGC
		UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(OwningClass);
		UActorComponent* ActualTemplate = nullptr;
		if (BPGC)
		{
			ActualTemplate = Node->GetActualComponentTemplate(BPGC);
		}
		if (!ActualTemplate)
		{
			ActualTemplate = Node->ComponentTemplate;
		}

		const FName Pretty = Node->GetVariableName();
		FName TemplateKey = NAME_None;

		if (ActualTemplate)
		{
			TemplateKey = ActualTemplate->GetFName();  // trust SCS, even if on a parent CDO
		}
		else
		{
			// Heuristic only if we truly can't resolve (rare)
			TemplateKey = FName(*(Pretty.ToString() + TEXT("_GEN_VARIABLE")));
		}

		// If the template already exists from the CDO pass, just improve the label.
		for (auto& Opt : Out)
		{
			if (Opt->TemplateName == TemplateKey)
			{
				Opt->Label = Pretty;
				return;
			}
		}

		// Otherwise add it now (covers design-time-only cases)
		if (!Seen.Contains(TemplateKey))
		{
			Seen.Add(TemplateKey);
			auto Opt = MakeShared<SPinVarPanel::FCompOption>();
			Opt->Label        = Pretty;
			Opt->TemplateName = TemplateKey;
			Out.Add(Opt);
		}
	};

	// ---- 2) Walk the *Blueprint chain*: for each BPGC in the class hierarchy,
	// use that Blueprint's SCS to prettify labels and catch unusual templates.
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

	UE_LOG(LogTemp, Verbose, TEXT("PinVar: Component options for %s"), *Class->GetName());
	for (const auto& Opt : Out)
	{
		UE_LOG(LogTemp, Verbose, TEXT("  Label='%s'  TemplateName='%s'"),
			*Opt->Label.ToString(), *Opt->TemplateName.ToString());
	}
}



// ----------------- widget -----------------

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

			// Remove
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f, 0.f)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "FlatButton")
				.OnClicked(this, &SPinVarPanel::OnRemoveBlueprintVariableClicked)
				[
					SNew(STextBlock).Text(FText::FromString(TEXT("Remove Variable")))
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
	Grouped.Reset();
	Rebuild();
}

void SPinVarPanel::Rebuild()
{
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
		return;
	}

	for (auto& KV : Grouped)
	{
		const FName Group = KV.Key;
		TArray<FEntry>& Entries = KV.Value;

		TSharedRef<SVerticalBox> ListVB = SNew(SVerticalBox);

		RootBox->AddSlot()
		.AutoHeight()
		.Padding(0.f, 4.f)
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()[ SNew(STextBlock).Text(FText::FromName(Group)) ]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()[ SNew(SSeparator) ]
				+ SVerticalBox::Slot().AutoHeight()[ ListVB ]
			]
		];

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

void SPinVarPanel::GatherPinnedProperties()
{
	if (!GEditor) return;
	UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>();
	if (!Subsystem) return;

	FPropertyEditorModule& PropEd = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Track which classes already got a header per group
	TMap<FName, TSet<FName>> LabeledClassesByGroup;

	for (const TPair<FName, TArray<FPinnedVariable>>& Pair : Subsystem->PinnedGroups)
	{
		const FName ClassName = Pair.Key;
		UClass* Cls = FindFirstObjectSafe<UClass>(*ClassName.ToString());
		if (!Cls || IsSkelOrReinst(Cls)) continue;

		UObject* CDO = Cls->GetDefaultObject(true);
		if (!CDO) continue;

		const FName ClassFName = Cls->GetFName();
		const FText ClassLabel = FText::FromString(Cls->GetName());

		for (const FPinnedVariable& Pinned : Pair.Value)
		{
			const FName VarName = Pinned.VariableName;

			UObject* TargetObj = CDO;
			if (!Pinned.ComponentTemplateName.IsNone())
			{
				// Bind to the component template (default subobject on the CDO)
				TargetObj = FindComponentTemplate(Cls, Pinned.ComponentTemplateName);
				if (!TargetObj)
				{
					UE_LOG(LogTemp, Warning, TEXT("PinVar.Panel: component template '%s' not found on %s"),
						*Pinned.ComponentTemplateName.ToString(), *CDO->GetName());
					continue;
				}
			}

			FProperty* FoundProp = FindFProperty<FProperty>(TargetObj->GetClass(), VarName);
			if (!FoundProp) continue;

			// Split "Core|Debug"
			TArray<FString> Tokens;
			const FString GroupStr = Pinned.GroupName.ToString();
			GroupStr.ParseIntoArray(Tokens, TEXT("|"), true);
			if (Tokens.Num() == 0) { Tokens.Add(GroupStr); }

			for (const FString& Tok : Tokens)
			{
				const FName GroupName(*Tok.TrimStartAndEnd());
				if (GroupName.IsNone()) continue;

				// Fresh property view per group
				FSinglePropertyParams Params;
				TSharedPtr<ISinglePropertyView> View =
					PropEd.CreateSingleProperty(TargetObj, VarName, Params);
				if (!View.IsValid()) continue;

				const bool bFirstForThisClassInGroup =
					!LabeledClassesByGroup.FindOrAdd(GroupName).Contains(ClassFName);

				TSharedRef<SVerticalBox> RowVB = SNew(SVerticalBox);

				if (bFirstForThisClassInGroup)
				{
					RowVB->AddSlot().AutoHeight()
					[
						SNew(STextBlock)
						.Text(ClassLabel)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					];
				}

				if (!Pinned.ComponentTemplateName.IsNone())
				{
					RowVB->AddSlot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("• %s"), *Pinned.ComponentTemplateName.ToString())))
						.ColorAndOpacity(FLinearColor(0.8f,0.8f,0.8f,1.f))
					];
				}

				RowVB->AddSlot().AutoHeight().Padding(0.f, 2.f, 0.f, 0.f)
				[
					StaticCastSharedRef<SWidget>(View.ToSharedRef())
				];

				Grouped.FindOrAdd(GroupName).Add({ GroupName, RowVB });

				if (bFirstForThisClassInGroup)
				{
					LabeledClassesByGroup.FindOrAdd(GroupName).Add(ClassFName);
				}
			}
		}
	}
}

FReply SPinVarPanel::OnAddBlueprintVariableClicked()
{
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

	FSlateApplication::Get().AddWindow(PickerWindow);
	return FReply::Handled();
}

void SPinVarPanel::OnBlueprintPicked(const FAssetData& AssetData)
{
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

	// Dialog state
	struct FState : public TSharedFromThis<FState>
	{
		UBlueprint* BP = nullptr;
		UClass* Class = nullptr;

		// Source: 0=Local BP var, 1=Parent C++ var, 2=Component var
		int32 SourceIndex = 0;

		// Local BP vars
		TArray<TSharedPtr<FName>> LocalVarOpts;
		TSharedPtr<FName> LocalVarSel;

		// Parent C++ props
		TArray<TSharedPtr<FName>> NativePropOpts;
		TSharedPtr<FName> NativePropSel;

		// Component + its props
		TArray<TSharedPtr<SPinVarPanel::FCompOption>> CompOpts;
		TSharedPtr<SPinVarPanel::FCompOption>         CompSel;
		TArray<TSharedPtr<FName>> CompPropOpts;
		TSharedPtr<SComboBox<TSharedPtr<FName>>> CompPropCombo;
		TSharedPtr<FName>         CompPropSel;

		// Group input
		FString GroupStr;
	};
	TSharedRef<FState> S = MakeShared<FState>();
	S->BP = BP;
	S->Class = TargetClass;

	// Populate local variables
	TArray<FName> Tmp;
	GatherLocalVars(BP, Tmp);
	for (const FName& N : Tmp) S->LocalVarOpts.Add(MakeShared<FName>(N));
	if (S->LocalVarOpts.Num()) S->LocalVarSel = S->LocalVarOpts[0];

	Tmp.Reset();
	GatherNativeProps(TargetClass, Tmp);
	for (const FName& N : Tmp) S->NativePropOpts.Add(MakeShared<FName>(N));
	if (S->NativePropOpts.Num()) S->NativePropSel = S->NativePropOpts[0];

	// Components: from CDO subobjects + SCS mapping
	BuildComponentOptions(BP, TargetClass, S->CompOpts);
	if (S->CompOpts.Num())
	{
		S->CompSel = S->CompOpts[0];
	}
	// First component's props (use exact TemplateName + log)
	if (S->CompSel.IsValid())
	{
		if (UObject* Template = FindComponentTemplate(S->Class, S->CompSel->TemplateName))
		{
			TArray<FName> P;
			GatherComponentPropsByTemplate(Template, P);
			for (const FName& N : P) S->CompPropOpts.Add(MakeShared<FName>(N));
			if (S->CompPropOpts.Num()) S->CompPropSel = S->CompPropOpts[0];

			// make the dropdown aware of the new items
			if (S->CompPropCombo.IsValid())
			{
				S->CompPropCombo->RefreshOptions();
				S->CompPropCombo->SetSelectedItem(S->CompPropSel);
			}

			UE_LOG(LogTemp, Log, TEXT("PinVar: (init) Component '%s' resolved to '%s' and offers %d props"),
				*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), S->CompPropOpts.Num());

		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PinVar: (init) Template not found for '%s' (template key '%s') on %s"),
				*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), *S->Class->GetName());
		}
	}
	
	// Build dialog
	TSharedRef<SWindow> Dialog = SNew(SWindow)
		.Title(FText::FromString(TEXT("Add Variable to Group")))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.ClientSize(FVector2D(520, 380));

	auto MakeNameCombo = [](TArray<TSharedPtr<FName>>& Source, TSharedPtr<FName>& Sel, TFunction<void(TSharedPtr<FName>)> OnSel)
	{
		return SNew(SComboBox<TSharedPtr<FName>>)
			.OptionsSource(&Source)
			.OnGenerateWidget_Lambda([](TSharedPtr<FName> It)
			{
				return SNew(STextBlock).Text(FText::FromName(It.IsValid()? *It : NAME_None));
			})
			.OnSelectionChanged_Lambda([&Sel, OnSel](TSharedPtr<FName> NewSel, ESelectInfo::Type)
			{
				Sel = NewSel;
				if (OnSel) OnSel(NewSel);
			})
			.InitiallySelectedItem(Sel)
			[
				SNew(STextBlock)
				.Text_Lambda([&Sel]()
				{
					return Sel.IsValid()? FText::FromName(*Sel) : FText::FromString(TEXT("None"));
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
				MakeNameCombo(S->LocalVarOpts, S->LocalVarSel, nullptr)
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
				MakeNameCombo(S->NativePropOpts, S->NativePropSel, nullptr)
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
				SNew(SComboBox<TSharedPtr<SPinVarPanel::FCompOption>>)
				.OptionsSource(&S->CompOpts)
				.OnGenerateWidget_Lambda([](TSharedPtr<SPinVarPanel::FCompOption> It)
				{
					return SNew(STextBlock).Text(FText::FromName(It.IsValid() ? It->Label : NAME_None));
				})
				.OnSelectionChanged_Lambda([S,this](TSharedPtr<SPinVarPanel::FCompOption> NewSel, ESelectInfo::Type)
				{
					S->CompSel = NewSel;
					S->CompPropOpts.Reset();
					S->CompPropSel.Reset();

					if (S->Class && S->CompSel.IsValid())
					{
						if (UObject* Tmpl = FindComponentTemplate(S->Class, S->CompSel->TemplateName))
						{
							TArray<FName> P;
							GatherComponentPropsByTemplate(Tmpl, P);
							for (const FName& N : P) S->CompPropOpts.Add(MakeShared<FName>(N));
								if (S->CompPropOpts.Num()) S->CompPropSel = S->CompPropOpts[0];

								if (S->CompPropCombo.IsValid())
								{
									S->CompPropCombo->RefreshOptions();
									S->CompPropCombo->SetSelectedItem(S->CompPropSel);
								}

								UE_LOG(LogTemp, Log, TEXT("PinVar: Component '%s' resolved to '%s' -> %d props"),
									*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), S->CompPropOpts.Num());

						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("PinVar: Template not found for '%s' (template key '%s') on %s"),
								*S->CompSel->Label.ToString(), *S->CompSel->TemplateName.ToString(), *S->Class->GetName());
						}
					}
				})
				[
					SNew(STextBlock)
					.Text_Lambda([S]()
					{
						return S->CompSel.IsValid() ? FText::FromName(S->CompSel->Label) : FText::FromString(TEXT("None"));
					})
				]
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0,4,0,0)
			[
				SNew(STextBlock).Text(FText::FromString("Property:"))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,2,0,0)
			[
				SNew(SComboBox<TSharedPtr<FName>>)
				.OptionsSource(&S->CompPropOpts)
				.OnGenerateWidget_Lambda([](TSharedPtr<FName> It)
				{
					return SNew(STextBlock).Text(FText::FromName(It.IsValid()? *It : NAME_None));
				})
				.OnSelectionChanged_Lambda([S](TSharedPtr<FName> NewSel, ESelectInfo::Type)
				{
					S->CompPropSel = NewSel;
				})
				.InitiallySelectedItem(S->CompPropSel)
				[
					SNew(STextBlock)
					.Text_Lambda([S]()
					{
						return S->CompPropSel.IsValid()? FText::FromName(*S->CompPropSel) : FText::FromString(TEXT("None"));
					})
				]
			]
		]

		// Group
		+ SVerticalBox::Slot().AutoHeight().Padding(12,10,12,8)
		[
			SNew(STextBlock).Text(FText::FromString("Group Name (A or A|B):"))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(12,0,12,12)
		[
			SNew(SEditableTextBox)
			.Text_Lambda([S]() { return FText::FromString(S->GroupStr); })
			.OnTextChanged_Lambda([S](const FText& T){ S->GroupStr = T.ToString(); })
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
					if (!S->Class || S->GroupStr.IsEmpty()) { Dialog->RequestDestroyWindow(); return FReply::Handled(); }

					FName VarName = NAME_None;
					FName CompName = NAME_None;

					if (S->SourceIndex == 0 && S->LocalVarSel.IsValid())
					{
						VarName = *S->LocalVarSel;
					}
					else if (S->SourceIndex == 1 && S->NativePropSel.IsValid())
					{
						VarName = *S->NativePropSel;
					}
					else if (S->SourceIndex == 2 && S->CompSel.IsValid() && S->CompPropSel.IsValid())
					{
						CompName = S->CompSel->TemplateName; 
						VarName  = *S->CompPropSel;
					}

					if (VarName.IsNone())
					{
						Dialog->RequestDestroyWindow();
						return FReply::Handled();
					}

					UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>();
					if (Subsystem)
					{
						FString Norm = S->GroupStr; 
						TArray<FString> Groups; Norm.ParseIntoArray(Groups, TEXT(","), true);
						for (FString& G : Groups)
						{
							G = G.TrimStartAndEnd();
							if (!G.IsEmpty())
							{
								Subsystem->StagePinVariable(S->Class->GetFName(), VarName, FName(*G), CompName);
							}
						}
						Subsystem->SaveToDisk();
					}

					Refresh();
					Dialog->RequestDestroyWindow();
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

FReply SPinVarPanel::OnRemoveBlueprintVariableClicked()
{
	ShowRemoveStagedDialog();
	return FReply::Handled();
}

void SPinVarPanel::ShowRemoveStagedDialog()
{
	if (!GEditor) return;
	UPinVarSubsystem* Subsystem = GEditor->GetEditorSubsystem<UPinVarSubsystem>();
	if (!Subsystem) return;

	struct FRemoveState : public TSharedFromThis<FRemoveState>
	{
		struct FItem
		{
			FName ClassName;
			FName VarName;
			FName GroupName;
			FName CompName;
			FString Label;
		};
		TArray<TSharedPtr<FItem>> Items;
		TSharedPtr<FItem> Selected;
	};
	TSharedRef<FRemoveState> State = MakeShared<FRemoveState>();

	{
		TArray<TTuple<FName,FName,FName,FName>> Quads;
		Subsystem->GetAllStagedWithComp(Quads);

		if (Quads.Num() == 0)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No staged variables to remove.")));
			return;
		}

		State->Items.Reserve(Quads.Num());
		for (const auto& Q : Quads)
		{
			const FName ClassName = Q.Get<0>();
			const FName VarName   = Q.Get<1>();
			const FName GroupName = Q.Get<2>();
			const FName CompName  = Q.Get<3>();

			FString ClassStr = ClassName.ToString();
			if (UClass* C = FindFirstObjectSafe<UClass>(*ClassStr))
			{
				ClassStr = C->GetName();
			}

			auto Item = MakeShared<FRemoveState::FItem>();
			Item->ClassName = ClassName;
			Item->VarName   = VarName;
			Item->GroupName = GroupName;
			Item->CompName  = CompName;
			Item->Label     = CompName.IsNone()
				? FString::Printf(TEXT("%s :: %s  —  %s"), *ClassStr, *VarName.ToString(), *GroupName.ToString())
				: FString::Printf(TEXT("%s[%s] :: %s  —  %s"), *ClassStr, *CompName.ToString(), *VarName.ToString(), *GroupName.ToString());
			State->Items.Add(Item);
		}
		State->Selected = State->Items[0];
	}

	TSharedRef<SWindow> Dialog = SNew(SWindow)
		.Title(FText::FromString(TEXT("Remove Staged Variable")))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.ClientSize(FVector2D(600, 200));

	Dialog->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(12,10,12,8)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("Select an entry to remove:"))) ]
		+ SVerticalBox::Slot().AutoHeight().Padding(12,0,12,8)
		[
			SNew(SComboBox<TSharedPtr<FRemoveState::FItem>>)
			.OptionsSource(&State->Items)
			.OnGenerateWidget_Lambda([](TSharedPtr<FRemoveState::FItem> In)
			{
				return SNew(STextBlock).Text(FText::FromString(In.IsValid()? In->Label : TEXT("")));
			})
			.OnSelectionChanged_Lambda([State](TSharedPtr<FRemoveState::FItem> In, ESelectInfo::Type)
			{
				State->Selected = In;
			})
			.InitiallySelectedItem(State->Selected)
			[
				SNew(STextBlock)
				.Text_Lambda([State]()
				{
					return FText::FromString(State->Selected.IsValid()? State->Selected->Label : TEXT(""));
				})
			]
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(12,0,12,12)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
			[
				SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "Button")
				.ForegroundColor(FLinearColor::Red)
				.Text(FText::FromString(TEXT("Remove")))
				.OnClicked_Lambda([this, Dialog, State, Subsystem]() -> FReply
				{
					if (State->Selected.IsValid())
					{
						Subsystem->UnstagePinVariable(State->Selected->ClassName,
							State->Selected->VarName, State->Selected->GroupName, State->Selected->CompName);
						Refresh();
					}
					Dialog->RequestDestroyWindow();
					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("Cancel")))
				.OnClicked_Lambda([Dialog]() -> FReply
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
