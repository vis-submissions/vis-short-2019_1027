#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <boost/filesystem.hpp>

#include <GL/glew.h>

#include <visionaray/gl/debug_callback.h>
#include <visionaray/gl/handle.h>
#include <visionaray/gl/program.h>
#include <visionaray/gl/shader.h>
#include <visionaray/math/io.h>
#include <visionaray/pinhole_camera.h>

#include <common/input/key_event.h>
#include <common/manip/pan_manipulator.h>
#include <common/manip/zoom_manipulator.h>
#include <common/timer.h>
#include <common/viewer_glut.h>
#include <Support/CmdLine.h>
#include <Support/CmdLineUtil.h>
#include <Support/StringSplit.h>

#define VERBOSE

#include "gd.h"
#include "graph.h"

using namespace support;
using cmdline_options = std::vector<std::shared_ptr<cl::OptionBase>>;

using namespace visionaray;

enum eInputSource {
    ARTIFICIAL,
    TREE,
    FILEINPUT
};

eInputSource inputSource = eInputSource::ARTIFICIAL;

//-------------------------------------------------------------------------------------------------
// Misc.helpers
//

std::istream& operator>>(std::istream& in, pinhole_camera& cam)
{
    vec3 eye;
    vec3 center;
    vec3 up;

    in >> eye >> std::ws >> center >> std::ws >> up >> std::ws;
    cam.look_at(eye, center, up);

    return in;
}

std::ostream& operator<<(std::ostream& out, pinhole_camera const& cam)
{
    out << cam.eye() << '\n';
    out << cam.center() << '\n';
    out << cam.up() << '\n';
    return out;
}


static std::vector<std::string> stringSplit(std::string s, char delim)
{
    std::vector<std::string> result;

    std::istringstream stream(s);

    for (std::string token; std::getline(stream, token, delim); )
    {
        result.push_back(token);
    }

    return result;
}

static std::string getExt(const std::string &fileName)
{
    int pos = fileName.rfind('.');
    if (pos == fileName.npos)
        return "";
    return fileName.substr(pos);
}

struct ArtificialOptions {
    int Clusters = 80;
    int NodesPerCluster = 50;
    int EdgesPerCluster = 100;
    bool Connected = true;
};

struct TreeOptions {
    int depth=1;
    int degree=2;
};

struct Cmd {
    std::vector<std::string> inputFiles;
    std::string sOutputFile = "";
    int maxIterations = -1;
    int loadRepetitions = 1;
    bool active=false;
    bool benchmarkMode=false;


    LayouterMode layouterMode=LayouterMode::RTX;

    unsigned rebuildRTXAccelAfter=1;

    ArtificialOptions artificialOptions;
    TreeOptions treeOptions;
    std::string initial_camera;
};

//-------------------------------------------------------------------------------------------------
// Graph loaders
//

void load_artificial(Graph& g, const ArtificialOptions &opt)
{

    Graph temp;
    for (int c = 0; c < opt.Clusters; ++c)
    {
        for (int n = 0; n < opt.NodesPerCluster; ++n)
        {
            temp.nodes.emplace_back();
        }

        std::vector<bool> isConnected(opt.NodesPerCluster * opt.NodesPerCluster);
        std::fill(isConnected.begin(), isConnected.end(), false);

        std::default_random_engine rng;
        std::uniform_int_distribution<int> dist(0, opt.NodesPerCluster - 1);
        for (int e = 0; e < opt.EdgesPerCluster; ++e)
        {
            int n1 = 0;
            int n2 = 0;

            for (;;)
            {
                n1 = dist(rng);
                n2 = dist(rng);

                if (n1 == n2)
                    continue;

                if (isConnected[n1 * opt.NodesPerCluster + n2])
                    continue;

                isConnected[n1 * opt.NodesPerCluster + n2] = true;
                isConnected[n2 * opt.NodesPerCluster + n1] = true;

                unsigned real_n1 = n1 + (int)temp.nodes.size() - opt.NodesPerCluster;
                unsigned real_n2 = n2 + (int)temp.nodes.size() - opt.NodesPerCluster;

                temp.edges.push_back({real_n1,real_n2});

                break;
            }
        }

        constexpr int NumClustersToConnectWith = 4;
        if (opt.Connected && c >= NumClustersToConnectWith)
        {
            for (int nc = 0; nc < NumClustersToConnectWith; ++nc)
            {
            // Connect the clusters with one edge
            int n1 = dist(rng);
            int n2 = dist(rng);

            std::uniform_int_distribution<int> connDist(0, NumClustersToConnectWith);
            unsigned real_n1 = n1 + (c-connDist(rng)-1) * opt.NodesPerCluster;
            unsigned real_n2 = n2 + c * opt.NodesPerCluster;

            temp.edges.push_back({real_n1,real_n2});
            }
        }
    }

    g.append(temp);
}


