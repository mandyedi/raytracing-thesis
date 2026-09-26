#ifndef RT_SCENE_H
#define RT_SCENE_H

class RTObject;
class RTLight;
class RTCamera;
namespace Ui
{
    class MainWindow;
};

class RTScene
{
public:
    RTScene();
    ~RTScene();

    // Object
    void addObject(RTObject *object);
    void setActiveObject(QString name);
    void noActiveObject();

    size_t           getNumberOfObjects();
    RTObject*        getObject(const int &i);
    RTObject*        getActiveObject();
    QList<RTObject*> getObjects();
    void             removeActiveObject();

    QString addObj( std::string &objFileName );
    QString addSphere();
    QString addPlane();
    QString addCube();
    QString addPyramid();
    QString addCylinder();
    QString addCone();
    QString addTorus();

    // Light
    void setActiveLight( QString name );
    void noActiveLight();
    QString addPointLight();
    QString addDistantLight();

    size_t        getNumberOfLights();
    RTLight*        getLight( const int i );
    RTLight*        getActiveLight();
    QList<RTLight*> getLights();
    void          removeActiveLight();

    // Common
    void setCamera( RTCamera *camera );
    void removeAll();
    void saveScene();
    void openScene( FILE *file, Ui::MainWindow *ui );

private:
    RTCamera         *Camera;
    QList<RTObject*>  Objects;
    RTObject         *ActiveObject;
    int               ActiveObjectIndex;
    QList<RTLight*>   Lights;
    RTLight          *ActiveLight;
    int               ActiveLightIndex;
    unsigned int      ObjectNameCount;

    static std::string getResourceFile( const QString &prefix, const QString &resource );
};

#endif // RT_SCENE_H
