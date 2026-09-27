#include <algorithm>
#include <cmath>
#include <limits>
#include "rt_bvh.h"
#include "rt_triangle.h"
#include "objects/rt_object.h"
#include "scene/rt_scene.h"

namespace
{

const float        Infinity     = std::numeric_limits<float>::infinity();
const unsigned int LeafSize     = 4;    // triangles in a leaf: one SIMD pack
const int          NumberOfBins = 16;   // split candidates per axis are the planes between the bins
const int          MaxSAHDepth  = 64;   // deeper nodes are split in half, which bounds the tree depth
const int          StackSize    = 128;  // more than the tree depth: MaxSAHDepth plus halving the rest

struct Hit
{
    float        Distance;
    float        U;
    float        V;
    unsigned int Object;
    unsigned int Triangle;
    bool         Found;
};

// Hits at the same distance go to the lowest object, then the lowest triangle: the one that testing
// every object in order with RTObject::intersect keeps, so the BVH finds exactly the same hits
inline bool isCloser( float t, unsigned int object, unsigned int triangle, const Hit &hit )
{
    if ( t != hit.Distance )
    {
        return t < hit.Distance;
    }
    return hit.Found && ( object != hit.Object ? object < hit.Object : triangle < hit.Triangle );
}

inline void setHit( Hit &hit, float t, float u, float v, unsigned int object, unsigned int triangle )
{
    hit.Distance = t;
    hit.U        = u;
    hit.V        = v;
    hit.Object   = object;
    hit.Triangle = triangle;
    hit.Found    = true;
}

inline float coordinate( const RTVector &v, int axis )
{
    return axis == 0 ? v.x() : ( axis == 1 ? v.y() : v.z() );
}

inline void grow( float *mn, float *mx, const float *otherMin, const float *otherMax )
{
    for ( int a = 0; a < 3; a++ )
    {
        mn[a] = std::min( mn[a], otherMin[a] );
        mx[a] = std::max( mx[a], otherMax[a] );
    }
}

// Half the surface area of a box
inline float surfaceArea( const float *mn, const float *mx )
{
    float dx = mx[0] - mn[0];
    float dy = mx[1] - mn[1];
    float dz = mx[2] - mn[2];
    return dx * dy + dy * dz + dz * dx;
}

// A leaf costs one pack test for 1 to 4 triangles
inline float packs( unsigned int triangles )
{
    return static_cast<float>( ( triangles + LeafSize - 1 ) / LeafSize );
}

inline int binIndex( float centroid, float centroidMin, float scale )
{
    return std::min( static_cast<int>( ( centroid - centroidMin ) * scale ), NumberOfBins - 1 );
}

// 1 / d that stays finite for d = 0, so the slab test never multiplies 0 by infinity
inline float safeInverse( float d )
{
    const float tiny = 1e-20f;
    if ( std::fabs( d ) < tiny )
    {
        d = std::signbit( d ) ? -tiny : tiny;
    }
    return 1.0f / d;
}

// Slab test: the distance where the ray enters the box, or infinity if it misses the box before tMax.
// nearPlane is 0 on the axes the ray goes along in the positive direction (it enters at Min), 1 on the others.
inline float boxEntry( const float *mn, const float *mx, const float *origin, const float *invDir, const int *nearPlane, float tMax )
{
    const float *planes[2] = { mn, mx };
    float tNear = 0.0f;
    float tFar  = tMax;
    for ( int a = 0; a < 3; a++ )
    {
        tNear = std::max( tNear, ( planes[nearPlane[a]][a] - origin[a] ) * invDir[a] );
        tFar  = std::min( tFar, ( planes[1 - nearPlane[a]][a] - origin[a] ) * invDir[a] );
    }
    return tNear <= tFar ? tNear : Infinity;
}

} // namespace