void recursive_add_children(Graph& g, unsigned prev, int depth, int degree) {
  if (depth > 0) {
    for (int i = 0; i < degree; ++i) {
      g.nodes.emplace_back();
      g.edges.push_back({prev, (unsigned) g.nodes.size()-1});
      recursive_add_children(g, g.nodes.size()-1, depth - 1, degree);
    }
  }
}

void load_complete_tree(Graph& g, int depth, int degree) {
  
  //Root Node
  g.nodes.emplace_back();
  recursive_add_children(g, g.nodes.size()-1, depth, degree);
}

void load_gephi_csv(Graph& g, std::string fileName)
{
    // Assumes "Source,Target,..." csv files
    std::ifstream in(fileName);

    std::string line;
    // ignore first;
    std::getline(in, line);

    std::map<std::string, unsigned> knownNodes;

    Graph temp;
    while (std::getline(in, line))
    {
        std::vector<std::string> columns = stringSplit(line, ',');

        auto sourceIt = knownNodes.find(columns[0]);
        unsigned sourceID;

        if (sourceIt == knownNodes.end())
        {
            sourceID = (unsigned)g.nodes.size();
            g.nodes.emplace_back();
            knownNodes.insert({columns[0],sourceID});
        }
        else
            sourceID = sourceIt->second;


        auto targetIt = knownNodes.find(columns[1]);
        unsigned targetID;

        if (targetIt == knownNodes.end())
        {
            targetID = (unsigned)g.nodes.size();
            g.nodes.emplace_back();
            knownNodes.insert({columns[1],targetID});
        }
        else
            targetID = targetIt->second;


        g.edges.push_back({sourceID,targetID});
    }
}


// from https://snap.stanford.edu/data/
void load_deezer(Graph& g, std::string fileName)
{
    std::ifstream in(fileName);

    std::string line;
    // ignore first;
    std::getline(in, line);

    Graph temp;
    while (std::getline(in, line))
    {
        unsigned i, j;
        sscanf(line.c_str(), "%i, %i", &i, &j);

        //if (i > 4000 || j > 4000)
        //    continue;

        temp.edges.push_back({i,j});

        if ((i + 1) > temp.nodes.size() || (j + 1) > temp.nodes.size())
        {
            temp.nodes.resize(std::max(i + 1, j + 1));
        }
    }

    g.append(temp);
}

struct renderer : visionaray::viewer_glut
{
    bool requestExit = false;
    bool benchmarkMode = false;

    struct GraphPipeline
    {
        gl::buffer  vertex_buffer;
        gl::buffer  index_buffer;
        gl::program prog;
        gl::shader  vert;
        gl::shader  frag;
        GLuint      view_loc;
        GLuint      proj_loc;
        GLuint      vertex_loc;
    };

    GraphPipeline graphPipeline;

    bool buildGraphPipeline()
    {
        // Setup shaders
        graphPipeline.vert.reset(glCreateShader(GL_VERTEX_SHADER));
        graphPipeline.vert.set_source(R"(
            attribute vec2 vertex;

            uniform mat4 view;
            uniform mat4 proj;


            void main(void)
            {
                gl_Position = proj * view * vec4(vertex, 0.0, 1.0);
            }
            )");
        graphPipeline.vert.compile();
        if (!graphPipeline.vert.check_compiled())
        {
            return false;
        }

        graphPipeline.frag.reset(glCreateShader(GL_FRAGMENT_SHADER));
        graphPipeline.frag.set_source(R"(
            void main(void)
            {
                gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            }
            )");
        graphPipeline.frag.compile();
        if (!graphPipeline.frag.check_compiled())
        {
            return false;
        }

        graphPipeline.prog.reset(glCreateProgram());
        graphPipeline.prog.attach_shader(graphPipeline.vert);
        graphPipeline.prog.attach_shader(graphPipeline.frag);

        graphPipeline.prog.link();
        if (!graphPipeline.prog.check_linked())
        {
            return false;
        }

        graphPipeline.vertex_loc = glGetAttribLocation(graphPipeline.prog.get(), "vertex");
        graphPipeline.view_loc   = glGetUniformLocation(graphPipeline.prog.get(), "view");
        graphPipeline.proj_loc   = glGetUniformLocation(graphPipeline.prog.get(), "proj");


        // Setup vbo
        graphPipeline.vertex_buffer.reset(gl::create_buffer());
        graphPipeline.index_buffer.reset(gl::create_buffer());

        return true;
    }

