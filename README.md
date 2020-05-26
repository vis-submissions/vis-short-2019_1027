VIS 2020 Shortpaper Submission No. 1027
=======================================

This is a development version of the source code accompanying the VIS 2020 shortpaper submission no. 1027.

We intend to publish this code on github under an open source license (probably MIT) on acceptance.

Note that the code is not yet refactored in a way that is suitable for publication. There are several dependencies on 3rd-party libraries (most notably Visionaray) that we want to get rid of eventually.

Hence, this code base is only meant as a quick orientation for the potential reviewer who is willing or curious to take a look into the source code we build our method upon.

Source Code Overview
--------------------

While some of the source code files merely contain "boilerplate code", the following implementation files might be of interest for a potential review:

Implementation of the graph drawing framework: [gd.h](/gd.h) / [gd.cu](/gd.cu)
These files implement the various phases of the graph drawing algorithm with CUDA. For the repulsive phase, the files contain a simple CUDA kernel implementing the naive method, or alternatively call into the OWL/OptiX or LBVH nearest neighbor programs and kernels that are implemented elsewhere.

Implementation of the OptiX device programs: [optixSpringEmbedder.h](/optixSpringEmbedder.h) / [optixSpringEmbedder.cu](/optixSpringEmbedder.cu)
These files contain the optix intersection and bounds programs that implement the ray tracing-based nearest neighbor query. For the OWL/host side, see the routines in [gd.cu](/gd.cu) that call into these programs.

Implementation of the LBVH data structure: [lbvh.h](/lbvh.h) / [lbvh.cu](/lbvh.cu)
These files contain the implementation of our reference method based on the fast BVH tree construction algorithm by Karras.

Implementation of the user interface: [main.cpp](/main.cpp)
This file implements the user interface. The graph drawing algorithm runs in a separate thread from the user interface. We use a simple OpenGL renderer that only draws the graph edges as GL lines. This is not how we generated the images for the paper; for that,we rather use the software [Tulip](https://tulip.labri.fr/TulipDrupal/). Our graph layouts can be  exported to the format supported by Tulip.
