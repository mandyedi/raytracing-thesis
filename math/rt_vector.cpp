#include <math.h>
#include "rt_vector.h"
#include "utils.h"

RTVector::~RTVector()
{

}

RTVector RTVector::Normalized() const
{
    float length = Length();
    if ( !Utils::IsNull( length ) )
    {
        float invLength = 1.0f / length;
        return RTVector( Vector[0] * invLength , Vector[1] * invLength, Vector[2] * invLength );
    }
    else if ( Utils::IsNull( length - 1.0f ) )
    {
        return *this;
    }

    return RTVector();
}

void RTVector::Normalize()
{
    float length = Length();
    if ( Utils::IsNull( length ) || Utils::IsNull( length - 1.0f ) )
    {
        return;
    }

    float invLength = 1.0f / length;
    Vector[0] *= invLength;
    Vector[1] *= invLength;
    Vector[2] *= invLength;
}

float RTVector::Length() const
{
    return sqrtf( Vector[0] * Vector[0] + Vector[1] * Vector[1] + Vector[2] * Vector[2] );
}

/*static*/ float RTVector::DotProduct( const RTVector &a, const RTVector &b )
{
    return a.Vector[0] * b.Vector[0] + a.Vector[1] * b.Vector[1] + a.Vector[2] * b.Vector[2];
}

/*static*/ RTVector RTVector::CrossProduct( const RTVector &a, const RTVector &b )
{
    // [0] X
    // [1] Y
    // [2] Z
    return RTVector( a.Vector[1] * b.Vector[2] - a.Vector[2] * b.Vector[1],
                     a.Vector[2] * b.Vector[0] - a.Vector[0] * b.Vector[2],
                     a.Vector[0] * b.Vector[1] - a.Vector[1] * b.Vector[0] );
}
