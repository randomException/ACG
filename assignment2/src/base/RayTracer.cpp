#define _CRT_SECURE_NO_WARNINGS

#include "base/Defs.hpp"
#include "base/Math.hpp"
#include "RayTracer.hpp"
#include <stdio.h>
#include "rtIntersect.inl"
#include <fstream>

#include "rtlib.hpp"

#include "BvhNode.hpp"
#include <algorithm>
#include <limits>
#include <string>
#include <tuple>

#include <sys/stat.h>
#include "filesaves.hpp"
#include <cmath>



// Helper function for hashing scene data for caching BVHs
extern "C" void MD5Buffer(void* buffer, size_t bufLen, unsigned int* pDigest);


namespace FW
{


    Vec2f getTexelCoords(Vec2f uv, const Vec2i size)
    {

        // YOUR CODE HERE (R3):
        // Get texel indices of texel nearest to the uv vector. Used in texturing.
        // UV coordinates range from negative to positive infinity. First map them
        // to a range between 0 and 1 in order to support tiling textures, then
        // scale the coordinates by image resolution and find the nearest pixel.
        Vec2f mapping;
        mapping.x = fmod(uv.x, 1);
        if (mapping.x < 0) {
            mapping.x = 1 + mapping.x;
        }

        mapping.y = fmod(uv.y, 1);
        if (mapping.y < 0) {
            mapping.y = 1 + mapping.y;
        }

        Vec2f result;
        result.x = (int)(mapping.x * size.x);
        result.y = (int)(mapping.y * size.y);
        return result;
    }

    Mat3f formBasis(const Vec3f& n) {
        // YOUR CODE HERE (R4):
        Mat3f R = Mat3f();

        Vec3f Q = n;
        float min = abs(Q[0]);
        float min_index = 0;
        for (int i = 1; i < 3; i++) {
            if (abs(Q[i]) < min) {
                min = abs(Q[i]);
                min_index = i;
            }
        }
        Q[min_index] = 1;

        Vec3f T = normalize(cross(Q, n));
        Vec3f B = normalize(cross(n, T));

        R.setCol(0, T);
        R.setCol(1, B);
        R.setCol(2, n);

        return R;
    }


    String RayTracer::computeMD5(const std::vector<Vec3f>& vertices)
    {
        unsigned char digest[16];
        MD5Buffer((void*)&vertices[0], sizeof(Vec3f)*vertices.size(), (unsigned int*)digest);

        // turn into string
        char ad[33];
        for (int i = 0; i < 16; ++i)
            ::sprintf(ad + i * 2, "%02x", digest[i]);
        ad[32] = 0;

        return FW::String(ad);
    }


    // --------------------------------------------------------------------------


    RayTracer::RayTracer()
    {
    }

    RayTracer::~RayTracer()
    {
    }


    void RayTracer::loadHierarchy(const char* filename, std::vector<RTTriangle>& triangles)
    {
        // YOUR CODE HERE (R2):
        m_triangles = &triangles;

        std::ifstream infile(filename, std::ios::binary);
        size_t triangleCount;
        read(infile, triangleCount);
        //std::cout << triangleCount << std::endl;

        std::vector<RTTriangle>* sortedTriangles = new std::vector<RTTriangle>();
        for (auto i = 0; i < triangleCount; i++) {
            size_t id;
            read(infile, id);
            RTTriangle tri = (*m_triangles)[id];
            sortedTriangles->push_back(tri);
        }
        m_triangles = sortedTriangles;

        BvhNode.reset(new Node());
        readNodeFromFile(infile, *BvhNode);

    }


    void RayTracer::saveHierarchy(const char* filename, const std::vector<RTTriangle>& triangles) {
        // YOUR CODE HERE (R2)

        std::ofstream outfile(filename, std::ios::binary);
        write(outfile, tupleList.size());
        for (auto i = 0; i < tupleList.size(); i++) {
            size_t id = std::get<1>(tupleList[i]);
            write(outfile, id);
        }

        writeNodeToFile(outfile, *BvhNode);

    }

