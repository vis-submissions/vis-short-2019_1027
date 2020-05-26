#pragma once

#include <memory>

#include <visionaray/math/forward.h>

#include "graph.h"
#include "image.h"

class BVH
{
public:

  BVH();
 ~BVH();

  /*! Pass nodes in device global memory */
  void build(const Node* nodes, unsigned numNodes);

  /*! Pass nodes and disp in device global memory */
  void computeRepForces(const Node* nodes, unsigned numNodes, visionaray::vec2* disp, float k);

  void draw(const Node* vertices, size_t numVerts, image_rgb8& img);

private:

  struct Impl;
  std::unique_ptr<Impl> impl_;

};

