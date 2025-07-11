#pragma once

#include "DetailWidgetRow.h"
#include "IDetailCustomization.h"

class FHittableComponentDetail final : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance() { return MakeShared<FHittableComponentDetail>(); }
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};
