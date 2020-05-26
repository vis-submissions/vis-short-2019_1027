#include <algorithm>
#include <cassert>
#include <iostream>
#include <fstream>
#include <ostream>
#include <numeric>
#include <stack>

#include <visionaray/math/math.h>

#include "draw.h"
#include "graph.h"
#include "image.h"

using namespace visionaray;

void Graph::append(Graph const& other)
{
    size_t oldNumNodes = nodes.size();
    size_t oldNumEdges = edges.size();

    nodes.insert(nodes.end(), other.nodes.begin(), other.nodes.end());
    edges.insert(edges.end(), other.edges.begin(), other.edges.end());

    if (oldNumNodes == 0)
        return;

    for (size_t e = oldNumEdges; e < edges.size(); ++e)
    {
        edges[e].i1 += (unsigned)oldNumNodes;
        edges[e].i2 += (unsigned)oldNumNodes;
    }
}

void Graph::draw(image_rgb8& img, float nodeRadius)
{
    basic_aabb<float, 2> bbox;
    bbox.invalidate();

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        bbox.insert(nodes[i]);
    }

    vec2 img_size(img.width(), img.height());

    auto projectNodeToImg = [&](Node n)
    {
        vec2 pos = n;
        pos -= bbox.min;
        pos /= bbox.max - bbox.min;
        pos *= img_size - 2 * nodeRadius;
        pos += nodeRadius;
        return pos;
    };

    for (size_t i = 0; i < edges.size(); ++i)
    {
        vec2 pos1 = projectNodeToImg(nodes[edges[i].i1]);
        vec2 pos2 = projectNodeToImg(nodes[edges[i].i2]);

        draw_line(pos1.x, pos1.y, pos2.x, pos2.y, img);
    }

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        vec2 pos = projectNodeToImg(nodes[i]);

        fill_circle(pos.x, pos.y, nodeRadius, img, {128, 0, 0});
        draw_circle(pos.x, pos.y, nodeRadius, img);
    }
}

void Graph::saveAsTlp(const std::string& outFilename, std::ios::openmode om)
{

    std::ofstream out(outFilename,om);
    
    out << "(tlp \"2.3\"" << std::endl;
    out << "(nb_nodes " << nodes.size() << ")" << std::endl;
    
    //out << "(cluster 0" << std::endl;

    out << "(nodes " << 0 << ".." << nodes.size()-1;
    out << ")" << std::endl;

    for(int eIdx=0; eIdx < edges.size(); eIdx++) {
	out << "(edge " << eIdx << " " << edges[eIdx].i1 << " " << edges[eIdx].i2 << ")" << std::endl;
    }

    out << "(property 0 layout \"viewLayout\"" << std::endl;
    out << "(default \"(886,346,302)\" \"()\")" << std::endl;
    for(int nIdx=0; nIdx < nodes.size(); nIdx++) {
    	out << "(node " << nIdx << " \"(" << nodes[nIdx].x << "," << nodes[nIdx].y << ",0" << ")\")" << std::endl; 
    }
    out << ")" << std::endl;

    //out << ")" << std::endl; //end cluster
    out << ")" << std::endl; //eof

}

