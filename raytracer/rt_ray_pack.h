#ifndef RT_RAY_PACK_H
#define RT_RAY_PACK_H

#include "math/rt_vector.h"
#include "math/rt_vector_pack.h"

class RTRayPack
{
public:
    RTRayPack();

    RTRayPack( const RTVector &origin, const RTVector &direction )
    {
        Origin    = RTVectorPack( origin, origin, origin, origin );
        Direction = RTVectorPack( direction, direction, direction, direction );
    }

    ~RTRayPack();

    RTVectorPack Origin;
    RTVectorPack Direction;
};

#endif // RT_RAY_PACK_H
