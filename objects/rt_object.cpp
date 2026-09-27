#include <cmath>
#include <limits>
#include <vector>
#include <emmintrin.h>
#include "rt_object.h"
#include "raytracer/rt_triangle.h"
#include "3rd_party/tiny_obj_loader.h"

RTObject::RTObject( std::string objFileName, RTVector position )
    : Name("no name")
    , Position( position )
    , Scale( RTVector( 1.0f, 1.0f, 1.0f ) )
    , Color( RTVector( 0.5f, 0.5f, 0.5f ) )
    , Diffuse( 0.5f )
    , Specular( 0.5f )
    , SpecularExponent( 2.0f )
    , Reflection( 0.8f )
    , SmoothShading( true )
    , Globject(nullptr)
    , MaterialType( RTMaterialType::Diffuse )
    , ObjectType( RTObjectType::Cube )
    , Vertices( nullptr )
    , TriangleNormals( nullptr )
    , VertexNormals( nullptr )
    , NumberOfVertices( 0 )
    , WorldTriangles( nullptr )
    , WorldTrianglePacks( nullptr )
    , NumberOfWorldTrianglePacks( 0 )
{
    std::vector<tinyobj::shape_t> shapes;
    std::string err = tinyobj::LoadObj( shapes, objFileName.c_str() );

    if ( err.empty() && ( shapes.empty() || shapes[0].mesh.indices.empty() ) )
    {
        err = "No triangles in " + objFileName;
    }

    if ( !err.empty() )
    {
        // tinyobjloader ends its messages with a newline
        while ( !err.empty() && ( err.back() == '\n' || err.back() == '\r' ) )
        {
            err.pop_back();
        }
        LoadError = err;
    }
    else
    {
        // Use only the first shape
        const tinyobj::mesh_t &mesh = shapes[0].mesh;
        NumberOfVertices = mesh.indices.size();

        // Create mesh
        unsigned int reminder = ( 12 - NumberOfVertices % 12 ) % 12;

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

        // Corners (positions in mesh.indices) of each vertex, in order, so a corner finds
        // the triangles that share its vertex without scanning the whole mesh
        std::vector<std::vector<unsigned int>> cornersOfVertex( mesh.positions.size() / 3 );
        for ( unsigned int i = 0; i < mesh.indices.size(); i++ )
        {
            cornersOfVertex[mesh.indices[i]].push_back( i );
        }

        // Create vertex normals
        VertexNormals = new RTVector[NumberOfVertices];
        for ( unsigned int i = 0; i < mesh.indices.size(); i++ )
        {
            VertexNormals[i].setX( 0 );
            VertexNormals[i].setY( 0 );
            VertexNormals[i].setZ( 0 );
            const RTVector &currentNormal = TriangleNormals[i / 3];
            for ( unsigned int j : cornersOfVertex[mesh.indices[i]] )
            {
                float d = RTVector::DotProduct( currentNormal, TriangleNormals[j / 3] );
                if ( d > 0.5f )
                {
                    VertexNormals[i] += TriangleNormals[j / 3];
                }
            }
            VertexNormals[i].Normalize();
        }

        for ( unsigned int i = NumberOfVertices - 1; i < NumberOfVertices + reminder; i++ )
        {
            Vertices[i] = Vertices[NumberOfVertices - 1];
        }

        WorldTriangles = new RTVector[NumberOfVertices + reminder];
        NumberOfWorldTrianglePacks = ( NumberOfVertices + reminder ) / 4;
        WorldTrianglePacks = (RTVectorPack*)_mm_malloc( NumberOfWorldTrianglePacks * sizeof( RTVectorPack ), 16 );
        updateWorldSpace();
    }
}

RTObject::~RTObject()
{
    delete [] Vertices;
    delete [] TriangleNormals;
    delete [] VertexNormals;
    delete [] WorldTriangles;
    _mm_free( WorldTrianglePacks );
}

bool RTObject::isLoaded()
{
    return LoadError.empty();
}

std::string RTObject::getLoadError()
{
    return LoadError;
}

