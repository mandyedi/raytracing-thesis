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
const int          MaxSAHDepth  = 48;   // deeper binary nodes are split in half, so no path is deeper than 48 + 31
const int          StackSize    = 256;  // each level of the 4-wide tree leaves at most 3 children waiting: 3 * 79 + 1

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

// Slab tests of a node's 4 child boxes (Node::Bounds): returns a bit mask of the boxes the ray enters before
// tMax, and where it enters each one in tNear. On each axis the ray enters a box at the plane in row nearRow
// (Min, or Max if it goes in the negative direction) and leaves it at the one in row farRow.
inline int testChildBoxes( const float bounds[6][4], const float *origin, const float *invDir, const int *nearRow, const int *farRow, float tMax, float *tNear )
{
    int mask = 0;
    for ( int i = 0; i < 4; i++ )
    {
        // Missing children come last and have inverted boxes
        if ( bounds[0][i] == Infinity )
        {
            break;
        }
        float tEnter = 0.0f;
        float tExit  = tMax;
        for ( int a = 0; a < 3; a++ )
        {
            tEnter = std::max( tEnter, ( bounds[nearRow[a]][i] - origin[a] ) * invDir[a] );
            tExit  = std::min( tExit, ( bounds[farRow[a]][i] - origin[a] ) * invDir[a] );
        }
        tNear[i] = tEnter;
        mask |= ( tEnter <= tExit ? 1 : 0 ) << i;
    }
    return mask;
}

// The same for the SIMD path: one SSE lane per child box
inline int testChildBoxesSIMD( const float bounds[6][4], const __m128 *origin, const __m128 *invDir, const int *nearRow, const int *farRow, float tMax, float *tNear )
{
    __m128 tEnter = _mm_setzero_ps();
    __m128 tExit  = _mm_set1_ps( tMax );
    for ( int a = 0; a < 3; a++ )
    {
        tEnter = _mm_max_ps( tEnter, _mm_mul_ps( _mm_sub_ps( _mm_load_ps( bounds[nearRow[a]] ), origin[a] ), invDir[a] ) );
        tExit  = _mm_min_ps( tExit, _mm_mul_ps( _mm_sub_ps( _mm_load_ps( bounds[farRow[a]] ), origin[a] ), invDir[a] ) );
    }
    _mm_store_ps( tNear, tEnter );
    return _mm_movemask_ps( _mm_cmple_ps( tEnter, tExit ) );
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

    // First a binary tree; a binary tree whose leaves have at least 1 triangle has fewer than 2 nodes per triangle
    std::vector<BuildNode> buildNodes;
    buildNodes.reserve( 2 * triangles.size() );
    buildNodes.push_back( BuildNode() );
    subdivide( buildNodes, triangles, 0, 0, static_cast<unsigned int>( triangles.size() ), 0 );

    collapse( buildNodes, 0 );
}

void RTBVH::subdivide( std::vector<BuildNode> &buildNodes, std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count, int depth )
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
        buildNodes[nodeIndex].Min[a] = mn[a] - ( 1e-4f + 1e-6f * std::fabs( mn[a] ) );
        buildNodes[nodeIndex].Max[a] = mx[a] + ( 1e-4f + 1e-6f * std::fabs( mx[a] ) );
    }

    if ( count <= LeafSize )
    {
        makeLeaf( buildNodes, triangles, nodeIndex, first, count );
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

    const unsigned int left = static_cast<unsigned int>( buildNodes.size() );
    buildNodes.push_back( BuildNode() );
    buildNodes.push_back( BuildNode() );
    buildNodes[nodeIndex].LeftOrFirst = left;
    buildNodes[nodeIndex].Count       = 0;

    subdivide( buildNodes, triangles, left, first, leftCount, depth + 1 );
    subdivide( buildNodes, triangles, left + 1, first + leftCount, count - leftCount, depth + 1 );
}

