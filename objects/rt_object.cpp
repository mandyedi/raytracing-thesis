#include <QDebug>
#include "rt_object.h"
#include "3rd_party/tiny_obj_loader.h"

RTObject::RTObject( std::string objFileName, RTVector position )
    : Name("no name")
    , Position( position )
    , Color( RTVector( 0.5f, 0.5f, 0.5f ) )
    , Diffuse( 0.5f )
    , Specular( 0.5f )
    , SpecularExponent( 2.0f )
    , Reflection( 0.8f )
    , SmoothShading( true )
    , Globject(nullptr)
    , MaterialType( RTMaterialType::Diffuse )
    , ObjectType( RTObjectType::Cube )
{
    std::vector<tinyobj::shape_t> shapes;
    std::string err = tinyobj::LoadObj( shapes, objFileName.c_str() );

    if ( !err.empty() )
    {
        qDebug() << err.c_str() << "\n";
    }
    else
    {
        // Use only the first shape
        const tinyobj::mesh_t &mesh = shapes[0].mesh;
        NumberOfVertices = mesh.indices.size();

        // Create mesh
        unsigned int reminder = NumberOfVertices % 12;

        // Create vertices
        Vertices = new RTVector[NumberOfVertices + reminder];
        for( unsigned int i = 0; i < NumberOfVertices; i++ )
        {
            unsigned int positionIndex = mesh.indices[i];
            Vertices[i].setX( mesh.positions[ positionIndex * 3 ] );
            Vertices[i].setY( mesh.positions[ positionIndex * 3 + 1] );
            Vertices[i].setZ( mesh.positions[ positionIndex * 3 + 2] );
        }

        // Create triangle (face) normals
        TriangleNormals = new RTVector[NumberOfVertices / 3];
        for ( unsigned int i = 0; i < NumberOfVertices; i += 3 )
        {
            const RTVector &v0 = Vertices[i];
            const RTVector &v1 = Vertices[i + 1];
            const RTVector &v2 = Vertices[i + 2];
            RTVector v0v1 = v1 - v0;
            RTVector v0v2 = v2 - v0;
            TriangleNormals[i / 3] = RTVector::CrossProduct( v0v1, v0v2 );
            TriangleNormals[i / 3].Normalize();
        }

        // Create vertex normals
        VertexNormals = new RTVector[NumberOfVertices];
        for ( unsigned int i = 0; i < mesh.indices.size(); i++ )
        {
            unsigned int indexOuter = mesh.indices[i];
            VertexNormals[i].setX( 0 );
            VertexNormals[i].setY( 0 );
            VertexNormals[i].setZ( 0 );
            const RTVector &currentNormal = TriangleNormals[i / 3];
            for ( unsigned int j = 0; j < mesh.indices.size(); j++ )
            {
                unsigned int indexInner = mesh.indices[j];
                if ( indexOuter == indexInner )
                {
                    float d = RTVector::DotProduct( currentNormal, TriangleNormals[j / 3] );
                    if ( d > 0.5f )
                    {
                        VertexNormals[i] += TriangleNormals[j / 3];
                    }
                }
            }
            VertexNormals[i].Normalize();
        }

        for ( unsigned int i = NumberOfVertices - 1; i < NumberOfVertices + reminder; i++ )
        {
            Vertices[i] = Vertices[NumberOfVertices - 1];
        }

        NumberOfVertexPacks = ( NumberOfVertices + reminder ) / 4;

        VerticesSIMDPack = (RTVectorPack*)_aligned_malloc( NumberOfVertexPacks * sizeof( RTVectorPack ), 16 );

        unsigned int packIndex = 0;
        for ( unsigned int i = 0; i < NumberOfVertices + reminder; i += 12 )
        {
            // Triangle 0
            RTVector &v0 = Vertices[i];
            RTVector &v1 = Vertices[i+1];
            RTVector &v2 = Vertices[i+2];
            // Triangle 1
            RTVector &v3 = Vertices[i+3];
            RTVector &v4 = Vertices[i+4];
            RTVector &v5 = Vertices[i+5];
            // Triangle 2
            RTVector &v6 = Vertices[i+6];
            RTVector &v7 = Vertices[i+7];
            RTVector &v8 = Vertices[i+8];
            // Triangle 3
            RTVector &v9 = Vertices[i+9];
            RTVector &v10 = Vertices[i+10];
            RTVector &v11 = Vertices[i+11];

            VerticesSIMDPack[packIndex]   = RTVectorPack( v0, v3, v6, v9 );
            VerticesSIMDPack[packIndex+1] = RTVectorPack( v1, v4, v7, v10 );
            VerticesSIMDPack[packIndex+2] = RTVectorPack( v2, v5, v8, v11 );

            packIndex += 3;
        }
    }
}