const RTVector* RTObject::getVertices()
{
    return Vertices;
}

unsigned int RTObject::getNumberOfVertices()
{
    return NumberOfVertices;
}

const RTVector* RTObject::getWorldTriangles()
{
    return WorldTriangles;
}

void RTObject::setGLObject( std::shared_ptr<GLObject> glObject )
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

void RTObject::setName( const std::string &name )
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

std::string RTObject::getName()
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

RTVector RTObject::getNormal( unsigned int triangleIndex )
{
    return TriangleNormals[triangleIndex];
}

void RTObject::getNormalsForSmooth( unsigned int triangleIndex, RTVector &n0, RTVector &n1, RTVector &n2 )
{
    n0 = VertexNormals[ triangleIndex * 3 ];
    n1 = VertexNormals[ triangleIndex * 3 + 1 ];
    n2 = VertexNormals[ triangleIndex * 3 + 2 ];
}

void RTObject::updateWorldSpace()
{
    // Vertices and WorldTriangles are padded to a multiple of 12 (4 triangles)
    const unsigned int numberOfPaddedVertices = NumberOfWorldTrianglePacks * 4;

    for ( unsigned int i = 0; i < numberOfPaddedVertices; i += 3 )
    {
        RTVector v0 = ( Vertices[i] * Scale ) + Position;
        RTVector v1 = ( Vertices[i + 1] * Scale ) + Position;
        RTVector v2 = ( Vertices[i + 2] * Scale ) + Position;
        WorldTriangles[i]     = v0;
        WorldTriangles[i + 1] = v1 - v0;
        WorldTriangles[i + 2] = v2 - v0;
    }

    unsigned int packIndex = 0;
    for ( unsigned int i = 0; i < numberOfPaddedVertices; i += 12 )
    {
        // v0, v0v1 and v0v2 of 4 triangles
        WorldTrianglePacks[packIndex]   = RTVectorPack( WorldTriangles[i],     WorldTriangles[i + 3], WorldTriangles[i + 6], WorldTriangles[i + 9] );
        WorldTrianglePacks[packIndex+1] = RTVectorPack( WorldTriangles[i + 1], WorldTriangles[i + 4], WorldTriangles[i + 7], WorldTriangles[i + 10] );
        WorldTrianglePacks[packIndex+2] = RTVectorPack( WorldTriangles[i + 2], WorldTriangles[i + 5], WorldTriangles[i + 8], WorldTriangles[i + 11] );

        packIndex += 3;
    }
}

int RTObject::intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD )
{
    bool isect = false;

    if ( useSIMD )
    {
        RTRayPack rayPack( ray.Origin, ray.Direction );

        unsigned int triangleIndexPacked = 0;
        for( unsigned int i = 0; i < NumberOfWorldTrianglePacks; i += 3 )
        {
            const RTVectorPack &v0   = WorldTrianglePacks[ i ];
            const RTVectorPack &v0v1 = WorldTrianglePacks[ i + 1 ];
            const RTVectorPack &v0v2 = WorldTrianglePacks[ i + 2 ];

            static float max = std::numeric_limits<float>::max();
            __m128 packT = _mm_set1_ps( max );
            __m128 mask;
            __m128 packU;
            __m128 packV;

            if ( RTTriangle::intersectPack( rayPack, v0, v0v1, v0v2, packT, mask, packU, packV ) )
            {
                alignas( 16 ) float tArray[4];
                _mm_store_ps( tArray, packT );

                alignas( 16 ) float maskArray[4];
                _mm_store_ps( maskArray, mask );

                alignas( 16 ) float uArray[4];
                _mm_store_ps( uArray, packU );

                alignas( 16 ) float vArray[4];
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
            const RTVector &v0   = WorldTriangles[ i ];
            const RTVector &v0v1 = WorldTriangles[ i + 1 ];
            const RTVector &v0v2 = WorldTriangles[ i + 2 ];
            float t = std::numeric_limits<float>::max();
            float tempU;
            float tempV;
            if ( RTTriangle::intersect( ray, v0, v0v1, v0v2, t, tempU, tempV ) && t < distance )
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