void RTBVH::build( RTScene *scene )
{
    Nodes.clear();
    Packs.clear();
    Triangles.clear();
    SlotObjects.clear();
    SlotTriangles.clear();
    Objects.clear();

    std::vector<BuildTriangle> triangles;
    for ( size_t o = 0; o < scene->getNumberOfObjects(); o++ )
    {
        RTObject *object = scene->getObject( static_cast<int>( o ) );
        Objects.push_back( object );

        const RTVector *world = object->getWorldTriangles();
        const unsigned int numberOfTriangles = object->getNumberOfVertices() / 3;
        for ( unsigned int i = 0; i < numberOfTriangles; i++ )
        {
            const RTVector &v0 = world[i * 3];
            const RTVector v1 = v0 + world[i * 3 + 1];
            const RTVector v2 = v0 + world[i * 3 + 2];

            BuildTriangle triangle;
            for ( int a = 0; a < 3; a++ )
            {
                const float c0 = coordinate( v0, a );
                const float c1 = coordinate( v1, a );
                const float c2 = coordinate( v2, a );
                triangle.Min[a]      = std::min( c0, std::min( c1, c2 ) );
                triangle.Max[a]      = std::max( c0, std::max( c1, c2 ) );
                triangle.Centroid[a] = ( triangle.Min[a] + triangle.Max[a] ) * 0.5f;
            }
            triangle.Object = static_cast<unsigned int>( o );
            triangle.Index  = i;
            triangles.push_back( triangle );
        }
    }

    if ( triangles.empty() )
    {
        return;
    }

    // A binary tree whose leaves have at least 1 triangle has fewer than 2 nodes per triangle
    Nodes.reserve( 2 * triangles.size() );
    Nodes.push_back( Node() );
    subdivide( triangles, 0, 0, static_cast<unsigned int>( triangles.size() ), 0 );
}

void RTBVH::subdivide( std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count, int depth )
{
    float mn[3]          = { Infinity, Infinity, Infinity };
    float mx[3]          = { -Infinity, -Infinity, -Infinity };
    float centroidMin[3] = { Infinity, Infinity, Infinity };
    float centroidMax[3] = { -Infinity, -Infinity, -Infinity };
    for ( unsigned int i = first; i < first + count; i++ )
    {
        grow( mn, mx, triangles[i].Min, triangles[i].Max );
        grow( centroidMin, centroidMax, triangles[i].Centroid, triangles[i].Centroid );
    }

    // Padded, so rounding in the slab test never skips a triangle that the triangle test hits
    for ( int a = 0; a < 3; a++ )
    {
        Nodes[nodeIndex].Min[a] = mn[a] - ( 1e-4f + 1e-6f * std::fabs( mn[a] ) );
        Nodes[nodeIndex].Max[a] = mx[a] + ( 1e-4f + 1e-6f * std::fabs( mx[a] ) );
    }

    if ( count <= LeafSize )
    {
        makeLeaf( triangles, nodeIndex, first, count );
        return;
    }

    // Surface area heuristic: on each axis, try the planes between NumberOfBins bins of the triangle centroids
    int   bestAxis  = -1;
    int   bestPlane = 0;    // the left side gets bins 0 to bestPlane
    float bestCost  = Infinity;
    for ( int axis = 0; axis < 3 && depth < MaxSAHDepth; axis++ )
    {
        const float extent = centroidMax[axis] - centroidMin[axis];
        if ( !( extent > 0.0f ) )
        {
            continue;
        }
        const float scale = NumberOfBins / extent;

        unsigned int binCount[NumberOfBins] = {};
        float binMin[NumberOfBins][3];
        float binMax[NumberOfBins][3];
        for ( int b = 0; b < NumberOfBins; b++ )
        {
            for ( int a = 0; a < 3; a++ )
            {
                binMin[b][a] = Infinity;
                binMax[b][a] = -Infinity;
            }
        }
        for ( unsigned int i = first; i < first + count; i++ )
        {
            const int b = binIndex( triangles[i].Centroid[axis], centroidMin[axis], scale );
            binCount[b]++;
            grow( binMin[b], binMax[b], triangles[i].Min, triangles[i].Max );
        }

        // Right side of each plane, from the last bin
        unsigned int rightCount[NumberOfBins - 1];
        float        rightArea[NumberOfBins - 1];
        float        rightMin[3] = { Infinity, Infinity, Infinity };
        float        rightMax[3] = { -Infinity, -Infinity, -Infinity };
        unsigned int rightTotal  = 0;
        for ( int plane = NumberOfBins - 2; plane >= 0; plane-- )
        {
            rightTotal += binCount[plane + 1];
            grow( rightMin, rightMax, binMin[plane + 1], binMax[plane + 1] );
            rightCount[plane] = rightTotal;
            rightArea[plane]  = rightTotal > 0 ? surfaceArea( rightMin, rightMax ) : 0.0f;
        }

        float        leftMin[3] = { Infinity, Infinity, Infinity };
        float        leftMax[3] = { -Infinity, -Infinity, -Infinity };
        unsigned int leftTotal  = 0;
        for ( int plane = 0; plane < NumberOfBins - 1; plane++ )
        {
            leftTotal += binCount[plane];
            grow( leftMin, leftMax, binMin[plane], binMax[plane] );
            if ( leftTotal == 0 || rightCount[plane] == 0 )
            {
                continue;
            }
            const float cost = surfaceArea( leftMin, leftMax ) * packs( leftTotal ) + rightArea[plane] * packs( rightCount[plane] );
            if ( cost < bestCost )
            {
                bestCost  = cost;
                bestAxis  = axis;
                bestPlane = plane;
            }
        }
    }

    std::vector<BuildTriangle>::iterator begin = triangles.begin() + first;
    std::vector<BuildTriangle>::iterator end   = begin + count;
    unsigned int leftCount = 0;
    if ( bestAxis >= 0 )
    {
        const float centroidMinAxis = centroidMin[bestAxis];
        const float scale = NumberOfBins / ( centroidMax[bestAxis] - centroidMin[bestAxis] );
        std::vector<BuildTriangle>::iterator middle = std::partition( begin, end, [bestAxis, bestPlane, centroidMinAxis, scale]( const BuildTriangle &triangle )
        {
            return binIndex( triangle.Centroid[bestAxis], centroidMinAxis, scale ) <= bestPlane;
        } );
        leftCount = static_cast<unsigned int>( middle - begin );
    }
    if ( leftCount == 0 || leftCount == count )
    {
        // All centroids in one point, or too deep for the heuristic: split in half along the widest axis
        int axis = 0;
        for ( int a = 1; a < 3; a++ )
        {
            if ( centroidMax[a] - centroidMin[a] > centroidMax[axis] - centroidMin[axis] )
            {
                axis = a;
            }
        }
        leftCount = count / 2;
        std::nth_element( begin, begin + leftCount, end, [axis]( const BuildTriangle &a, const BuildTriangle &b )
        {
            return a.Centroid[axis] < b.Centroid[axis];
        } );
    }

    const unsigned int left = static_cast<unsigned int>( Nodes.size() );
    Nodes.push_back( Node() );
    Nodes.push_back( Node() );
    Nodes[nodeIndex].LeftOrFirst = left;
    Nodes[nodeIndex].Count       = 0;

    subdivide( triangles, left, first, leftCount, depth + 1 );
    subdivide( triangles, left + 1, first + leftCount, count - leftCount, depth + 1 );
}

