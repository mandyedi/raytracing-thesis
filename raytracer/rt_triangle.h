#ifndef RT_TRIANGLE_H
#define RT_TRIANGLE_H

#include <cmath>
#include <emmintrin.h>
#include "math/rt_vector.h"
#include "math/rt_vector_pack.h"
#include "raytracer/rt_ray.h"
#include "raytracer/rt_ray_pack.h"

// Möller-Trumbore ray-triangle tests on world space triangles, given as the first vertex and the
// edges to the other two (v0, v1 - v0, v2 - v0). RTObject and RTBVH share them, so both find
// exactly the same hits.
class RTTriangle
{
public:
    // True if the ray hits the triangle at t > 0
    static bool intersect( const RTRay &ray, const RTVector &v0, const RTVector &v0v1, const RTVector &v0v2, float &t, float &u, float &v );

    // 4 triangles at a time: true if any is hit; maskValid is 1.0f in the lanes of the hit ones, 0.0f in the others
    static bool intersectPack( const RTRayPack &rayPack, const RTVectorPack &v0, const RTVectorPack &v0v1, const RTVectorPack &v0v2, __m128 &tPack, __m128 &maskValid, __m128 &uPack, __m128 &vPack );
};

inline bool RTTriangle::intersect( const RTRay &ray, const RTVector &v0, const RTVector &v0v1, const RTVector &v0v2, float &t, float &u, float &v )
{
    RTVector pvec = RTVector::CrossProduct( ray.Direction, v0v2 );
    float det = RTVector::DotProduct( v0v1, pvec );

    // ray and triangle are parallel if det is close to 0
    if ( fabs( static_cast<double>( det ) ) < 0.00000001 )
    {
        return false;
    }

    float invDet = 1 / det;

    RTVector tvec = ray.Origin - v0;
    u = RTVector::DotProduct( tvec, pvec ) * invDet;

    if ( u < 0 || u > 1 )
    {
        return false;
    }

    RTVector qvec = RTVector::CrossProduct( tvec, v0v1 );
    v = RTVector::DotProduct( ray.Direction, qvec ) * invDet;

    if ( v < 0 || u + v > 1 )
    {
        return false;
    }

    t = RTVector::DotProduct( v0v2, qvec ) * invDet;

    return ( t > 0 ) ? true : false;
}

inline bool RTTriangle::intersectPack( const RTRayPack &rayPack, const RTVectorPack &v0, const RTVectorPack &v0v1, const RTVectorPack &v0v2, __m128 &tPack, __m128 &maskValid, __m128 &uPack, __m128 &vPack )
{
    static const __m128 zeros = _mm_setzero_ps();
    static const __m128 zerosE = _mm_set1_ps( 0.000001f );
    static const __m128 ones  = _mm_set1_ps( 1.0f );
    static const __m128 maskFloatSign = _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ) );

    RTVectorPack pvec = RTVectorPack::crossProduct( rayPack.Direction, v0v2 );
    __m128 det = RTVectorPack::dotProduct( v0v1, pvec );

    // ray and triangle are parallel if det is close to 0
    __m128 detMask = _mm_cmplt_ps( _mm_andnot_ps( maskFloatSign, det ), zerosE );
    if ( _mm_movemask_ps( detMask ) == 0xf )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( detMask, ones );

    __m128 invDet = _mm_div_ps( ones, det );

    RTVectorPack tvec = rayPack.Origin - v0;
    uPack = _mm_mul_ps( RTVectorPack::dotProduct( tvec, pvec ), invDet );

    __m128 maskA = _mm_cmplt_ps( uPack, zeros );
    __m128 maskB = _mm_cmpgt_ps( uPack, ones );
    __m128 maskAB = _mm_or_ps( maskA, maskB );
    if ( _mm_movemask_ps( maskAB ) == 0xf )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( maskAB, maskValid );

    RTVectorPack qvec = RTVectorPack::crossProduct( tvec, v0v1 );
    vPack = _mm_mul_ps( RTVectorPack::dotProduct( rayPack.Direction, qvec ), invDet );

    maskA = _mm_cmplt_ps( vPack, zeros );
    maskB = _mm_cmpgt_ps( _mm_add_ps( uPack, vPack ), ones );
    maskAB = _mm_or_ps( maskA, maskB );
    if ( _mm_movemask_ps( maskAB ) == 0xf )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( maskAB, maskValid );

    tPack = _mm_mul_ps( RTVectorPack::dotProduct( v0v2, qvec ), invDet );

    maskA = _mm_cmplt_ps( tPack, zerosE );
    maskValid = _mm_andnot_ps( maskA, maskValid );

    return _mm_movemask_ps( _mm_cmpeq_ps( maskValid, ones ) ) == 0x00 ? false : true;
}

#endif // RT_TRIANGLE_H
