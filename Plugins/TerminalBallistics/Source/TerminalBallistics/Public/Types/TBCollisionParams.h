// Copyright Erik Hedberg. All rights reserved.

#pragma once

#include "CollisionQueryParams.h"

#include "TBCollisionParams.generated.h"

USTRUCT(BlueprintType)
struct TERMINALBALLISTICS_API FTBCollisionParams
{
	GENERATED_BODY()

public:
	FCollisionQueryParams QueryParams;
	FCollisionObjectQueryParams ObjectQueryParams;

	FTBCollisionParams()
	{
		QueryParams = FCollisionQueryParams::DefaultQueryParam;
		ObjectQueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam;
	}

	FTBCollisionParams(FCollisionQueryParams params, FCollisionObjectQueryParams objParams)
	{
		QueryParams = params;
		ObjectQueryParams = objParams;
	}
};
