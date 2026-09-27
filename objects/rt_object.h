#ifndef RT_OBJECT_H
#define RT_OBJECT_H

#include <memory>
#include <string>
#include <xmmintrin.h>
#include "raytracer/rt_ray.h"
#include "math/rt_vector.h"
#include "raytracer/rt_ray_pack.h"
#include "math/rt_vector_pack.h"

class GLObject;

class RTObject
{
public: // functions
    RTObject( std::string objFileName, RTVector position );
    ~RTObject();

    // Owns its vertex and normal arrays
    RTObject( const RTObject& ) = delete;
    RTObject& operator=( const RTObject& ) = delete;

    enum class RTMaterialType { Diffuse = 0, Specular, DiffuseAndSpecular, Reflective, Size };
    enum class RTObjectType { Plane = 0, Sphere, Cube, Pyramid, Cylinder, Cone, Tours, Obj, Size };

    RTVector getNormal( unsigned int triangleIndex );
    void getNormalsForSmooth( unsigned int triangleIndex, RTVector &n0, RTVector &n1, RTVector &n2 );
    int intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD );

    // Rebuilds the world space triangles that intersect() tests from the position and scale.
    // RTRenderer::render calls it for every object; other callers of intersect() must call it after a move.
    void updateWorldSpace();

    // False if the .obj file could not be loaded; getLoadError() tells why
    bool        isLoaded();
    std::string getLoadError();

    // Triangle list (3 vertices per triangle) in object space
    const RTVector* getVertices();
    unsigned int    getNumberOfVertices();

    // The world space triangles of updateWorldSpace(): v0, v1 - v0 and v2 - v0 for each triangle
    const RTVector* getWorldTriangles();

    inline GLObject* getGlObject()
    {
        return Globject.get();
    }

    void   setGLObject( std::shared_ptr<GLObject> glObject );

    void   updatePosition( RTVector position );
    void   movePosition( RTVector movementStep );
    void   setScale( RTVector scale );

    void setName( const std::string &name );
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
    std::string getName();
    RTMaterialType getMaterialType();
    RTObjectType getObjectType();
    RTVector getColor();
    float getDiffuse();
    float getSpecular();
    float getSpecularExponent();
    float getReflection();
    bool getSmoothShading();

private:
    std::string Name;
    std::string LoadError;

    RTVector  Position;
    RTVector  Scale;
    RTVector  Color;
    float     Diffuse;          // |1 - Specular|
    float     Specular;         // |1 - Diffuse|
    float     SpecularExponent;
    float     Reflection;       // <= 1 && >= 0
    bool      SmoothShading;

    // The GUI creates it where GLObject is complete; a shared_ptr keeps that deleter,
    // so the Qt-free core can destroy it while only forward-declaring GLObject
    std::shared_ptr<GLObject> Globject;

    RTMaterialType MaterialType;
    RTObjectType ObjectType;

    RTVector *Vertices;
    RTVector *TriangleNormals;
    RTVector *VertexNormals;
    unsigned int NumberOfVertices;

    // World space triangles from updateWorldSpace(): the first vertex and the edges to the
    // other two (v0, v1 - v0, v2 - v0), padded like Vertices
    RTVector *WorldTriangles;

    // The same for the SIMD path: 3 packs (v0, v0v1, v0v2) per 4 triangles
    RTVectorPack *WorldTrianglePacks;
    unsigned int NumberOfWorldTrianglePacks;
};

#endif // RT_OBJECT_H
