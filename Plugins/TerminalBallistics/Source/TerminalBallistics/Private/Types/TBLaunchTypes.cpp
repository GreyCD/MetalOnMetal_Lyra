// Copyright Erik Hedberg. All rights reserved.

#include "Types/TBLaunchTypes.h"
#include "Engine/NetSerialization.h"



bool FTBLaunchParams::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	using namespace UE::Net;
	TB_PACK_ARCHIVE_WITH_BITFIELDS_THREE(Ar, IgnoreOwner, AddToOwnerVelocity, bForceNoTracer);
	Ar << ProjectileSpeed;
	Ar << EffectiveRange;
	Ar << Timescale;
	Ar << OwnerIgnoreDistance;
	Ar << TracerActivationDistance;
	Ar << FireTransform;
	bOutSuccess = SafeNetSerializeTArray_Default<63>(Ar, ToIgnore);
	bOutSuccess &= SafeNetSerializeTArray_Default<31>(Ar, ObjectTypes);
	SerializeOptionalValue(Ar.IsSaving(), Ar, TraceChannel, (TEnumAsByte<ECollisionChannel>)ECC_GameTraceChannel10);
	SerializeOptionalValue(Ar.IsSaving(), Ar, Owner, (TObjectPtr<AActor>)nullptr);
	SerializeOptionalValue(Ar.IsSaving(), Ar, Instigator, (TObjectPtr<AController>)nullptr);
	SerializeOptionalValue(Ar.IsSaving(), Ar, Payload, (TObjectPtr<UObject>)nullptr);
	return bOutSuccess;
}