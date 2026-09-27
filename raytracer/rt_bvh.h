#ifndef RT_BVH_H
#define RT_BVH_H

#include <vector>
#include "math/rt_vector.h"
#include "math/rt_vector_pack.h"
#include "raytracer/rt_ray.h"

class RTObject;
class RTScene;

// Bounding volume hierarchy over the world space triangles of all objects of a scene: a tree of axis-aligned
// boxes with up to 4 triangles in each leaf. It's built as a binary tree with the surface area heuristic, then
// collapsed so each node has up to 4 children, whose boxes the SIMD path tests at once (BVH4).
// A ray only tests the triangles in the boxes it passes through, instead of every triangle.
class RTBVH
{
public:
    // Call RTObject::updateWorldSpace() on the objects first; RTRenderer::render does both
    void build( RTScene *scene );

    // The closest hit before distance, the same one as testing every object with RTObject::intersect
    RTObject* intersect( const RTRay &ray, float &distance, unsigned int &triangleIndex, float &u, float &v, bool useSIMD ) const;

    // True if the ray hits any triangle before maxDistance. It stops at the first one it finds (any hit),
    // which is all a shadow ray needs.
    bool occluded( const RTRay &ray, float maxDistance, bool useSIMD ) const;

private:
    // The boxes of the 4 children side by side, one SSE register per row. Missing children have
    // inverted boxes (Min = infinity, Max = -infinity), which no ray hits.
    struct alignas( 16 ) Node
    {
        float        Bounds[6][4];  // MinX, MinY, MinZ, MaxX, MaxY, MaxZ of each child
        unsigned int Child[4];      // inner child: its node index; leaf: its first triangle slot
        unsigned int Count[4];      // leaf: 1 to 4 triangles; inner child: 0
    };

    // A node of the binary tree that the 4-wide one is collapsed from
    struct BuildNode
    {
        float        Min[3];
        unsigned int LeftOrFirst;   // inner node: the left child, the right one follows it; leaf: the first triangle slot
        float        Max[3];
        unsigned int Count;         // leaf: 1 to 4 triangles; inner node: 0
    };

    // A triangle while the tree is built
    struct BuildTriangle
    {
        float        Min[3];
        float        Max[3];
        float        Centroid[3];
        unsigned int Object;        // index in the scene
        unsigned int Index;         // index in the object
    };

    void subdivide( std::vector<BuildNode> &buildNodes, std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count, int depth );
    void makeLeaf( std::vector<BuildNode> &buildNodes, const std::vector<BuildTriangle> &triangles, unsigned int nodeIndex, unsigned int first, unsigned int count );

    // Makes the 4-wide node of a binary node and its subtree; returns its index in Nodes
    unsigned int collapse( const std::vector<BuildNode> &buildNodes, unsigned int buildIndex );

    std::vector<Node> Nodes;

    // Each leaf has 4 triangle slots from its first one on; the unused ones hold triangles that are never hit
    std::vector<RTVectorPack> Packs;            // SIMD path: v0, v0v1 and v0v2 of each leaf's 4 slots
    std::vector<RTVector>     Triangles;        // scalar path: v0, v0v1 and v0v2 of each slot
    std::vector<unsigned int> SlotObjects;      // object index of each slot
    std::vector<unsigned int> SlotTriangles;    // triangle index of each slot, in its object
    std::vector<RTObject*>    Objects;          // in the scene's order
};

#endif // RT_BVH_H