RTObject::~RTObject()
{
    delete [] Vertices;
    _aligned_free( VerticesSIMDPack );
}

void RTObject::setGLObject( GLObject *glObject )
{
    Globject = glObject;
}

void RTObject::updatePosition( RTVector position )
{
    Position = position;
}

void RTObject::movePosition( RTVector movementStep )
{
    Position += movementStep;
}

void RTObject::setScale( RTVector scale )
{
    Scale = scale;
}

void RTObject::setName( QString name )
{
    Name = name;
}

void RTObject::setMaterialType( RTObject::RTMaterialType type )
{
    MaterialType = type;
}

void RTObject::setObjectType( RTObject::RTObjectType type )
{
    ObjectType = type;
}

void RTObject::setColor( RTVector color )
{
    Color = color;
}

void RTObject::setDiffuse( float diffuse )
{
    Diffuse = diffuse;
}

void RTObject::setSpecular( float specular )
{
    Specular = specular;
}

void RTObject::setSpecularExponent( float specularExponent )
{
    SpecularExponent = specularExponent;
}

void RTObject::setReflection( float reflection )
{
    Reflection = reflection;
}

void RTObject::setSmoothShading( bool enabled )
{
    SmoothShading = enabled;
}

RTVector RTObject::getPosition()
{
    return Position;
}

RTVector RTObject::getScale()
{
    return Scale;
}

QString RTObject::getName()
{
    return Name;
}

RTObject::RTMaterialType RTObject::getMaterialType()
{
    return MaterialType;
}

RTObject::RTObjectType RTObject::getObjectType()
{
    return ObjectType;
}

RTVector RTObject::getColor()
{
    return Color;
}

float RTObject::getDiffuse()
{
    return Diffuse;
}

float RTObject::getSpecular()
{
    return Specular;
}

float RTObject::getSpecularExponent()
{
    return SpecularExponent;
}

float RTObject::getReflection()
{
    return Reflection;
}

bool RTObject::getSmoothShading()
{
    return SmoothShading;
}

RTVector RTObject::getNormal( unsigned int triangleIndex, float u, float v )
{
    return TriangleNormals[triangleIndex];
}

void RTObject::getNormalsForSmooth( unsigned int triangleIndex, RTVector &n0, RTVector &n1, RTVector &n2 )
{
    n0 = VertexNormals[ triangleIndex * 3 ];
    n1 = VertexNormals[ triangleIndex * 3 + 1 ];
    n2 = VertexNormals[ triangleIndex * 3 + 2 ];
}

int RTObject::intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD )
{
    bool isect = false;

    if ( useSIMD )
    {
        RTVectorPack scalePack( Scale, Scale, Scale, Scale );
        RTVectorPack positionPack( Position, Position, Position, Position );

        RTRayPack rayPack( ray.Origin, ray.Direction );

        unsigned int triangleIndexPacked = 0;
        for( unsigned int i = 0; i < NumberOfVertexPacks; i += 3 )
        {
            const RTVectorPack &pack0 = VerticesSIMDPack[ i ];
            const RTVectorPack &pack1 = VerticesSIMDPack[ i + 1 ];
            const RTVectorPack &pack2 = VerticesSIMDPack[ i + 2 ];

            static float max = std::numeric_limits<float>::max();
            __m128 packT = _mm_set1_ps( max );
            __m128 mask;
            __m128 packU;
            __m128 packV;

            if ( intersectTriangleSIMDPacked( scalePack, positionPack, rayPack, pack0, pack1, pack2, packT, mask, packU, packV ) )
            {
                float tArray[4];
                _mm_store_ps( tArray, packT );

                float maskArray[4];
                _mm_store_ps( maskArray, mask );

                float uArray[4];
                _mm_store_ps( uArray, packU );

                float vArray[4];
                _mm_store_ps( vArray, packV );

                for ( int j = 0; j < 4; j++ )
                {
                    if ( tArray[j] > 0 && tArray[j] < distance && maskArray[j] )
                    {
                        distance = tArray[j];
                        triangleIndex = triangleIndexPacked + j;
                        u = uArray[j];
                        v = vArray[j];
                        isect = true;
                    }
                }
            }
            triangleIndexPacked += 4;
        }
    }
    else
    {
        for ( unsigned int i = 0; i < NumberOfVertices; i += 3 )
        {
            const RTVector &v0 = Vertices[ i ];
            const RTVector &v1 = Vertices[ i + 1 ];
            const RTVector &v2 = Vertices[ i + 2 ];
            float t = std::numeric_limits<float>::max();
            float tempU;
            float tempV;
            if ( intersectTriangle( ray, v0, v1, v2, t, tempU, tempV ) && t < distance )
            {
                distance = t;
                triangleIndex = i / 3;
                isect = true;
                u = tempU;
                v = tempV;
            }
        }
    }

    return isect;
}