    void updateGraphPipeline()
    {
        // Store OpenGL state
        GLint array_buffer_binding = 0;
        GLint element_array_buffer_binding = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_array_buffer_binding);

        glBindBuffer(GL_ARRAY_BUFFER, graphPipeline.vertex_buffer.get());
        glBufferData(GL_ARRAY_BUFFER,
                     graphBack.nodes.size() * sizeof(vec2),
                     graphBack.nodes.data(),
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphPipeline.index_buffer.get());
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     graphBack.edges.size() * sizeof(vec2ui),
                     graphBack.edges.data(),
                     GL_STATIC_DRAW);

        // Restore OpenGL state
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_binding);
        glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    }

    renderer(bool active=false, bool benchmarkMode=false)
        : viewer_glut(1024, 1024, "Spring Embedder"), active(active), benchmarkMode(benchmarkMode)
    {
    }

    void on_display()
    {
        if(requestExit) { 
            quit();
            return;
        }

        if (!glInitialized)
        {
            glewInit();

            glClearColor(1,1,1,1);
            glEnable(GL_DEPTH_TEST);

            glInitialized = buildGraphPipeline();

            debugcb.activate();
        }

        if (imageUpdated)
        {
            std::unique_lock<std::mutex> l(mtx);

            graphBack = graph;

            updateGraphPipeline();

            imageUpdated = false;
        }

        // Store OpenGL state
        GLint array_buffer_binding = 0;
        GLint element_array_buffer_binding = 0;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer_binding);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_array_buffer_binding);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw buffers

        graphPipeline.prog.enable();

        glUniformMatrix4fv(graphPipeline.view_loc, 1, GL_FALSE, cam.get_view_matrix().data());
        glUniformMatrix4fv(graphPipeline.proj_loc, 1, GL_FALSE, cam.get_proj_matrix().data());

        glBindBuffer(GL_ARRAY_BUFFER, graphPipeline.vertex_buffer.get());
        glEnableVertexAttribArray(graphPipeline.vertex_loc);
        glVertexAttribPointer(graphPipeline.vertex_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphPipeline.index_buffer.get());

        glDrawElements(GL_LINES, graphBack.edges.size()*2, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(graphPipeline.vertex_loc);

        graphPipeline.prog.disable();

        // Restore OpenGL state
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer_binding);
        glBindBuffer(GL_ARRAY_BUFFER, array_buffer_binding);
    }

    void on_key_press(visionaray::key_event const& event)
    {
        if(!benchmarkMode) {
        static const std::string camera_file_base = "visionaray-camera";
        static const std::string camera_file_suffix = ".txt";
        
        switch (event.key())
        {
        case visionaray::keyboard::Space:
            {
                std::unique_lock<std::mutex> l(mtx);
                active = !active;
            }
            break;

        case 'p':
            {
                graphBack.printStatistics(true);
            }
            break;

        case 's':
            {
                std::cout << "Save tulip dump\n";
                //if (!cmd.sOutputFile.empty())
                //    graphBack.saveAsTlp(cmd.sOutputFile);
                //else
                    graphBack.saveAsTlp("dump.tlp");
            }
            break;

        case 'u':
            {
                int inc = 0;
                std::string inc_str = "";

                std::string filename = camera_file_base + inc_str + camera_file_suffix;

                while (boost::filesystem::exists(filename))
                {
                    ++inc;
                    inc_str = std::to_string(inc);

                    while (inc_str.length() < 4)
                    {
                        inc_str = std::string("0") + inc_str;
                    }

                    inc_str = std::string("-") + inc_str;

                    filename = camera_file_base + inc_str + camera_file_suffix;
                }

                std::ofstream file(filename);
                if (file.good())
                {
                    std::cout << "Storing camera to file: " << filename << '\n';
                    file << cam;
                }
            }
            break;

        case 'v':
            {
                std::string filename = camera_file_base + camera_file_suffix;

                load_camera(filename);
            }
            break;
        }
        }

        visionaray::viewer_glut::on_key_press(event);
        
    }

    void load_camera(std::string filename)
    {
        std::ifstream file(filename);
        if (file.good())
        {
            file >> cam;
            std::cout << "Load camera from file: " << filename << '\n';
        }
    }


    Graph graph;
    Graph graphBack;
    image_rgb8 img = {1024, 1024};

    GLuint tex;

    bool glInitialized = false;

    gl::debug_callback debugcb;

    bool imageUpdated = false;

    bool active = false;

    std::mutex mtx;

    pinhole_camera cam;

    timer total_time;
};



