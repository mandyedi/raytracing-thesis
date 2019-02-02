#ifndef RT_VECTOR_H
#define RT_VECTOR_H

#include <iostream>

class RTVector
{

private:

    float Vector[3];

public:

    RTVector() {}

    explicit RTVector( const float *v )
    {
        Vector[0] = v[0];
        Vector[1] = v[1];
        Vector[2] = v[2];
    }

    explicit RTVector( float x, float y, float z )
    {
        Vector[0] = x;
        Vector[1] = y;
        Vector[2] = z;
    }

    ~RTVector();

    inline float x() const
    {
        return Vector[0];
    }

    inline float y() const
    {
        return Vector[1];
    }

    inline float z() const
    {
        return Vector[2];
    }

    inline void setX( float x )
    {
        Vector[0] = x;
    }

    inline void setY( float y )
    {
        Vector[1] = y;
    }

    inline void setZ( float z )
    {
        Vector[2] = z;
    }

    RTVector Normalized() const;
    void Normalize();

    float Length() const;

    static float  DotProduct( const RTVector &a, const RTVector &b );
    static RTVector CrossProduct( const RTVector &a, const RTVector &b );

    RTVector& operator += ( const RTVector &v );
    RTVector& operator -= ( const RTVector &v );

    friend RTVector operator * ( const RTVector &v, float a );
    friend RTVector operator * ( float a, const RTVector &v );
    friend RTVector operator * ( const RTVector &v1, const RTVector &v2 );
    friend RTVector operator / ( const RTVector &v, float a );
    friend RTVector operator + ( const RTVector &a, const RTVector &b );
    friend RTVector operator - ( const RTVector &a, const RTVector &b );
    friend RTVector operator - ( const RTVector &v );
    friend std::ostream& operator << ( std::ostream& os, const RTVector &v );
} __attribute__ ( ( aligned (16) ) );

inline RTVector &RTVector::operator += ( const RTVector &v )
{
    Vector[0] += v.Vector[0];
    Vector[1] += v.Vector[1];
    Vector[2] += v.Vector[2];

    return *this;
}

inline RTVector &RTVector::operator -= ( const RTVector &v )
{
    Vector[0] -= v.Vector[0];
    Vector[1] -= v.Vector[1];
    Vector[2] -= v.Vector[2];

    return *this;
}

inline RTVector operator * ( const RTVector &v, float a )
{
    return RTVector( v.Vector[0] * a, v.Vector[1] * a, v.Vector[2] * a );
}

inline RTVector operator * ( float a, const RTVector &v )
{
    return RTVector( v.Vector[0] * a, v.Vector[1] * a, v.Vector[2] * a );
}

inline RTVector operator * ( const RTVector &v1, const RTVector &v2 )
{
    return RTVector( v1.Vector[0] * v2.Vector[0], v1.Vector[1] * v2.Vector[1], v1.Vector[2] * v2.Vector[2] );
}

inline RTVector operator / ( const RTVector &v, float a )
{
    return RTVector( v.Vector[0] / a, v.Vector[1] / a, v.Vector[2] / a );
}

inline RTVector operator + ( const RTVector &a, const RTVector &b )
{
    return RTVector( a.Vector[0] + b.Vector[0], a.Vector[1] + b.Vector[1], a.Vector[2] + b.Vector[2] );
}

inline RTVector operator - ( const RTVector &a, const RTVector &b )
{
    return RTVector( a.Vector[0] - b.Vector[0], a.Vector[1] - b.Vector[1], a.Vector[2] - b.Vector[2] );
}

inline RTVector operator - ( const RTVector &v )
{
    return RTVector( -v.Vector[0], -v.Vector[1], -v.Vector[2] );
}

inline std::ostream& operator << ( std::ostream& os, const RTVector &v )
{
    os << "(" << v.Vector[0] << ", " << v.Vector[1] << ", " << v.Vector[2] << ")";
    return os;
}

#endif // RT_VECTOR_H
