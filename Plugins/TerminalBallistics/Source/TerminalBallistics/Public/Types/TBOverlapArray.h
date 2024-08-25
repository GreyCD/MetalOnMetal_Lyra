// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "Containers/Array.h"
#include "Engine/OverlapResult.h"
#include "Misc/TBMacrosAndFunctions.h"

#include "TBOverlapArray.generated.h"

/**
* Wrapper struct for TArray<FOverlapResult>
*/
USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FTBOverlapArray
{
	GENERATED_BODY()

public:
	TArray<FOverlapResult> Overlaps;

	FTBOverlapArray() = default;

	FTBOverlapArray(TArray<FOverlapResult> arr)
		: Overlaps(arr)
	{}

	TB_ARRAY_WRAPPED_STRUCT_ITERATOR_FUNCS(Overlaps)
};