void genCommandLineOptions(Cmd &cmd, cmdline_options &options) {
    options.emplace_back(cl::makeOption<std::string&>(
        cl::Parser<>(),
        "camera",
        cl::Desc("Text file with camera parameters"),
        cl::ArgRequired,
        cl::init(cmd.initial_camera)
        ) );

     options.emplace_back(cl::makeOption<int&>(
        cl::Parser<>(), "n",
        cl::ArgName("int"),
        cl::ArgOptional,
        cl::init(cmd.maxIterations),
        cl::Desc("Maximum number of iterations")
        ));
   
     options.emplace_back(cl::makeOption<int&>(
        cl::Parser<>(), "r",
        cl::ArgName("int"),
        cl::ArgOptional,
        cl::init(cmd.loadRepetitions),
        cl::Desc("Number of repetitions when loading data")
        ));

     options.emplace_back(cl::makeOption<bool&>(
        cl::Parser<>(), "bench",
        cl::ArgName("bool"),
        cl::ArgOptional,
        cl::init(cmd.benchmarkMode),
        cl::Desc("Benchmarking mode to disable keypress event handling")
        ));

     options.emplace_back(cl::makeOption<std::string&>(
        cl::Parser<>(), "o",
        cl::ArgName("string"),
        cl::ArgOptional,
        cl::init(cmd.sOutputFile),
        cl::Desc("Output tlp file")
        ));

    options.emplace_back(cl::makeOption<LayouterMode&>({
            {"naive",         LayouterMode::Naive,          "Naive Implementation"},
            { "rtx",          LayouterMode::RTX,          "RTX Mode"             },
            { "lbvh",         LayouterMode::LBVH,          "LBVH Mode"              },
        },
        "mode",
        cl::ArgOptional,
        cl::init(cmd.layouterMode),
        cl::Desc("Select graph layout mode")
        ));


    options.emplace_back(cl::makeOption<eInputSource&>({
            {"artificial",         eInputSource::ARTIFICIAL,          "Artificial Graph Generation"},
            { "tree",          eInputSource::TREE,          "Tree Generation"             },
            { "file",         eInputSource::FILEINPUT,          "File Input"              },
        },
        "dt",
        cl::ArgOptional,
        cl::init(inputSource),
        cl::Desc("Select data generation mode")
        ));


    options.emplace_back(cl::makeOption<unsigned&>(
        cl::Parser<>(), "refit_after",
        cl::ArgOptional,
        cl::init(cmd.rebuildRTXAccelAfter),
        cl::Desc("Refit RTX BVH after N iterations")
        ));

   //Artificial
    options.emplace_back(cl::makeOption<int&>(cl::Parser<>(), "C", cl::ArgName("int"),
        cl::ArgOptional,cl::init(cmd.artificialOptions.Clusters),cl::Desc("Clusters")
        ));

    options.emplace_back(cl::makeOption<int&>(cl::Parser<>(), "npc", cl::ArgName("int"),
        cl::ArgOptional,cl::init(cmd.artificialOptions.NodesPerCluster),cl::Desc("Nodes per Clusters")
        ));

    options.emplace_back(cl::makeOption<int&>(cl::Parser<>(), "epc", cl::ArgName("int"),
        cl::ArgOptional,cl::init(cmd.artificialOptions.EdgesPerCluster),cl::Desc("Edges per Clusters")
        ));

    options.emplace_back(cl::makeOption<bool&>(cl::Parser<>(), "connected", cl::ArgName("bool"),
        cl::ArgOptional,cl::init(cmd.artificialOptions.Connected),cl::Desc("Generate connected graph")
        ));
 
    options.emplace_back(cl::makeOption<int&>(cl::Parser<>(), "trDepth", cl::ArgName("int"),
        cl::ArgOptional,cl::init(cmd.treeOptions.depth),cl::Desc("Tree data generation depth")
        ));

    options.emplace_back(cl::makeOption<int&>(cl::Parser<>(), "trDegree", cl::ArgName("int"),
        cl::ArgOptional,cl::init(cmd.treeOptions.degree),cl::Desc("Tree data generation degree")
        ));

}