    void RayTracer::readNodeFromFile(std::ifstream& infile, Node& node) {

        read(infile, node.box.min);
        read(infile, node.box.max);
        read(infile, node.startPrim);
        read(infile, node.endPrim);

        node.leftChild = nullptr;
        node.rightChild = nullptr;

        int hasChild;
        read(infile, hasChild);
        if (hasChild) {
            node.leftChild.reset(new Node());
            readNodeFromFile(infile, *node.leftChild);
            node.rightChild.reset(new Node());
            readNodeFromFile(infile, *node.rightChild);
        }
    }

    void RayTracer::writeNodeToFile(std::ofstream& outfile, Node& node) {

        write(outfile, node.box.min);
        write(outfile, node.box.max);
        write(outfile, node.startPrim);
        write(outfile, node.endPrim);
        if (node.leftChild != nullptr && node.rightChild != nullptr) {
            int hasChild = 1;
            write(outfile, hasChild);
            writeNodeToFile(outfile, *node.leftChild);
            writeNodeToFile(outfile, *node.rightChild);
        }
        else {
            int hasChild = 0;
            write(outfile, hasChild);
        }
    }

    void RayTracer::constructHierarchy(std::vector<RTTriangle>& triangles, SplitMode splitMode) {
        // YOUR CODE HERE (R1):
        m_triangles = &triangles;
        makeTupleList(triangles);

        // Object Median
        BvhNode.reset(new Node());
        BvhNode->startPrim = 0;
        BvhNode->endPrim = triangles.size() - 1;

        minimumTriangles = 11;
        switch (splitMode)
        {
        case FW::SplitMode_SpatialMedian:
            std::cout << "SplitMode_SpatialMedian" << std::endl;
            spatialMedian(*m_triangles, *BvhNode, 0, triangles.size() - 1);
            break;
        case FW::SplitMode_ObjectMedian:
            std::cout << "SplitMode_ObjectMedian" << std::endl;
            objectMedian(*m_triangles, *BvhNode, 0, triangles.size() - 1);
            break;
        case FW::SplitMode_Sah:
            std::cout << "SplitMode_Sah" << std::endl;
            sah(*m_triangles, *BvhNode, 0, triangles.size() - 1);
            break;
        case FW::SplitMode_None:
            std::cout << "SplitMode_None" << std::endl;
            noneSplitMode(*m_triangles, *BvhNode, 0, triangles.size() - 1);
            break;
        case FW::SplitMode_Linear:
            std::cout << "SplitMode_Linear" << std::endl;
            //break
        default:
            objectMedian(*m_triangles, *BvhNode, 0, triangles.size() - 1);
            break;
        }
    }

    void RayTracer::makeTupleList(std::vector<RTTriangle> triangles) {

        for (auto i = 0; i < triangles.size(); i++) {
            std::tuple<RTTriangle, int> tuple = std::make_tuple(triangles[i], i);
            tupleList.push_back(tuple);
        }
    }

    struct triangleCompare
    {
        int axis;
        triangleCompare(int _axis) : axis(_axis) {}

        inline bool operator() (const RTTriangle& triangle1, const RTTriangle& triangle2)
        {
            return (triangle1.centroid()[axis] < triangle2.centroid()[axis]);
        }
    };

    struct tupleTriangleCompare
    {
        int axis;
        tupleTriangleCompare(int _axis) : axis(_axis) {}

        inline bool operator() (std::tuple<RTTriangle, int>& tuple1, std::tuple<RTTriangle, int>& tuple2)
        {
            return (std::get<0>(tuple1).centroid()[axis] < std::get<0>(tuple2).centroid()[axis]);
        }
    };

    void RayTracer::noneSplitMode(std::vector<RTTriangle>& triangles, Node& node, int start, int end) {
        m_triangles = &triangles;

        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        for (int i = start + 1; i <= end; i++) {
            Vec3f vec_max = triangles[i].max();
            Vec3f vec_min = triangles[i].min();
            for (int j = 0; j < 3; j++) {
                if (vec_min[j] < min_vec[j]) {
                    min_vec[j] = vec_min[j];
                }
                if (vec_max[j] > max_vec[j]) {
                    max_vec[j] = vec_max[j];
                }
            }
        }
        node.box.min = min_vec;
        node.box.max = max_vec;

        node.leftChild = nullptr;
        node.rightChild = nullptr;
    }

