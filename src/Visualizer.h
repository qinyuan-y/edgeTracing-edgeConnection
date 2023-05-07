#ifndef VISUALIZER_H
#define VISUALIZER_H

// General includes
#include <iostream>

// OpenCV
#include <opencv2/core/core.hpp>

#include "EdgeIdMap.h"
#include "Edges.h"

class Visualizer
{
public:
	/** Write SVG Visualization based on edges and the clusters from edgeIdMap.
	 *  @img Input image.
	 * 	@edges Internal class for traced edges.
	 * 	@edgeIdMap Internal class for edgeId representation.
	 * 	@markStartEnd Mark start- and endpoints with circles.
	 *  @showInput Draw input image.
	 */
	static void saveEdgesAsSVG(cv::Mat &img, const Edges &edges, const EdgeIdMap &edgeIdMap, bool markStartEnd = true, bool showInput = false);

	/** Write SVG Visualization based on edgeIdMap only.
	 * 	@edgeIdMap Internal class for edgeId representation.
	 */
	static void saveEdgeIdMapAsSVG(const EdgeIdMap &edgeIdMap);
};

#endif // VISUALIZER_H
