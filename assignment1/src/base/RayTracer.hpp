#pragma once


#include "RTTriangle.hpp"
#include "RaycastResult.hpp"
#include "rtlib.hpp"
#include "Bvh.hpp"
#include "BvhNode.hpp"

#include "base/String.hpp"

#include <vector>
#include <atomic>

namespace FW
{


// Given a vector n, forms an orthogonal matrix with n as the last column, i.e.,
// a coordinate system aligned such that n is its local z axis.
// You'll have to fill in the implementation for this.
Mat3f formBasis(const Vec3f& n);

Vec2f getTexelCoords(Vec2f uv, const Vec2i size);


// Main class for tracing rays using BVHs.
class RayTracer {
public:
	RayTracer(void);
	~RayTracer(void);

	void				constructHierarchy(std::vector<RTTriangle>& triangles, SplitMode splitMode);

	void				saveHierarchy(const char* filename, const std::vector<RTTriangle>& triangles);
	void				loadHierarchy(const char* filename, std::vector<RTTriangle>& triangles);

    void                writeNodeToFile(std::ofstream& outfile, Node& node);
    void                readNodeFromFile(std::ifstream& infile, Node& node);
    void                makeTupleList(std::vector<RTTriangle> triangles);

    //void                sortTriangles();

    void				spatialMedian(std::vector<RTTriangle>& triangles, Node& node, int start, int end);
	void				objectMedian(std::vector<RTTriangle>& triangles, Node& node, int start, int end);
    void				sah(std::vector<RTTriangle>& triangles, Node& node, int start, int end);
    void				noneSplitMode(std::vector<RTTriangle>& triangles, Node& node, int start, int end);

    int                 binarySearch(std::vector<RTTriangle>& triangles, int start, int end, float goal, int axis);
        
    float               rightBoundingBoxSurfaceArea(std::vector<RTTriangle>& triangles, int start, int end);
    float               leftBoundingBoxSurfaceArea(std::vector<RTTriangle>& triangles, int start, int end);
    
    std::vector<float>  calculateLeftSideAABBSurfaceAreas(std::vector<RTTriangle>& triangles, int start, int end);
    std::vector<float>  calculateRightSideAABBSurfaceAreas(std::vector<RTTriangle>& triangles, int start, int end);

	RaycastResult		raycast(const Vec3f& orig, const Vec3f& dir) const;

	// This function computes an MD5 checksum of the input scene data,
	// WITH the assumption that all vertices are allocated in one big chunk.
	static FW::String	computeMD5(const std::vector<Vec3f>& vertices);

	std::vector<RTTriangle>* m_triangles;
    std::vector<std::tuple<RTTriangle, int>> tupleList;

    std::vector<AABB> leftSideBoxes;
    std::vector<AABB> rightSideBoxes;

	void resetRayCounter() { m_rayCount = 0; }
	int getRayCount() { return m_rayCount; }

	std::unique_ptr<Node> BvhNode;
    int minimumTriangles;

private:
	mutable std::atomic<int> m_rayCount; 
};


} // namespace FW