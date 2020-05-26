#pragma once

#include <memory>
#include <mutex>

#include "graph.h"
#include "image.h"

enum LayouterMode
{
        Naive,RTX,LBVH,
};

struct Layouter
{
    Layouter(Graph& g,
             image_rgb8& img,
             std::mutex& mtx,
             LayouterMode mode=LayouterMode::RTX,
             unsigned rebuildRTXAccelAfter=1);
   ~Layouter();

    void iterate();

    struct Impl;
    std::unique_ptr<Impl> impl;
};
