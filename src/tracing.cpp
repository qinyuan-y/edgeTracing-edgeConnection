// General includes
#include <iostream>
#include <string>

//#include <fstream>
#include <fstream>

// OpenCV
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

// Classes for config, tracing and result visualization
#include "ConfigParser.h"
#include "EdgeProcessor.h"
#include "Visualizer.h"
#include "EdgesConnection.h"
#include "Visualizer2.h"

int main(int argc, const char *argv[])
{
	std::ofstream outfile;
	cv::Mat img;
	if (argc > 1)
	{
		std::cout << "Read Image..." << argv[1] << std::endl;
		img = cv::imread(argv[1], 0);
//		// Threshold input edge images when needed
//		cv::Mat dst;
//		cv::threshold(img, dst, 110, 255, 3);
//		img = dst;
		if (!img.data)
		{
			std::cout << "Could not find or open image. Quit." << std::endl;
			return -1;
		}
	}
	else
	{
		std::cout << argc << argv[0] << "Invalid number of arguments. Quit." << std::endl;
		return -1;
	}

	//Trace edges
	EdgeProcessor edgeProcessor;
    clock_t t3 = clock();
	edgeProcessor.traceEdges(img);
	clock_t t4 = clock();
	std::cout << "traceEdges took: " << double(t4 - t3) / CLOCKS_PER_SEC << "s" << std::endl;
	/*outfile.open("benchmark.txt", std::ios_base::app); // append instead of overwrite
	outfile << double(t4 - t3)/ CLOCKS_PER_SEC;
	outfile << ", ";
	outfile.close();*/

	edgeProcessor.printEdgeInfos(img);

	// Get read-only references to internal edges and edgeIdMap
	Edges edges = edgeProcessor.getEdges();
	EdgeIdMap edgeIdMap = edgeProcessor.getEdgeIdMap();

	// Edge Connection
	clock_t t5 = clock();
	std::vector<std::vector<cv::Point>> completeEdgeCombinationSequences;
	EdgesConnection edgesConnection;
	completeEdgeCombinationSequences = edgesConnection.edgesConnectionG(img, edges, edgeIdMap);
	// If the number of edge combinations exceed the restrictions (details can be found in the function EdgesConnection::edgesConnectionG)
	// the funtion "edgesConnection.edgesConnection" will return 0.
	if (completeEdgeCombinationSequences.size() == 0)
	{
		std::cout << "The number of edge combinations exceed the restrictions, considering thresholding the input image with greater threshold" << std::endl;
		return 0;
	}
	clock_t t6 = clock();
	std::cout << "edgeConnection took: " << double(t6 - t5) / CLOCKS_PER_SEC << "s" << std::endl;

	// Visualization of internal edges and edgeIdMap
	Visualizer::saveEdgesAsSVG(img, edges, edgeIdMap, true, false);
	Visualizer::saveEdgeIdMapAsSVG(edgeIdMap);

	// Visualization of connected edges
	Visualizer2::saveConnectedEdgesAsSVG(img, edgeIdMap, completeEdgeCombinationSequences, "connected_edges");

	std::cout << "Finished." << std::endl;
	return 0;
}