    void RayTracer::objectMedian(std::vector<RTTriangle>& triangles, Node& node, int start, int end) {
        m_triangles = &triangles;

        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        for (int i = start + 1; i <= end; i++) {
            Vec3f vec_max = triangles[i].max();
            Vec3f vec_min = triangles[i].min();
            for (int j = 0; j < 3; j++) {
                if (vec_min[j] < min_vec[j]) {
                    min_vec[j] = vec_min[j];
                }
                if (vec_max[j] > max_vec[j]) {
                    max_vec[j] = vec_max[j];
                }
            }
        }

        node.box.min = min_vec;
        node.box.max = max_vec;

        int longestAxis;
        float x_axis = node.box.max[0] - node.box.min[0];
        float y_axis = node.box.max[1] - node.box.min[1];
        float z_axis = node.box.max[2] - node.box.min[2];

        if (x_axis >= y_axis && x_axis >= z_axis) {
            longestAxis = 0;
        }
        else if (y_axis >= x_axis && y_axis >= z_axis) {
            longestAxis = 1;
        }
        else {
            longestAxis = 2;
        }

        triangleCompare comparison = triangleCompare(longestAxis);
        std::sort(m_triangles->begin() + start, m_triangles->begin() + end + 1, comparison);

        // Sort for tuples (so we can save triangles in file)
        tupleTriangleCompare tupleComparison = tupleTriangleCompare(longestAxis);
        std::sort(tupleList.begin() + start, tupleList.begin() + end + 1, tupleComparison);

        node.leftChild = nullptr;
        node.rightChild = nullptr;

        int size = end - start + 1;

        if (size >= minimumTriangles) {
            int splitID = size / 2;

            node.leftChild.reset(new Node());
            node.leftChild->startPrim = start;
            node.leftChild->endPrim = start + splitID - 1;
            objectMedian(triangles, *node.leftChild, node.leftChild->startPrim, node.leftChild->endPrim);

            node.rightChild.reset(new Node());
            node.rightChild->startPrim = start + splitID;
            node.rightChild->endPrim = end;
            objectMedian(triangles, *node.rightChild, node.rightChild->startPrim, node.rightChild->endPrim);
        }
    }

    void RayTracer::sah(std::vector<RTTriangle>& triangles, Node& node, int start, int end) {
        m_triangles = &triangles;

        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        for (int i = start + 1; i <= end; i++) {
            Vec3f vec_max = triangles[i].max();
            Vec3f vec_min = triangles[i].min();
            for (int j = 0; j < 3; j++) {
                if (vec_min[j] < min_vec[j]) {
                    min_vec[j] = vec_min[j];
                }
                if (vec_max[j] > max_vec[j]) {
                    max_vec[j] = vec_max[j];
                }
            }
        }

        node.box.min = min_vec;
        node.box.max = max_vec;

        node.leftChild = nullptr;
        node.rightChild = nullptr;

        int size = end - start + 1;
        if (size < minimumTriangles) {
            return;
        }

        int bestAxis = -1;
        int overallBestSplitID;
        int f_best;

        // Calculate best split ID and best split axis
        for (int a = 0; a < 3; a++) {
            triangleCompare comparison = triangleCompare(a);
            std::sort(m_triangles->begin() + start, m_triangles->begin() + end + 1, comparison);

            // Sort for tuples (so we can save triangles in file)
            tupleTriangleCompare tupleComparison = tupleTriangleCompare(a);
            std::sort(tupleList.begin() + start, tupleList.begin() + end + 1, tupleComparison);

            std::vector<float> leftSideAABBSurfaceAreas = calculateLeftSideAABBSurfaceAreas(triangles, start, end);
            std::vector<float> rightSideAABBSurfaceAreas = calculateRightSideAABBSurfaceAreas(triangles, start, end);

            float f_min = std::numeric_limits<double>::infinity();
            int bestSplitID;

            for (int i = start; i < end; i++) {
                float LSA = leftSideAABBSurfaceAreas[i - start];
                float RSA = rightSideAABBSurfaceAreas[end - i - 1];

                int L = i - start + 1;
                int R = end - i;

                float f = LSA * L + RSA * R;

                if (f < f_min) {
                    f_min = f;
                    bestSplitID = i;
                }
            }

            if (bestAxis == -1) {
                bestAxis = 0;
                overallBestSplitID = bestSplitID;
                f_best = f_min;
            }
            else if (f_min < f_best) {
                bestAxis = a;
                overallBestSplitID = bestSplitID;
                f_best = f_min;
            }
        } // End searching best split ID and best split axis

          // Sort triangles again if z is not the best axis
        if (bestAxis != 2) {
            triangleCompare comparison = triangleCompare(bestAxis);
            std::sort(m_triangles->begin() + start, m_triangles->begin() + end + 1, comparison);

            // Sort for tuples (so we can save triangles in file)
            tupleTriangleCompare tupleComparison = tupleTriangleCompare(bestAxis);
            std::sort(tupleList.begin() + start, tupleList.begin() + end + 1, tupleComparison);
        }

        node.leftChild = nullptr;
        node.rightChild = nullptr;

        /*int*/ size = end - start + 1;
        //if (size >= minimumTriangles) {
        // Setup childrens
        node.leftChild.reset(new Node());
        node.leftChild->startPrim = start;
        node.leftChild->endPrim = overallBestSplitID;
        sah(triangles, *node.leftChild, node.leftChild->startPrim, node.leftChild->endPrim);

        node.rightChild.reset(new Node());
        node.rightChild->startPrim = overallBestSplitID + 1;
        node.rightChild->endPrim = end;
        sah(triangles, *node.rightChild, node.rightChild->startPrim, node.rightChild->endPrim);
        //}
    }

