#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include <QString>
#include <QGLBuffer>
#include <QTextStream>
#include <xmmintrin.h>
#include "opengl/gl_object.h"
#include "raytracer/rt_ray.h"
#include "math/rt_vector.h"
#include "raytracer/rt_ray_pack.h"
#include "math/rt_vector_pack.h"

class RTObject
{
public: // functions
    RTObject( std::string objFileName, RTVector position );
    ~RTObject();

    enum class RTMaterialType { Diffuse = 0, Specular, DiffuseAndSpecular, Reflective, Size };
    enum class RTObjectType { Plane = 0, Sphere, Cube, Pyramid, Cylinder, Cone, Tours, Obj, Size };

    RTVector getNormal( unsigned int triangleIndex, float u, float v );
    void getNormalsForSmooth( unsigned int triangleIndex, RTVector &n0, RTVector &n1, RTVector &n2 );
    int intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD );

    inline GLObject* getGlObject()
    {
        return Globject;
    }

    void   setGLObject( GLObject *glObject );

    void   updatePosition( RTVector position );
    void   movePosition( RTVector movementStep );
    void   setScale( RTVector scale );

    void setName( QString name );
    void setMaterialType( RTMaterialType type );
    void setObjectType( RTObjectType type );
    void setColor( RTVector color );
    void setDiffuse( float diffuse );
    void setSpecular( float specular );
    void setSpecularExponent( float specularExponent );
    void setReflection( float reflection );
    void setSmoothShading( bool enabled );

    RTVector getPosition();
    RTVector getScale();
    QString getName();
    RTMaterialType getMaterialType();
    RTObjectType getObjectType();
    RTVector getColor();
    float getDiffuse();
    float getSpecular();
    float getSpecularExponent();
    float getReflection();
    bool getSmoothShading();

private:
    QString    Name;

    RTVector  Position;
    RTVector  Scale;
    RTVector  Color;
    float     Diffuse;          // |1 - Specular|
    float     Specular;         // |1 - Diffuse|
    float     SpecularExponent;
    float     Reflection;       // <= 1 && >= 0
    bool      SmoothShading;

    GLObject *Globject;

    RTMaterialType MaterialType;
    RTObjectType ObjectType;

    RTVector *Vertices;
    RTVector *TriangleNormals;
    RTVector *VertexNormals;
    unsigned int NumberOfVertices;

    bool intersectTriangle( const RTRay &ray, const RTVector &v0, const RTVector &v1, const RTVector &v2, float &t, float &u, float &v );

    RTVectorPack *VerticesSIMDPack;
    unsigned int NumberOfVertexPacks;

    bool intersectTriangleSIMDPacked( const RTVectorPack &scalePack, const RTVectorPack &positionPack, const RTRayPack &rayPack, const RTVectorPack &vPack0, const RTVectorPack &vPack1, const RTVectorPack &vPack2, __m128 &tPack, __m128 &maskValid, __m128 &uPack, __m128 &vPack );
};

#endif // RT_OBJECT_H
