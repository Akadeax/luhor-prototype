#include "Details.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Components/ShapeComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "LuhorPrototype/HittableComponent.h"

#define LOCTEXT_NAMESPACE "Details"

void FHittableComponentDetail::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> selected;
	DetailLayout.GetObjectsBeingCustomized(selected);
	
	IDetailCategoryBuilder& category{ DetailLayout.EditCategory(
		TEXT("Warning"),
		FText::GetEmpty(),
		ECategoryPriority::Variable
	) };

	FSlateFontInfo warningFont{ IDetailLayoutBuilder::GetDetailFontBold() };
	warningFont.Size = 14.f;

	for (const TWeakObjectPtr<UObject>& weakObj : selected)
	{
		
		const UHittableComponent* hittable{ Cast<UHittableComponent>(weakObj.Get()) };
		if (!hittable) continue;

		if (!hittable->IsTemplate()) continue;

		const UBlueprintGeneratedClass* blueprintGC{ Cast<UBlueprintGeneratedClass>(hittable->GetOuter()) };
		check(blueprintGC);
		const AActor* actorCDO{ blueprintGC->GetDefaultObject<AActor>() };
		check(actorCDO);
		const USimpleConstructionScript* constructionScript{ blueprintGC->SimpleConstructionScript };
		check(constructionScript);

		USCS_Node* hittableNode{};
		for (USCS_Node* node : constructionScript->GetAllNodes())
		{
			if (node->ComponentTemplate.Get() == hittable)
			{
				hittableNode = node;
				break;
			}
		}

		bool hasAnyShapeChild{ false };
		for (USCS_Node* node : constructionScript->GetAllNodes())
		{
			if (!node->IsChildOf(hittableNode)) continue;
			if (node->ComponentTemplate.IsA<UShapeComponent>())
			{
				hasAnyShapeChild = true;
				break;
			}
		}

		if (!hasAnyShapeChild)
		{
			category.AddCustomRow(LOCTEXT("HittableWarningFilter", "Hittable Warning"))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HittableWarningText", "Pas op he! This hittable component has no valid Shape child!"))
				.ColorAndOpacity(FStyleColors::Warning)
				.Font(warningFont)
				.AutoWrapText(true)
				.Margin(5.f)
			];
		}
	}
}

#undef LOCTEXT_NAMESPACE
