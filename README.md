# edgeTracing-edgeConnection

# 1. edgeTracing(Original codes are from the GetLab https://getwww.uni-paderborn.de/)

INTRODUCTION
------------

This algorithm traces edge pixels in an ordered sequence. It takes various image file formats as input (*.jpg, *.png etc.) and generates a visualization of the traced edges. Furthermore, the algorithm has a model to save ambiguous pixels on edge junctions or pixel clusters. 

Example usage:

tracing ./images/example.png 

REQUIREMENTS
------------

This algorithm requires the OpenCV (https://opencv.org) module.

CONFIGURATION
-------------

The ambiguous pixel can be configured with the configuration file config.csv located in the ./config folder. To configure manually, the config_gui.py can be used. The config.csv file is read by the algorithm to connect the edges of the ambiguity accordingly.

BUILDING
-------------
Build by making a build directory (i.e. build/), run cmake in that dir to build the executable.

mkdir build && cd build
cmake ..
./tracing path-to-image/example-img.png

STRUCTURE
-------------
The main sources are in src/. The files for the configuration are in config/.

.
├── CMakeLists.txt
├── config
│   ├── config_default.csv
│   ├── config.csv
│   ├── config_gui.py
│   └── config_parser.py
└── src
    ├── ConfigParser.cpp
    ├── ConfigParser.h
    ├── EdgeIdMap.cpp
    ├── EdgeIdMap.h
    ├── EdgeProcessor.cpp
    ├── EdgeProcessor.h
    ├── Edges.cpp
    ├── Edges.h
    ├── tracing.cpp
    ├── Visualizer.cpp
    └── Visualizer.h
    
# 2. edgeConnection

INTRODUCTION
------------
An edge connection algorithm is integrated in the tracing algorithm to connect edges around ambiguous pixels based on the tracing results.

CAUTIONS
------------
In the source folder, 4 new files are added: 
EdgesConnection.cpp
EdgesConnection.h
Visualizer2.cpp
Visualizer2.h

There are 6 hyper-parameters can be set by users to adjust the connection: 
"grayValueThreshold"
"length"
"L"
"D"
"G"
"C"

They can be set and are explained in the private part of the class "EdgesConnection" in file "EdgesConnection.h".

USAGE
------------
The usage of the new algorithm follows the edgeTracing algorithm.
A file called "connected_edges" will be written in the corresponding "build" folder which shows the results after connection.

# Visualized Results

<img width="1035" alt="image" src="https://user-images.githubusercontent.com/125279855/236662011-200b9462-0002-4c31-925d-b47fff3429be.png">

Pixels with the same color belongs to the same traced edge.
