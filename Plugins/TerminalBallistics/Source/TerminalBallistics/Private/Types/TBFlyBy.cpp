// Copyright Erik Hedberg. All rights reserved.

#include "Types/TBFlyBy.h"
#include "Core/TBFlyByInterface.h"

void FTBFlyBy::NotifyActorOfFlyBy()
{
	check(IsInGameThread());

	if (HitActor->Implements<UTBFlyByInterface>())
	{
		ITBFlyByInterface::Execute_RecieveFlyByEvent(HitActor, *this);
	}
}
