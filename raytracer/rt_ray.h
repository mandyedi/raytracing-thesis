#ifndef RT_RAY_H
#define RT_RAY_H

#include "math/rt_vector.h"

class RTRay
{
public:
    RTRay();
    RTRay( const RTVector &origin, const RTVector &direction );

    RTVector Origin;
    RTVector Direction;
};

#endif // RT_RAY_H
