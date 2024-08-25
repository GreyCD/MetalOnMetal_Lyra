
#pragma once

#include "Types/TBProjectileId.h" // Include the header that defines FTBProjectileId

FORCEINLINE uint32 GetTypeHash(const FTBProjectileId& ProjectileId)
{
	return GetTypeHash(ProjectileId.Guid);
}