bool RTObject::intersectTriangle( const RTRay &ray, const RTVector &v0, const RTVector &v1, const RTVector &v2, float &t, float &u, float &v )
{
    RTVector v0t = ( v0 * Scale ) + Position;
    RTVector v1t = ( v1 * Scale ) + Position;
    RTVector v2t = ( v2 * Scale ) + Position;

    RTVector v0v1 = v1t - v0t;
    RTVector v0v2 = v2t - v0t;
    RTVector pvec = RTVector::CrossProduct( ray.Direction, v0v2 );
    float det = RTVector::DotProduct( v0v1, pvec );

    // ray and triangle are parallel if det is close to 0
    if ( fabs( static_cast<double>( det ) ) < 0.00000001 )
    {
        return false;
    }

    float invDet = 1 / det;

    RTVector tvec = ray.Origin - v0t;
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

bool RTObject::intersectTriangleSIMDPacked( const RTVectorPack &scalePack, const RTVectorPack &positionPack, const RTRayPack &rayPack, const RTVectorPack &vPack0, const RTVectorPack &vPack1, const RTVectorPack &vPack2, __m128 &tPack, __m128 &maskValid, __m128 &uPack, __m128 &vPack )
{
    static const __m128 zeros = _mm_setzero_ps();
    static const __m128 zerosE = _mm_set1_ps( 0.000001f );
    static const __m128 ones  = _mm_set1_ps( 1.0f );
    static const __m128 maskFloatSign = _mm_castsi128_ps( _mm_set1_epi32( 0x80000000 ) );

    RTVectorPack v0t = ( vPack0 * scalePack ) + positionPack;
    RTVectorPack v1t = ( vPack1 * scalePack ) + positionPack;
    RTVectorPack v2t = ( vPack2 * scalePack ) + positionPack;

    RTVectorPack v0v1 = v1t - v0t;
    RTVectorPack v0v2 = v2t - v0t;
    RTVectorPack pvec = RTVectorPack::crossProduct( rayPack.Direction, v0v2 );
    __m128 det = RTVectorPack::dotProduct( v0v1, pvec );

    // ray and triangle are parallel if det is close to 0
    __m128 detMask = _mm_cmplt_ps( _mm_andnot_ps( maskFloatSign, det ), zerosE );
    if ( _mm_movemask_ps( detMask ) == 0xff )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( detMask, ones );

    __m128 invDet = _mm_div_ps( ones, det );

    RTVectorPack tvec = rayPack.Origin - v0t;
    uPack = RTVectorPack::dotProduct( tvec, pvec ) * invDet;

    __m128 maskA = _mm_cmplt_ps( uPack, zeros );
    __m128 maskB = _mm_cmpgt_ps( uPack, ones );
    __m128 maskAB = _mm_or_ps( maskA, maskB );
    if ( _mm_movemask_ps( maskAB ) == 0xff )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( maskAB, maskValid );

    RTVectorPack qvec = RTVectorPack::crossProduct( tvec, v0v1 );
    vPack = RTVectorPack::dotProduct( rayPack.Direction, qvec ) * invDet;

    maskA = _mm_cmplt_ps( vPack, zeros );
    maskB = _mm_cmpgt_ps( _mm_add_ps( uPack, vPack ), ones );
    maskAB = _mm_or_ps( maskA, maskB );
    if ( _mm_movemask_ps( maskAB ) == 0xff )
    {
        return false;
    }

    maskValid = _mm_andnot_ps( maskAB, maskValid );

    tPack = RTVectorPack::dotProduct( v0v2, qvec ) * invDet;

    maskA = _mm_cmplt_ps( tPack, zerosE );
    maskValid = _mm_andnot_ps( maskA, maskValid );

    return _mm_movemask_ps( _mm_cmpeq_ps( maskValid, ones ) ) == 0x00 ? false : true;
}
