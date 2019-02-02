#ifndef RT_VECTOR_PACK_H
#define RT_VECTOR_PACK_H

#include <xmmintrin.h>
#include <QDebug>
#include "rt_vector.h"

class RTVectorPack
{
public:
    RTVectorPack();

    RTVectorPack( const RTVector &v0, const RTVector &v1, const RTVector &v2, const RTVector &v3 )
    {
        PackX = _mm_set_ps( v3.x(), v2.x(), v1.x(), v0.x() );
        PackY = _mm_set_ps( v3.y(), v2.y(), v1.y(), v0.y() );
        PackZ = _mm_set_ps( v3.z(), v2.z(), v1.z(), v0.z() );
    }

    ~RTVectorPack();

    void printPack()
    {
        qDebug() << "  " << get0( PackX ) << "  " << get1( PackX ) << "  " << get2( PackX ) << "  " << get3( PackX );
        qDebug() << "  " << get0( PackY ) << "  " << get1( PackY ) << "  " << get2( PackY ) << "  " << get3( PackY );
        qDebug() << "  " << get0( PackZ ) << "  " << get1( PackZ ) << "  " << get2( PackZ ) << "  " << get3( PackZ );
    }

    float get0( __m128 &pack ) const
    {
        return _mm_cvtss_f32( pack );
    }

    float get1( __m128 &pack ) const
    {
        return _mm_cvtss_f32( _mm_shuffle_ps( pack, pack, _MM_SHUFFLE( 1, 1, 1, 1 ) ) );
    }

    float get2( __m128 &pack ) const
    {
        return _mm_cvtss_f32( _mm_shuffle_ps( pack, pack, _MM_SHUFFLE( 2, 2, 2, 2 ) ) );
    }

    float get3( __m128 &pack ) const
    {
        return _mm_cvtss_f32( _mm_shuffle_ps( pack, pack, _MM_SHUFFLE( 3, 3, 3, 3 ) ) );
    }

    static __m128 dotProduct( const RTVectorPack &pack0, const RTVectorPack &pack1 )
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

    static RTVectorPack crossProduct( const RTVectorPack &v0, const RTVectorPack &v1 )
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

    friend RTVectorPack operator * ( const RTVectorPack &v0, const RTVectorPack &v1 );
    friend RTVectorPack operator + ( const RTVectorPack &v0, const RTVectorPack &v1 );
    friend RTVectorPack operator - ( const RTVectorPack &v0, const RTVectorPack &v1 );

private:
    __m128 PackX;
    __m128 PackY;
    __m128 PackZ;
};

inline RTVectorPack operator * ( const RTVectorPack &v0, const RTVectorPack &v1 )
{
    RTVectorPack v;

    v.PackX = _mm_mul_ps( v0.PackX, v1.PackX );
    v.PackY = _mm_mul_ps( v0.PackY, v1.PackY );
    v.PackZ = _mm_mul_ps( v0.PackZ, v1.PackZ );

    return v;
}

inline RTVectorPack operator + ( const RTVectorPack &v0, const RTVectorPack &v1 )
{
    RTVectorPack v;

    v.PackX = _mm_add_ps( v0.PackX, v1.PackX );
    v.PackY = _mm_add_ps( v0.PackY, v1.PackY );
    v.PackZ = _mm_add_ps( v0.PackZ, v1.PackZ );

    return v;
}

inline RTVectorPack operator - ( const RTVectorPack &v0, const RTVectorPack &v1 )
{
    RTVectorPack v;

    v.PackX = _mm_sub_ps( v0.PackX, v1.PackX );
    v.PackY = _mm_sub_ps( v0.PackY, v1.PackY );
    v.PackZ = _mm_sub_ps( v0.PackZ, v1.PackZ );

    return v;
}

#endif // RT_VECTOR_PACK_H
