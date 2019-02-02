// testConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <xmmintrin.h>
#include <smmintrin.h>

using namespace std;

class RTVector
{
public:
    RTVector(){}

    float x() { return Vector[0]; }
    float y() { return Vector[1]; }
    float z() { return Vector[2]; }
    
    inline static float length()
    {
        return 1.0f;
    }

    inline static float dotProduct( RTVector &v0, RTVector &v1 )
    {
        return  1.0f;
    }

    inline static RTVector crossProduct( RTVector &v0, RTVector &v1 )
    {
        RTVector cross;
        return cross;
    }

private:
    float Vector[3];
};

class RTVectorPack
{
public:
    RTVectorPack(){}

    RTVectorPack( RTVector &v0, RTVector &v1, RTVector &v2, RTVector &v3 )
    {
        PackX = _mm_set_ps( v0.x(), v1.x(), v2.x(), v3.x() );
        PackY = _mm_set_ps( v0.y(), v1.y(), v2.y(), v3.y() );
        PackZ = _mm_set_ps( v0.z(), v1.z(), v2.z(), v3.z() );
    }

    ~RTVectorPack(){};

    static __m128 dotProduct( RTVectorPack &pack0, RTVectorPack &pack1 )
    {
        __m128 dotPack =
        _mm_add_ps
        (
            _mm_mul_ps( pack0.PackX, pack1.PackX ),
            _mm_add_ps
            (
                _mm_mul_ps( pack0.PackY, pack1.PackY ),
                _mm_mul_ps( pack0.PackZ, pack1.PackZ )
            )
        );

        return dotPack;
    }

    static RTVectorPack crossProduct( RTVectorPack &v0, RTVectorPack &v1 )
    {
        // https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/geometry/math-operations-on-points-and-vectors
        //a.y * b.z - a.z * b.y, 
        //a.z * b.x - a.x * b.z, 
        //a.x * b.y - a.y * b.x
        
        RTVectorPack cross;

        cross.PackX = _mm_sub_ps( _mm_mul_ps( v0.PackY, v1.PackZ ), _mm_mul_ps( v0.PackZ, v1.PackY ) );
        cross.PackY = _mm_sub_ps( _mm_mul_ps( v0.PackZ, v1.PackX ), _mm_mul_ps( v0.PackX, v1.PackZ ) );
        cross.PackZ = _mm_sub_ps( _mm_mul_ps( v0.PackX, v1.PackY ), _mm_mul_ps( v0.PackY, v1.PackX ) );
        
        return cross;
    }

private:
    __m128 PackX;
    __m128 PackY;
    __m128 PackZ;

};

class RayPack
{
public:
    RayPack(){}

    RayPack( RTVector &origin, RTVector &direction )
    {
        Origin = RTVectorPack( origin, origin, origin, origin );
        Direction = RTVectorPack( direction, direction, direction, direction );
    }

    ~RayPack(){}

    RTVectorPack Origin;
    RTVectorPack Direction;
};

bool rayTriangleIntersect( 
    const RTVector &orig, const RTVector &dir, 
    const RTVector &v0, const RTVector &v1, const RTVector &v2, 
    float &t) 
{ 
    // compute plane's normal
    RTVector v0v1 = v1 - v0; 
    RTVector v0v2 = v2 - v0; 
    // no need to normalize
    RTVector N = RTVector::crossProduct( v0v1, v0v2 ); // N 
    float area2 = N.length(); 
 
    // Step 1: finding P
 
    // check if ray and plane are parallel ?
    float NdotRayDirection = RTVector::dotProduct( N, dir ); 
    if (fabs(NdotRayDirection) < 1e-8) // almost 0 
        return false; // they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float d = RTVector::dotProduct( N, v0 ); 
 
    // compute t (equation 3)
    t = ( RTVector::dotProduct( N, orig ) + d ) / NdotRayDirection; 
    // check if the triangle is in behind the ray
    if (t < 0) return false; // the triangle is behind 
 
    // compute the intersection point using equation 1
    RTVector P = orig + t * dir; 
 
    // Step 2: inside-outside test
    RTVector C; // vector perpendicular to triangle's plane 
 
    // edge 0
    RTVector edge0 = v1 - v0; 
    RTVector vp0 = P - v0; 
    C = RTVector::crossProduct( edge0, vp0 ); 
    if ( RTVector::dotProduct( N, C ) < 0 ) return false; // P is on the right side 
 
    // edge 1
    RTVector edge1 = v2 - v1; 
    RTVector vp1 = P - v1; 
    C = RTVector::crossProduct( edge1, vp1 ); 
    if ( RTVector::dotProduct( N, C ) < 0 )  return false; // P is on the right side 
 
    // edge 2
    RTVector edge2 = v0 - v2; 
    RTVector vp2 = P - v2; 
    C = RTVector::crossProduct( edge2, vp2 ); 
    if ( RTVector::dotProduct( N, C ) < 0 ) return false; // P is on the right side; 
 
    return true; // this ray hits the triangle 
}

int _tmain(int argc, _TCHAR* argv[])
{
    __m128 t = _mm_set_ps( 1.2f, -3.0f, 2.0f, 7.3f );
    __m128 mask = _mm_cmpgt_ps( t, _mm_setzero_ps() );
    __m128i maski = _mm_cvtps_epi32( mask );
    __m128 maskf = _mm_cvtepi32_ps( maski );

    std::cout << "\n";
	return 0;
}