    std::vector<float> RayTracer::calculateLeftSideAABBSurfaceAreas(std::vector<RTTriangle>& triangles, int start, int end) {
        std::vector<float> surfaceAreaList = std::vector<float>();
        leftSideBoxes = std::vector<AABB>();
        for (int i = start; i < end; i++) {
            surfaceAreaList.push_back(leftBoundingBoxSurfaceArea(triangles, i, end));
        }
        return surfaceAreaList;
    }

    std::vector<float> RayTracer::calculateRightSideAABBSurfaceAreas(std::vector<RTTriangle>& triangles, int start, int end) {
        std::vector<float> surfaceAreaList = std::vector<float>();
        rightSideBoxes = std::vector<AABB>();
        for (int i = end; i > start; i--) {
            surfaceAreaList.push_back(rightBoundingBoxSurfaceArea(triangles, i, end));
        }
        return surfaceAreaList;
    }

    float RayTracer::rightBoundingBoxSurfaceArea(std::vector<RTTriangle>& triangles, int start, int end) {
        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        if (rightSideBoxes.size() != 0) {
            for (int j = 0; j < 3; j++) {
                if (rightSideBoxes[rightSideBoxes.size() - 1].min[j] < min_vec[j]) {
                    min_vec[j] = rightSideBoxes[rightSideBoxes.size() - 1].min[j];
                }
                if (rightSideBoxes[rightSideBoxes.size() - 1].max[j] > max_vec[j]) {
                    max_vec[j] = rightSideBoxes[rightSideBoxes.size() - 1].max[j];
                }
            }
        }

        AABB tempBox = AABB();
        tempBox.max = max_vec;
        tempBox.min = min_vec;
        rightSideBoxes.push_back(tempBox);

        // area = 2xy + 2xz + 2yz
        float area = 2 * (max_vec[0] - min_vec[0]) * (max_vec[1] - min_vec[1]) +
            2 * (max_vec[0] - min_vec[0]) * (max_vec[2] - min_vec[2]) +
            2 * (max_vec[1] - min_vec[1]) * (max_vec[2] - min_vec[2]);

        return area;
    }