int main(int argc, char** argv)
{
    renderer rend;
	
    Cmd cmd;
    cmdline_options options;
 
    genCommandLineOptions(cmd, options);

    for(auto &opt : options) {
    	rend.add_cmdline_option(opt);
    }

    support::cl::CmdLine& cmdInst = rend.cmd_line_inst();
    auto cInputFileList = cl::makeOption<std::vector<std::string>>(
        cl::Parser<>(), cmdInst,
        "files",
        cl::ArgName("file list"),
        cl::ArgOptional,
        cl::Desc("A list of input files"),
        cl::Positional,
        cl::ZeroOrMore
        );

    try
    {
        rend.init(argc, argv);
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    rend.active = cmd.benchmarkMode;
    rend.benchmarkMode = cmd.benchmarkMode;

    for(const auto& filename : cInputFileList->value()) {
	    std::cout << "Inputfile: " << filename << std::endl;
        inputSource = eInputSource::FILEINPUT;
	    cmd.inputFiles.push_back(filename);
    }

    std::thread thrd([&](){
        if (!cmd.initial_camera.empty()) {
            rend.load_camera(cmd.initial_camera);
        }
        for(int i=0; i < cmd.loadRepetitions;i++) {

            switch(inputSource) {
                case eInputSource::ARTIFICIAL:
                    load_artificial(rend.graph,cmd.artificialOptions);
                break;

                case eInputSource::TREE:
                    load_complete_tree(rend.graph,cmd.treeOptions.depth, cmd.treeOptions.degree);
                break;

                case eInputSource::FILEINPUT:
                    for(auto f : cmd.inputFiles) {
                        std::cout << "Loading file <" << f << ">\n";
                        // TODO: make this more general
                        if (getExt(f) == ".csv")
                            load_gephi_csv(rend.graph,f);
                        else
                            load_deezer(rend.graph,f);
                    }
                break;

            }
        }

        Layouter l(rend.graph, rend.img, rend.mtx, cmd.layouterMode, cmd.rebuildRTXAccelAfter);
        {
            std::unique_lock<std::mutex> l(rend.mtx);
            rend.imageUpdated = true;
        }

#ifndef VERBOSE
	    rend.graph.printStatistics(true);
#endif

        for (int noIterations=0; ; )
        {
            bool active = false;
            {
                std::unique_lock<std::mutex> l(rend.mtx);
                if (!rend.active)
                    rend.total_time.reset();
                active = rend.active;
            }

            if (active)
            {
                l.iterate();
                {
                    std::unique_lock<std::mutex> l(rend.mtx);
                    rend.imageUpdated = true;
                } 

                if(noIterations==cmd.maxIterations) {
                    std::cout << "Total time for " << noIterations << " iterations: " << rend.total_time.elapsed() << " sec.\n";
                    if(cmd.sOutputFile.compare("")) {
                        std::cout << "Writing file <" << cmd.sOutputFile << "> ... ";
                        rend.graph.saveAsTlp(cmd.sOutputFile);
                        std::cout << "done" << std::endl;
                    }
                    {
                        std::unique_lock<std::mutex> l(rend.mtx);
                        rend.requestExit = true;
                    }
                    break;
                }
                ++noIterations;
            }
        }
    });

    rend.cam.set_viewport(0, 0, rend.width(), rend.height());
    rend.cam.perspective(45.0f * constants::degrees_to_radians<float>(), 1.f, 0.001f, 1000.0f);
    aabb bbox({0,0,0},{1,1,1}); // TODO: reset camera from worker thread when the data set was loaded
    rend.cam.view_all(bbox);
    rend.add_manipulator( std::make_shared<pan_manipulator>(rend.cam, mouse::Left) );
    rend.add_manipulator( std::make_shared<pan_manipulator>(rend.cam, mouse::Middle) );
    rend.add_manipulator( std::make_shared<zoom_manipulator>(rend.cam, mouse::Right) );

    rend.event_loop();
    thrd.join();

    return 0;
}
