#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <sys/stat.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <climits>
#include <unistd.h>
#endif

#include "raytracer/rt_renderer.h"
#include "scene/rt_camera.h"
#include "scene/rt_scene.h"

namespace
{

enum ExitCode
{
    ExitSuccess = 0,
    ExitUsage   = 1,
    ExitScene   = 2,
    ExitOutput  = 3
};

const char *Usage =
    "Usage: raytracer-cli <scene.sc> [options]\n"
    "\n"
    "Renders a scene file (as saved by the raytracer GUI) to a PNG image.\n"
    "\n"
    "Options:\n"
    "  -o, --output <file.png>  output image (default: <scene name>.png in the current folder)\n"
    "  --width <pixels>         image width (default: 800)\n"
    "  --height <pixels>        image height (default: 600)\n"
    "  --threads <count>        render threads (default: all hardware threads)\n"
    "  --depth <count>          maximum trace depth, limits reflections (default: 3)\n"
    "  --scalar                 trace without SSE\n"
    "  --resources <folder>     folder with the obj/ mesh folder\n"
    "                           (default: the resources folder next to the executable)\n"
    "  -h, --help               show this help\n";

// Folder of the running executable
std::string executableDirectory()
{
#ifdef _WIN32
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA( nullptr, path, MAX_PATH );
    if ( length == 0 || length == MAX_PATH )
    {
        return ".";
    }
    std::string file( path, length );
#else
    char path[PATH_MAX];
    ssize_t length = readlink( "/proc/self/exe", path, sizeof( path ) - 1 );
    if ( length <= 0 )
    {
        return ".";
    }
    std::string file( path, static_cast<size_t>( length ) );
#endif
    size_t slash = file.find_last_of( "/\\" );
    return slash == std::string::npos ? "." : file.substr( 0, slash );
}

bool isDirectory( const std::string &path )
{
    struct stat info;
    return stat( path.c_str(), &info ) == 0 && ( info.st_mode & S_IFDIR ) != 0;
}

// File name without folder and extension
std::string baseName( const std::string &path )
{
    size_t slash = path.find_last_of( "/\\" );
    std::string name = slash == std::string::npos ? path : path.substr( slash + 1 );
    size_t dot = name.find_last_of( '.' );
    return ( dot == std::string::npos || dot == 0 ) ? name : name.substr( 0, dot );
}

bool parsePositive( const std::string &text, int &value )
{
    char *end = nullptr;
    errno = 0;
    long number = std::strtol( text.c_str(), &end, 10 );
    if ( errno != 0 || end == text.c_str() || *end != '\0' || number < 1 || number > 100000 )
    {
        return false;
    }
    value = static_cast<int>( number );
    return true;
}

int usageError( const std::string &message )
{
    std::cerr << "raytracer-cli: " << message << "\n\n" << Usage;
    return ExitUsage;
}

} // namespace

int main( int argc, char *argv[] )
{
    std::string scenePath;
    std::string outputPath;
    std::string resourcesDirectory;

    RTRenderSettings settings;
    unsigned int hardwareThreads = std::thread::hardware_concurrency();
    settings.NumberOfThreads = hardwareThreads > 0 ? static_cast<int>( hardwareThreads ) : 1;

    for ( int i = 1; i < argc; i++ )
    {
        const std::string arg = argv[i];

        if ( arg == "-h" || arg == "--help" )
        {
            std::cout << Usage;
            return ExitSuccess;
        }
        if ( arg == "--scalar" )
        {
            settings.UseSIMD = false;
            continue;
        }

        const bool takesValue = arg == "-o" || arg == "--output" || arg == "--resources" ||
                                arg == "--width" || arg == "--height" || arg == "--threads" || arg == "--depth";
        if ( takesValue )
        {
            if ( i + 1 >= argc )
            {
                return usageError( "missing value after " + arg );
            }
            const std::string value = argv[++i];

            if ( arg == "-o" || arg == "--output" )
            {
                outputPath = value;
            }
            else if ( arg == "--resources" )
            {
                resourcesDirectory = value;
            }
            else
            {
                int number = 0;
                if ( !parsePositive( value, number ) )
                {
                    return usageError( arg + " needs a positive whole number, got '" + value + "'" );
                }
                if ( arg == "--width" )        settings.Width = number;
                else if ( arg == "--height" )  settings.Height = number;
                else if ( arg == "--threads" ) settings.NumberOfThreads = number;
                else                           settings.MaxTraceDepth = number;
            }
            continue;
        }

        if ( arg.size() > 1 && arg[0] == '-' )
        {
            return usageError( "unknown option " + arg );
        }
        if ( !scenePath.empty() )
        {
            return usageError( "only one scene file can be rendered at a time" );
        }
        scenePath = arg;
    }

    if ( scenePath.empty() )
    {
        return usageError( "no scene file given" );
    }

    if ( resourcesDirectory.empty() )
    {
        resourcesDirectory = executableDirectory() + "/resources";
    }
    const std::string meshDirectory = resourcesDirectory + "/obj";
    if ( !isDirectory( meshDirectory ) )
    {
        std::cerr << "raytracer-cli: mesh folder not found: " << meshDirectory << " (use --resources <folder>)\n";
        return ExitScene;
    }

    RTCamera camera;
    RTScene  scene;
    scene.setCamera( &camera );
    scene.setMeshDirectory( meshDirectory );

    std::string error;
    if ( !scene.openScene( scenePath, error ) )
    {
        std::cerr << "raytracer-cli: " << error << "\n";
        return ExitScene;
    }

    RTRenderer renderer( settings );
    RTImage image = renderer.render( &scene, camera );

    if ( outputPath.empty() )
    {
        outputPath = baseName( scenePath ) + ".png";
    }
    if ( !image.savePNG( outputPath ) )
    {
        std::cerr << "raytracer-cli: cannot write " << outputPath << "\n";
        return ExitOutput;
    }

    std::cout << "Rendered " << scenePath << " (" << scene.getNumberOfObjects() << " objects, "
              << scene.getNumberOfLights() << " lights) to " << outputPath << ": "
              << settings.Width << "x" << settings.Height << ", " << settings.NumberOfThreads << " threads, "
              << ( settings.UseSIMD ? "SSE" : "scalar" ) << ", " << renderer.getRenderTime() << " s\n";

    return ExitSuccess;
}