void RTBVH::makeLeaf( const std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count )
{
    Nodes[nodeIndex].LeftOrFirst = static_cast<unsigned int>( SlotObjects.size() );
    Nodes[nodeIndex].Count       = count;

    const RTVector zero( 0.0f, 0.0f, 0.0f );
    RTVector slots[3][LeafSize];    // v0, v0v1 and v0v2 of the 4 slots
    for ( unsigned int j = 0; j < LeafSize; j++ )
    {
        // Unused slots repeat the first triangle's v0 with zero edges: det is 0, so they're never hit
        const BuildTriangle &triangle = triangles[first + ( j < count ? j : 0 )];
        const RTVector *world = Objects[triangle.Object]->getWorldTriangles() + triangle.Index * 3;
        slots[0][j] = world[0];
        slots[1][j] = j < count ? world[1] : zero;
        slots[2][j] = j < count ? world[2] : zero;

        for ( int k = 0; k < 3; k++ )
        {
            Triangles.push_back( slots[k][j] );
        }
        SlotObjects.push_back( triangle.Object );
        SlotTriangles.push_back( triangle.Index );
    }

    for ( int k = 0; k < 3; k++ )
    {
        Packs.push_back( RTVectorPack( slots[k][0], slots[k][1], slots[k][2], slots[k][3] ) );
    }
}

