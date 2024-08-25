// Copyright Erik Hedberg. All rights reserved.

#include "Nodes/TBK2Node_FireBullet.h"

#include "AsyncNodes/TBFireBulletAsync.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Containers/EnumAsByte.h"
#include "Containers/Set.h"
#include "Core/TBBulletDataAsset.h"
#include "CoreGlobals.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraphSchema_K2.h"
#include "EdGraphUtilities.h"
#include "Engine/Blueprint.h"
#include "Engine/MemberReference.h"
#include "EngineLogs.h"
#include "HAL/PlatformCrt.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Self.h"
#include "K2Node_TemporaryVariable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/CompilerResultsLog.h"
#include "KismetCompiler.h"
#include "Logging/LogCategory.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"
#include "Templates/Casts.h"
#include "Templates/ChooseClass.h"
#include "Trace/Detail/Channel.h"
#include "UObject/Class.h"
#include "UObject/Field.h"
#include "UObject/Object.h"
#include "UObject/UnrealNames.h"
#include "UObject/UnrealType.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"



#define LOCTEXT_NAMESPACE "TBK2Node_FireBullet"

#define PinName_OnComplete TEXT("OnComplete")
#define PinDisplayName_OnComplete "On Complete"

#define PinName_OnHit TEXT("OnHit")
#define PinDisplayName_OnHit "On Hit"

#define PinName_OnExitHit TEXT("OnExitHit")
#define PinDisplayName_OnExitHit "On Exit Hit"

#define PinName_OnInjure TEXT("OnInjure")
#define PinDisplayName_OnInjure "On Injure"

#define PinName_OnUpdate TEXT("OnUpdate")
#define PinDisplayName_OnUpdate "On Update"

#define PinName_Bullet TEXT("Bullet")
#define PinName_ShouldUpdate TEXT("bEnableUpdate")

#define PinName_CompletionResult TEXT("CompletionResult")


#define IsPin(PinName) FindPinChecked(PinName) == Pin
#define IF_IS_PIN(PinName, Inner) if (IsPin(PinName)) { \
	Inner; \
}
#define ELIF_IS_PIN(PinName, Inner) if (IsPin(PinName)) { \
	Inner; \
}

#define GetPinName(RawPinName) PREPROCESSOR_JOIN(PinDisplayName_, RawPinName)
#define GetPinDisplayNameLOCTEXT(RawPinName) LOCTEXT(PREPROCESSOR_TO_STRING(PREPROCESSOR_JOIN(TBFireBullet_, PREPROCESSOR_JOIN(RawPinName, Pin))), PREPROCESSOR_JOIN(PinDisplayName_, RawPinName))


UTB_K2Node_FireBullet::UTB_K2Node_FireBullet()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UTBFireBulletAsync, CreateProxyObject);
	ProxyFactoryClass = UTBFireBulletAsync::StaticClass();
	ProxyClass = UTBFireBulletAsync::StaticClass();
}

FText UTB_K2Node_FireBullet::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("TBFireBullet_NodeTitle", "Fire Bullet");
}

FText UTB_K2Node_FireBullet::GetTooltipText() const
{
	return LOCTEXT("TBFireBullet_Tooltip", "Fires a bullet.");
}

FText UTB_K2Node_FireBullet::GetMenuCategory() const
{
	return LOCTEXT("TBFireBullet_Category", "Terminal Ballistics|Projectiles");
}


FText UTB_K2Node_FireBullet::GetPinDisplayName(const UEdGraphPin* Pin) const
{
	IF_IS_PIN(PinName_OnComplete, return GetPinDisplayNameLOCTEXT(OnComplete))
	ELIF_IS_PIN(PinName_OnHit, return GetPinDisplayNameLOCTEXT(OnHit))
	ELIF_IS_PIN(PinName_OnExitHit, return GetPinDisplayNameLOCTEXT(OnExitHit))
	ELIF_IS_PIN(PinName_OnInjure, return GetPinDisplayNameLOCTEXT(OnInjure))
	ELIF_IS_PIN(PinName_OnUpdate, return GetPinDisplayNameLOCTEXT(OnUpdate))
	return Super::GetPinDisplayName(Pin);
}

