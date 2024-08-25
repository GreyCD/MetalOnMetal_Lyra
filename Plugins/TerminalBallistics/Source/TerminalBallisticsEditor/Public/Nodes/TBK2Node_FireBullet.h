// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node_BaseAsyncTask.h"
#include "TBK2Node_FireBullet.generated.h"

class FText;
class FKismetCompilerContext;
class FCompilerResultsLog;

UCLASS()
class TERMINALBALLISTICSEDITOR_API UTB_K2Node_FireBullet : public UK2Node_BaseAsyncTask
{
	GENERATED_BODY()
public:

	UTB_K2Node_FireBullet();

	FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	FText GetTooltipText() const override;
	FText GetMenuCategory() const override;
	FText GetPinDisplayName(const UEdGraphPin* Pin) const override;
	void GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, class UEdGraph* SourceGraph) override;

private:
	virtual void AllocateDefaultPins() override;

	void SetupPinsForDelegate(class UFunction* DelegateSignatureFunction);

	bool ValidateUpdatePins(FCompilerResultsLog& MessageLog) const;

	mutable bool bHasEmittedUpdateDisconnected_Warning = false;
	mutable bool bHasEmittedUnnecessaryUpdate_Warning = false;
};
