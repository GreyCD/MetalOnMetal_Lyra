// Copyright Erik Hedberg. All rights reserved.

#include "Types/TBLocationAndDirection.h"
#include "Net/Core/Serialization/QuantizedVectorSerialization.h"


bool FTBLocationAndDirection::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << *this;
	bOutSuccess = true;
	return bOutSuccess;
}

FArchive& operator<<(FArchive& Ar, FTBLocationAndDirection& LocationAndDirection)
{
	using namespace UE::Net;
	SerializePackedVector<100, 30>(LocationAndDirection.Location, Ar);
	Ar << LocationAndDirection.Direction;
	return Ar;
}