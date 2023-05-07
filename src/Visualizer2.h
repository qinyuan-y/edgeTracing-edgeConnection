#ifndef VISUALIZER2_H
#define VISUALIZER2_H

// General includes
#include <iostream>
#include <string>
// OpenCV
#include <opencv2/core/core.hpp>

#include "EdgeIdMap.h"

class Visualizer2
{
public:
	/** Write SVG Visualization based on edges and the clusters from edgeIdMap.
	 *  @img Input image.
	 * 	@edges Internal class for traced edges.
	 * 	@edgeIdMap Internal class for edgeId representation.
	 * 	@markStartEnd Mark start- and endpoints with circles.
	 *  @showInput Draw input image.
	 */
	// static void saveEdgesAsSVG(cv::Mat &img, const Edges &edges, const EdgeIdMap &edgeIdMap, bool markStartEnd = true, bool showInput = false);

	/** Write SVG Visualization based on completeEdgeCombinationSequences.
	 * 	@completeEdgeCombinationSequences Edge sequences from EdgesConnection.
	 */
	static void saveConnectedEdgesAsSVG(cv::Mat &img, const EdgeIdMap &edgeIdMap, std::vector<std::vector<cv::Point>> completeEdgeCombinationSequences, std::string filename);
};

#endif // VISUALIZER2_H