void UTB_K2Node_FireBullet::GetPinHoverText(const UEdGraphPin& Pin, FString& HoverTextOut) const
{

	Super::GetPinHoverText(Pin, HoverTextOut);
}

void UTB_K2Node_FireBullet::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	bHasEmittedUnnecessaryUpdate_Warning = false;
	bHasEmittedUpdateDisconnected_Warning = false;
	ValidateUpdatePins(CompilerContext.MessageLog);
	Super::ExpandNode(CompilerContext, SourceGraph);
}

void UTB_K2Node_FireBullet::AllocateDefaultPins()
{
	/* Modified version of UK2Node_BaseAsyncTask::AllocateDefaultPins, where pins are created for ALL delegates, not just the first one. */

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	bool bExposeProxy = false;
	bool bHideThen = false;
	FText ExposeProxyDisplayName;
	for (const UStruct* TestStruct = ProxyClass; TestStruct; TestStruct = TestStruct->GetSuperStruct())
	{
		bExposeProxy |= TestStruct->HasMetaData(TEXT("ExposedAsyncProxy"));
		bHideThen |= TestStruct->HasMetaData(TEXT("HideThen"));
		if (ExposeProxyDisplayName.IsEmpty())
		{
			ExposeProxyDisplayName = TestStruct->GetMetaDataText(TEXT("ExposedAsyncProxy"));
		}
	}

	if (!bHideThen)
	{
		CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	}

	if (bExposeProxy)
	{
		UEdGraphPin* ProxyPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, ProxyClass, FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
		if (!ExposeProxyDisplayName.IsEmpty())
		{
			ProxyPin->PinFriendlyName = ExposeProxyDisplayName;
		}
	}

	UFunction* Function = GetFactoryFunction();
	if (!bHideThen && Function)
	{
		for (TFieldIterator<FProperty> PropIt(Function); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			FProperty* Param = *PropIt;
			// invert the check for function inputs (below) and exclude the factory func's return param - the assumption is
			// that the factory method will be returning the proxy object, and that other outputs should be forwarded along 
			// with the 'then' pin
			const bool bIsFunctionOutput = Param->HasAnyPropertyFlags(CPF_OutParm) && !Param->HasAnyPropertyFlags(CPF_ReferenceParm) && !Param->HasAnyPropertyFlags(CPF_ReturnParm);
			if (bIsFunctionOutput)
			{
				UEdGraphPin* Pin = CreatePin(EGPD_Output, NAME_None, Param->GetFName());
				K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType);
			}
		}
	}

	for (TFieldIterator<FProperty> PropertyIt(ProxyClass); PropertyIt; ++PropertyIt)
	{
		if (FMulticastDelegateProperty* Property = CastField<FMulticastDelegateProperty>(*PropertyIt))
		{
			if (FindPin(Property->GetFName()))
			{
				continue;
			}
			UEdGraphPin* ExecPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, Property->GetFName());
			ExecPin->PinToolTip = Property->GetToolTipText().ToString();
			ExecPin->PinFriendlyName = Property->GetDisplayNameText();

			SetupPinsForDelegate(Property->SignatureFunction);
		}
	}

	bool bAllPinsGood = true;
	if (Function)
	{
		TSet<FName> PinsToHide;
		FBlueprintEditorUtils::GetHiddenPinsForFunction(GetGraph(), Function, PinsToHide);
		for (TFieldIterator<FProperty> PropIt(Function); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			FProperty* Param = *PropIt;
			const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
			if (!bIsFunctionInput)
			{
				// skip function output, it's internal node data 
				continue;
			}

			UEdGraphNode::FCreatePinParams PinParams;
			PinParams.bIsReference = Param->HasAnyPropertyFlags(CPF_ReferenceParm) && bIsFunctionInput;
			UEdGraphPin* Pin = CreatePin(EGPD_Input, NAME_None, Param->GetFName(), PinParams);
			const bool bPinGood = (Pin && K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType));

			if (bPinGood)
			{
				// Check for a display name override
				const FString& PinDisplayName = Param->GetMetaData(FBlueprintMetadata::MD_DisplayName);
				if (!PinDisplayName.IsEmpty())
				{
					Pin->PinFriendlyName = FText::FromString(PinDisplayName);
				}

				//Flag pin as read only for const reference property
				Pin->bDefaultValueIsIgnored = Param->HasAllPropertyFlags(CPF_ConstParm | CPF_ReferenceParm) && (!Function->HasMetaData(FBlueprintMetadata::MD_AutoCreateRefTerm) || Pin->PinType.IsContainer());

				const bool bAdvancedPin = Param->HasAllPropertyFlags(CPF_AdvancedDisplay);
				Pin->bAdvancedView = bAdvancedPin;
				if (bAdvancedPin && (ENodeAdvancedPins::NoPins == AdvancedPinDisplay))
				{
					AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
				}

				FString ParamValue;
				if (K2Schema->FindFunctionParameterDefaultValue(Function, Param, ParamValue))
				{
					K2Schema->SetPinAutogeneratedDefaultValue(Pin, ParamValue);
				}
				else
				{
					K2Schema->SetPinAutogeneratedDefaultValueBasedOnType(Pin);
				}

				if (PinsToHide.Contains(Pin->PinName))
				{
					Pin->bHidden = true;
				}
			}

			bAllPinsGood = bAllPinsGood && bPinGood;
		}
	}

	Super::Super::AllocateDefaultPins(); // Skip K2Node_BaseAyncTask implementation, since it only handles pin setup for a single delegate
}

