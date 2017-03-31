#pragma once

// solution code removed

#include "rtutil.hpp"

#include <memory>
#include <vector>
#include "RTTriangle.hpp"

struct Node
{
    FW::AABB box; // Axis-aligned bounding box
    int startPrim, endPrim; // Indices in the global list
    std::unique_ptr<Node> leftChild;
    std::unique_ptr<Node> rightChild;

    FW::RaycastResult intersect_woop(const FW::Vec3f& orig, const FW::Vec3f& dir, std::vector<FW::RTTriangle>& triangles) const {

        FW::RaycastResult castresult;

        FW::Vec3f t_start;
        FW::Vec3f t_end;

        FW::Vec3f t_min = (box.min - orig) / dir;
        FW::Vec3f t_max = (box.max - orig) / dir;

        for (int i = 0; i < 3; i++) {
            t_start[i] = FW::min(t_min[i], t_max[i]);
            t_end[i] = FW::max(t_min[i], t_max[i]);
        }

        // Does ray hit a box?
        if (max(t_start) <= min(t_end)) {

            //If node has children
            if (leftChild != nullptr && rightChild != nullptr) {
                FW::RaycastResult castresult_left = leftChild->intersect_woop(orig, dir, triangles);
                FW::RaycastResult castresult_right = rightChild->intersect_woop(orig, dir, triangles);

                // Raycast hit of left node was closer than previous hit
                if (castresult_left.t <= castresult_right.t && castresult_left.t < castresult.t) {
                    castresult = castresult_left;
                }
                // Raycast hit of right node was closer than previous hit
                else if (castresult_right.t < castresult.t) {
                    castresult = castresult_right;
                }
            }

            // If node is leaf
            else {
                // From original code:
                // Loop through triangles in the node

                float tmin = 1.0f, umin = 0.0f, vmin = 0.0f;
                int imin = -1;

                for (int i = startPrim; i <= endPrim; i++) {

                    float t, u, v;
                    if (triangles[i].intersect_woop(orig, dir, t, u, v))
                    {
                        if (t > 0.0f && t < tmin)
                        {
                            imin = i;
                            tmin = t;
                            umin = u;
                            vmin = v;
                        }
                    }
                }
                if (imin != -1) {
                    FW::RaycastResult castresult_triangle = FW::RaycastResult(&(triangles)[imin], tmin, umin, vmin, orig + tmin*dir, orig, dir);
                    if (castresult_triangle.t < castresult.t) {
                        castresult = castresult_triangle;
                    }
                }
            }
        }

        return castresult;
    }
};