RTObject* RTBVH::intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD ) const
{
    if ( Nodes.empty() )
    {
        return nullptr;
    }

    const float origin[3]    = { ray.Origin.x(), ray.Origin.y(), ray.Origin.z() };
    const float invDir[3]    = { safeInverse( ray.Direction.x() ), safeInverse( ray.Direction.y() ), safeInverse( ray.Direction.z() ) };
    const int   nearPlane[3] = { invDir[0] >= 0.0f ? 0 : 1, invDir[1] >= 0.0f ? 0 : 1, invDir[2] >= 0.0f ? 0 : 1 };
    const RTRayPack rayPack( ray.Origin, ray.Direction );

    Hit hit;
    hit.Distance = distance;
    hit.Found    = false;

    if ( boxEntry( Nodes[0].Min, Nodes[0].Max, origin, invDir, nearPlane, hit.Distance ) == Infinity )
    {
        return nullptr;
    }

    struct Entry
    {
        unsigned int Node;
        float        Distance;
    };
    Entry stack[StackSize];
    int stackSize = 0;
    unsigned int nodeIndex = 0;
    while ( true )
    {
        const Node &node = Nodes[nodeIndex];
        if ( node.Count > 0 )
        {
            if ( useSIMD )
            {
                const unsigned int pack = node.LeftOrFirst / LeafSize * 3;
                __m128 packT;
                __m128 mask;
                __m128 packU;
                __m128 packV;
                if ( RTTriangle::intersectPack( rayPack, Packs[pack], Packs[pack + 1], Packs[pack + 2], packT, mask, packU, packV ) )
                {
                    alignas( 16 ) float tArray[4];
                    _mm_store_ps( tArray, packT );

                    alignas( 16 ) float maskArray[4];
                    _mm_store_ps( maskArray, mask );

                    alignas( 16 ) float uArray[4];
                    _mm_store_ps( uArray, packU );

                    alignas( 16 ) float vArray[4];
                    _mm_store_ps( vArray, packV );

                    for ( unsigned int j = 0; j < node.Count; j++ )
                    {
                        const unsigned int slot = node.LeftOrFirst + j;
                        if ( tArray[j] > 0 && maskArray[j] && isCloser( tArray[j], SlotObjects[slot], SlotTriangles[slot], hit ) )
                        {
                            setHit( hit, tArray[j], uArray[j], vArray[j], SlotObjects[slot], SlotTriangles[slot] );
                        }
                    }
                }
            }
            else
            {
                for ( unsigned int slot = node.LeftOrFirst; slot < node.LeftOrFirst + node.Count; slot++ )
                {
                    float t = std::numeric_limits<float>::max();
                    float tempU;
                    float tempV;
                    if ( RTTriangle::intersect( ray, Triangles[slot * 3], Triangles[slot * 3 + 1], Triangles[slot * 3 + 2], t, tempU, tempV ) &&
                         isCloser( t, SlotObjects[slot], SlotTriangles[slot], hit ) )
                    {
                        setHit( hit, t, tempU, tempV, SlotObjects[slot], SlotTriangles[slot] );
                    }
                }
            }
        }
        else
        {
            // Visit the nearer child first; the other one waits on the stack
            unsigned int nearChild    = node.LeftOrFirst;
            unsigned int farChild     = nearChild + 1;
            float        nearDistance = boxEntry( Nodes[nearChild].Min, Nodes[nearChild].Max, origin, invDir, nearPlane, hit.Distance );
            float        farDistance  = boxEntry( Nodes[farChild].Min, Nodes[farChild].Max, origin, invDir, nearPlane, hit.Distance );
            if ( farDistance < nearDistance )
            {
                std::swap( nearChild, farChild );
                std::swap( nearDistance, farDistance );
            }
            if ( nearDistance != Infinity )
            {
                if ( farDistance != Infinity )
                {
                    stack[stackSize].Node     = farChild;
                    stack[stackSize].Distance = farDistance;
                    stackSize++;
                }
                nodeIndex = nearChild;
                continue;
            }
        }

        // The next node that can still hold a closer hit, or an equally close one, which can win the tie
        bool found = false;
        while ( stackSize > 0 )
        {
            stackSize--;
            if ( stack[stackSize].Distance <= hit.Distance )
            {
                nodeIndex = stack[stackSize].Node;
                found     = true;
                break;
            }
        }
        if ( !found )
        {
            break;
        }
    }

    if ( !hit.Found )
    {
        return nullptr;
    }
    distance      = hit.Distance;
    triangleIndex = hit.Triangle;
    u             = hit.U;
    v             = hit.V;
    return Objects[hit.Object];
}