void UTB_K2Node_FireBullet::SetupPinsForDelegate(UFunction* DelegateSignatureFunction)
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	if (DelegateSignatureFunction)
	{
		for (TFieldIterator<FProperty> PropIt(DelegateSignatureFunction); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
		{
			FProperty* Param = *PropIt;
			const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
			if (bIsFunctionInput)
			{
				UEdGraphPin* Pin = CreatePin(EGPD_Output, NAME_None, Param->GetFName());
				K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType);

				UK2Node_CallFunction::GeneratePinTooltipFromFunction(*Pin, DelegateSignatureFunction);
			}
		}
	}
}

bool UTB_K2Node_FireBullet::ValidateUpdatePins(FCompilerResultsLog& MessageLog) const
{
	UEdGraphPin* ShouldUpdatePin = FindPin(PinName_ShouldUpdate);
	if (!ShouldUpdatePin)
	{
		return false;
	}
	ShouldUpdatePin = FEdGraphUtilities::GetNetFromPin(ShouldUpdatePin);

	UEdGraphPin* OnUpdatePin = FindPin(PinName_OnUpdate);
	const bool bOnUpdateIsLinked = OnUpdatePin->LinkedTo.Num() > 0;

	bool bShouldUpdate = ShouldUpdatePin->GetDefaultAsString().Equals("true", ESearchCase::IgnoreCase);

	if (bShouldUpdate && !bOnUpdateIsLinked)
	{
		if(!bHasEmittedUpdateDisconnected_Warning)
		{
			MessageLog.Warning(*NSLOCTEXT("KismetCompiler", "UpdateDisconnected_Warning", "@@is true, but@@is not used. This may cause unnecessary performance loss.").ToString(), ShouldUpdatePin, OnUpdatePin);
			bHasEmittedUpdateDisconnected_Warning = true;
		}
		return true;
	}
	else if (!bShouldUpdate && bOnUpdateIsLinked)
	{
		if(!bHasEmittedUnnecessaryUpdate_Warning)
		{
			MessageLog.Warning(*NSLOCTEXT("KismetCompiler", "UnnecessaryUpdate_Warning", "@@is in use but will never be called, since@@is set to false.").ToString(), OnUpdatePin, ShouldUpdatePin);
			bHasEmittedUnnecessaryUpdate_Warning = true;
		}
		return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
