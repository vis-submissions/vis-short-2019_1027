#pragma once

#include <vector>

#include <fstream>

#include <visionaray/math/forward.h>
#include <visionaray/math/vector.h>

#include "image.h"

typedef visionaray::vec2 Node;
struct Edge
{

    unsigned i1;
    unsigned i2;


    Edge() : i1(0), i2(0) {

    }

    Edge(unsigned i1, unsigned i2) : i1(i1), i2(i2) {

    }
};

class Graph
{
public:
    std::vector<Node> nodes;
    std::vector<Edge> edges;

    std::vector<std::vector<Edge>> adjacentEdges;

    void append(Graph const& other);
    void draw(image_rgb8& img, float nodeRadius);


    void saveAsTlp(const std::string& outFileName, std::ios::openmode om=std::ios::out);

    void printStatistics(bool extended = false);

private:
    void buildAdjacentEdges();
    unsigned countConnectedComponents();
    unsigned countConnectedComponents(std::vector<unsigned>& countPerComponent);

};