    float RayTracer::leftBoundingBoxSurfaceArea(std::vector<RTTriangle>& triangles, int start, int end) {
        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        if (leftSideBoxes.size() != 0) {
            for (int j = 0; j < 3; j++) {
                if (leftSideBoxes[leftSideBoxes.size() - 1].min[j] < min_vec[j]) {
                    min_vec[j] = leftSideBoxes[leftSideBoxes.size() - 1].min[j];
                }
                if (leftSideBoxes[leftSideBoxes.size() - 1].max[j] > max_vec[j]) {
                    max_vec[j] = leftSideBoxes[leftSideBoxes.size() - 1].max[j];
                }
            }
        }

        AABB tempBox = AABB();
        tempBox.max = max_vec;
        tempBox.min = min_vec;
        leftSideBoxes.push_back(tempBox);

        // area = 2xy + 2xz + 2yz
        float area = 2 * (max_vec[0] - min_vec[0]) * (max_vec[1] - min_vec[1]) +
            2 * (max_vec[0] - min_vec[0]) * (max_vec[2] - min_vec[2]) +
            2 * (max_vec[1] - min_vec[1]) * (max_vec[2] - min_vec[2]);

        return area;
    }

    void RayTracer::spatialMedian(std::vector<RTTriangle>& triangles, Node& node, int start, int end) {
        m_triangles = &triangles;

        Vec3f min_vec = triangles[start].min();
        Vec3f max_vec = triangles[start].max();

        for (int i = start + 1; i <= end; i++) {
            Vec3f vec_max = triangles[i].max();
            Vec3f vec_min = triangles[i].min();
            for (int j = 0; j < 3; j++) {
                if (vec_min[j] < min_vec[j]) {
                    min_vec[j] = vec_min[j];
                }
                if (vec_max[j] > max_vec[j]) {
                    max_vec[j] = vec_max[j];
                }
            }
        }

        node.box.min = min_vec;
        node.box.max = max_vec;

        int longestAxis;
        float x_axis = node.box.max[0] - node.box.min[0];
        float y_axis = node.box.max[1] - node.box.min[1];
        float z_axis = node.box.max[2] - node.box.min[2];

        if (x_axis >= y_axis && x_axis >= z_axis) {
            longestAxis = 0;
        }
        else if (y_axis >= x_axis && y_axis >= z_axis) {
            longestAxis = 1;
        }
        else {
            longestAxis = 2;
        }

        triangleCompare comparison = triangleCompare(longestAxis);
        std::sort(m_triangles->begin() + start, m_triangles->begin() + end + 1, comparison);

        // Sort for tuples (so we can save triangles in file)
        tupleTriangleCompare tupleComparison = tupleTriangleCompare(longestAxis);
        std::sort(tupleList.begin() + start, tupleList.begin() + end + 1, tupleComparison);

        float splitLocation = (triangles[start].centroid()[longestAxis] + triangles[end].centroid()[longestAxis]) / 2;

        node.leftChild = nullptr;
        node.rightChild = nullptr;

        int size = end - start + 1;

        if (size >= minimumTriangles) {
            int splitID = binarySearch(triangles, start, end, splitLocation, longestAxis);

            node.leftChild.reset(new Node());
            node.leftChild->startPrim = start;
            node.leftChild->endPrim = splitID;
            spatialMedian(triangles, *node.leftChild, node.leftChild->startPrim, node.leftChild->endPrim);

            node.rightChild.reset(new Node());
            node.rightChild->startPrim = splitID + 1;
            node.rightChild->endPrim = end;
            spatialMedian(triangles, *node.rightChild, node.rightChild->startPrim, node.rightChild->endPrim);
        }
    }

    int RayTracer::binarySearch(std::vector<RTTriangle>& triangles, int start, int end, float splitLocation, int axis) {
        int l = start;
        int r = end - 1;
        int m;

        while (l <= r) {
            m = (r + l) / 2;
            if (triangles[m].centroid()[axis] < splitLocation) {
                l = m + 1;
            }
            else if (triangles[m].centroid()[axis] > splitLocation) {
                r = m - 1;
            }
            else {
                return m;
            }
        }

        if (triangles[m].centroid()[axis] > splitLocation) {
            m -= 1;
        }

        return m;
    }


    RaycastResult RayTracer::raycast(const Vec3f& orig, const Vec3f& dir) const {
        ++m_rayCount;

        // YOUR CODE HERE (R1):
        // This is where you hierarchically traverse the tree you built!
        // You can use the existing code for the leaf nodes.

        return BvhNode->intersect_woop(orig, dir, *m_triangles);

    }


} // namespace FW