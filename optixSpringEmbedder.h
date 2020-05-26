#pragma once

#include <visionaray/math/math.h>

struct GraphGeom
{
    OptixTraversableHandle world;
    visionaray::vec2* nodes;
    visionaray::vec2* disp;
    unsigned numNodes;
    float k;
};