void Graph::printStatistics(bool extended)
{
    std::cout << "--- Graph Statistics ----------------------------------------\n";
    std::cout << "# Nodes: .............................. " << nodes.size() << '\n';
    std::cout << "# Edges: .............................. " << edges.size() << '\n';

    if (!extended)
        return;

    bool directed = false;

    std::vector<int> outgoing(nodes.size());
    std::vector<int> incoming(nodes.size());
    std::fill(outgoing.begin(), outgoing.end(), 0);
    std::fill(incoming.begin(), incoming.end(), 0);

    float area = 0.0;
    basic_aabb<float, 2> bbox;
    bbox.invalidate();

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        bbox.insert(nodes[i]);
    }

    float minEdgeLen =  FLT_MAX;
    float maxEdgeLen = -FLT_MAX;
    float accEdgeLen = 0.f;

    for (size_t i = 0; i < edges.size(); ++i)
    {
        Edge e = edges[i];
        outgoing[e.i1]++;
        incoming[e.i2]++;

        if (!directed)
        {
            outgoing[e.i2]++;
            incoming[e.i1]++;
        }

        float len = length(nodes[e.i1]-nodes[e.i2]);
        minEdgeLen = std::min(minEdgeLen, len);
        maxEdgeLen = std::max(maxEdgeLen, len);
        accEdgeLen += len;
    }

    std::sort(outgoing.begin(), outgoing.end());
    std::sort(incoming.begin(), incoming.end());

    int numOutgoing = std::accumulate(outgoing.begin(), outgoing.end(), 0);
    int numIncoming = std::accumulate(incoming.begin(), incoming.end(), 0);

    std::vector<unsigned> countPerComponent;
    unsigned cc = countConnectedComponents(countPerComponent);
    assert(cc = countPerComponent.size());
    std::sort(countPerComponent.begin(), countPerComponent.end());
    unsigned accCount = std::accumulate(countPerComponent.begin(), countPerComponent.end(), 0);

    std::cout << "Avg. outgoing edges per node: ......... " << numOutgoing/double(nodes.size()) << '\n';
    std::cout << "Max. outgoing edges per node: ......... " << outgoing.back() << '\n';
    std::cout << "Min. outgoing edges per node: ......... " << outgoing.front() << '\n';

    std::cout << "Avg. incoming edges per node: ......... " << numIncoming/double(nodes.size()) << '\n';
    std::cout << "Max. incoming edges per node: ......... " << incoming.back() << '\n';
    std::cout << "Min. incoming edges per node: ......... " << incoming.front() << '\n';

    std::cout << "Frame size: ........................... " << bbox.size() << '\n';

    std::cout << "Avg. edge length: ..................... " << accEdgeLen/float(edges.size()) << '\n';
    std::cout << "Max. edge length: ..................... " << maxEdgeLen << '\n';
    std::cout << "Min. edge length: ..................... " << minEdgeLen << '\n';

    std::cout << "# connected components: ............... " << cc << '\n';
    std::cout << "Avg. nodes per connected component: ... " << accCount/double(cc) << '\n';
    std::cout << "Max. nodes per connected component: ... " << countPerComponent.back() << '\n';
    std::cout << "Min. nodes per connected component: ... " << countPerComponent.front() << '\n';
}

void Graph::buildAdjacentEdges()
{
    adjacentEdges.resize(nodes.size());
    for (auto& ae : adjacentEdges)
        ae.resize(0);

    for (size_t i = 0; i < edges.size(); ++i)
    {
        Edge e = edges[i];
        adjacentEdges[e.i1].push_back(e);
        adjacentEdges[e.i2].push_back(e);
    }
}

unsigned Graph::countConnectedComponents()
{
    std::vector<unsigned> ignore;
    unsigned count = countConnectedComponents(ignore);
    assert(count == ignore.size());
    return count;
}

unsigned Graph::countConnectedComponents(std::vector<unsigned>& countPerComponent)
{
    if (adjacentEdges.empty())
        buildAdjacentEdges();

    std::vector<bool> seen(nodes.size(), false);

    unsigned count = 0;

    for (size_t i = 0; i < seen.size(); ++i)
    {
        if (seen[i])
            continue;

        count++;

        unsigned vertexCount = 0;

        std::stack<size_t> st;

        st.push(i);

        while (!st.empty())
        {
            size_t s = st.top();
            st.pop();

            if (!seen[s])
            {
                seen[s] = true;
                vertexCount++;
            }

            for (Edge e : adjacentEdges[s])
            {
                unsigned other = e.i1;
                if (e.i1 == s)
                    other = e.i2;

                if (!seen[other])
                    st.push(other);
            }
        }

        countPerComponent.push_back(vertexCount);
    }

    return count;
}
