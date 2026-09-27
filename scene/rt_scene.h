#ifndef RT_SCENE_H
#define RT_SCENE_H

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

class RTObject;
class RTLight;
class RTCamera;

class RTScene
{
public:
    RTScene();
    ~RTScene();

    // The scene owns its objects and lights
    RTScene( const RTScene& ) = delete;
    RTScene& operator=( const RTScene& ) = delete;

    // Object
    void addObject( RTObject *object );
    void setActiveObject( const std::string &name );
    void noActiveObject();

    size_t                        getNumberOfObjects();
    RTObject*                     getObject( const int &i );
    RTObject*                     getActiveObject();
    const std::vector<RTObject*>& getObjects();
    void                          removeActiveObject();

    std::string addObj( const std::string &objFileName );
    std::string addSphere();
    std::string addPlane();
    std::string addCube();
    std::string addPyramid();
    std::string addCylinder();
    std::string addCone();
    std::string addTorus();

    // Light
    void setActiveLight( const std::string &name );
    void noActiveLight();
    std::string addPointLight();
    std::string addDistantLight();

    size_t                       getNumberOfLights();
    RTLight*                     getLight( const int i );
    RTLight*                     getActiveLight();
    const std::vector<RTLight*>& getLights();
    void                         removeActiveLight();

    // Common
    void setCamera( RTCamera *camera );
    void removeAll();

    // Folder with the meshes of the primitives (sphere.obj, cube.obj, ...)
    void        setMeshDirectory( const std::string &directory );
    std::string getMeshFile( const std::string &fileName );

    bool saveScene( const std::string &fileName );

    // Adds the camera, objects and lights of a scene file. On failure, error tells the file and line.
    bool openScene( const std::string &fileName, std::string &error );

private:
    RTCamera               *Camera;
    std::vector<RTObject*>  Objects;
    RTObject               *ActiveObject;
    int                     ActiveObjectIndex;
    std::vector<RTLight*>   Lights;
    RTLight                *ActiveLight;
    int                     ActiveLightIndex;
    unsigned int            ObjectNameCount;
    std::string             MeshDirectory;

    std::string addMesh( RTObject *mesh, const std::string &name );

    // Read one line of a scene file; return an error message, or an empty string on success
    std::string readCamera( std::istream &in );
    std::string readObject( std::istream &in );
    std::string readLight( std::istream &in );
};

#endif // RT_SCENE_H