void RTBVH::makeLeaf( std::vector<BuildNode> &buildNodes, const std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count )
{
    buildNodes[nodeIndex].LeftOrFirst = static_cast<unsigned int>( SlotObjects.size() );
    buildNodes[nodeIndex].Count       = count;

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

unsigned int RTBVH::collapse( const std::vector<BuildNode> &buildNodes, unsigned int buildIndex )
{
    // Start with the binary node's children (or the node itself, a leaf root), then replace the largest
    // inner child with its two children until there are 4
    unsigned int children[4];
    int numberOfChildren = 0;
    const BuildNode &buildNode = buildNodes[buildIndex];
    if ( buildNode.Count > 0 )
    {
        children[numberOfChildren++] = buildIndex;
    }
    else
    {
        children[numberOfChildren++] = buildNode.LeftOrFirst;
        children[numberOfChildren++] = buildNode.LeftOrFirst + 1;
    }
    while ( numberOfChildren < 4 )
    {
        int   open     = -1;
        float openArea = -1.0f;
        for ( int i = 0; i < numberOfChildren; i++ )
        {
            const BuildNode &child = buildNodes[children[i]];
            if ( child.Count == 0 && surfaceArea( child.Min, child.Max ) > openArea )
            {
                open     = i;
                openArea = surfaceArea( child.Min, child.Max );
            }
        }
        if ( open < 0 )
        {
            break;
        }
        const unsigned int left = buildNodes[children[open]].LeftOrFirst;
        children[open] = left;
        children[numberOfChildren++] = left + 1;
    }

    // Written after the recursion, which can reallocate Nodes
    const unsigned int index = static_cast<unsigned int>( Nodes.size() );
    Nodes.push_back( Node() );
    Node node;
    for ( int i = 0; i < 4; i++ )
    {
        if ( i < numberOfChildren )
        {
            const BuildNode &child = buildNodes[children[i]];
            for ( int a = 0; a < 3; a++ )
            {
                node.Bounds[a][i]     = child.Min[a];
                node.Bounds[a + 3][i] = child.Max[a];
            }
            node.Child[i] = child.Count > 0 ? child.LeftOrFirst : collapse( buildNodes, children[i] );
            node.Count[i] = child.Count;
        }
        else
        {
            for ( int a = 0; a < 3; a++ )
            {
                node.Bounds[a][i]     = Infinity;
                node.Bounds[a + 3][i] = -Infinity;
            }
            node.Child[i] = 0;
            node.Count[i] = 0;
        }
    }
    Nodes[index] = node;
    return index;
}

RTObject* RTBVH::intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD ) const
{
    if ( Nodes.empty() )
    {
        return nullptr;
    }

    const float  origin[3]  = { ray.Origin.x(), ray.Origin.y(), ray.Origin.z() };
    const float  invDir[3]  = { safeInverse( ray.Direction.x() ), safeInverse( ray.Direction.y() ), safeInverse( ray.Direction.z() ) };
    const __m128 origin4[3] = { _mm_set1_ps( origin[0] ), _mm_set1_ps( origin[1] ), _mm_set1_ps( origin[2] ) };
    const __m128 invDir4[3] = { _mm_set1_ps( invDir[0] ), _mm_set1_ps( invDir[1] ), _mm_set1_ps( invDir[2] ) };
    const RTRayPack rayPack( ray.Origin, ray.Direction );

    // The rows of Node::Bounds where the ray enters and leaves the boxes on each axis:
    // Min (row a) and Max (row a + 3), the other way around if it goes in the negative direction
    int nearRow[3];
    int farRow[3];
    for ( int a = 0; a < 3; a++ )
    {
        nearRow[a] = invDir[a] >= 0.0f ? a : a + 3;
        farRow[a]  = invDir[a] >= 0.0f ? a + 3 : a;
    }

    Hit hit;
    hit.Distance = distance;
    hit.Found    = false;

    // A node or a leaf to visit, with the distance where the ray enters its box.
    // Count > 0 is a leaf, whose first triangle slot is in Child; 0 is a node.
    struct Entry
    {
        unsigned int Child;
        unsigned int Count;
        float        Distance;
    };
    Entry stack[StackSize];
    int stackSize = 0;
    Entry entry;
    entry.Child    = 0;
    entry.Count    = 0;
    entry.Distance = 0.0f;
    while ( true )
    {
        if ( entry.Count > 0 )
        {
            if ( useSIMD )
            {
                const unsigned int pack = entry.Child / LeafSize * 3;
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

                    for ( unsigned int j = 0; j < entry.Count; j++ )
                    {
                        const unsigned int slot = entry.Child + j;
                        if ( tArray[j] > 0 && maskArray[j] && isCloser( tArray[j], SlotObjects[slot], SlotTriangles[slot], hit ) )
                        {
                            setHit( hit, tArray[j], uArray[j], vArray[j], SlotObjects[slot], SlotTriangles[slot] );
                        }
                    }
                }
            }
            else
            {
                for ( unsigned int slot = entry.Child; slot < entry.Child + entry.Count; slot++ )
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
            const Node &node = Nodes[entry.Child];
            alignas( 16 ) float tNear[4];
            const int mask = useSIMD ? testChildBoxesSIMD( node.Bounds, origin4, invDir4, nearRow, farRow, hit.Distance, tNear )
                                     : testChildBoxes( node.Bounds, origin, invDir, nearRow, farRow, hit.Distance, tNear );
            if ( mask != 0 )
            {
                // Sort the children the ray enters, the farthest first. The nearest one is visited next,
                // the others wait on the stack.
                Entry children[4];
                int numberOfChildren = 0;
                for ( int i = 0; i < 4; i++ )
                {
                    if ( mask & ( 1 << i ) )
                    {
                        int k = numberOfChildren++;
                        while ( k > 0 && children[k - 1].Distance < tNear[i] )
                        {
                            children[k] = children[k - 1];
                            k--;
                        }
                        children[k].Child    = node.Child[i];
                        children[k].Count    = node.Count[i];
                        children[k].Distance = tNear[i];
                    }
                }
                for ( int i = 0; i < numberOfChildren - 1; i++ )
                {
                    stack[stackSize++] = children[i];
                }
                entry = children[numberOfChildren - 1];
                continue;
            }
        }

        // The next node or leaf that can still hold a closer hit, or an equally close one, which can win the tie
        bool found = false;
        while ( stackSize > 0 )
        {
            entry = stack[--stackSize];
            if ( entry.Distance <= hit.Distance )
            {
                found = true;
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
