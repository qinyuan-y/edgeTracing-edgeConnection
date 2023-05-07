#ifndef EDGEPROCESSOR_H_
#define EDGEPROCESSOR_H_

// General includes
#include <iostream>

// OpenCV
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>

#include "EdgeIdMap.h"
#include "Edges.h"
#include "ConfigParser.h"

class EdgeProcessor
{
public:
	/** Constructor
	 */
	EdgeProcessor();

	/** Main function for edge tracing.
	 *  @img 			Input Image.
	 */
	void traceEdges(cv::Mat &img);

	/** Print information about input image and traced edges.
	 */
	void printEdgeInfos(cv::Mat &img);

	/** Get read-only reference to edgeIdMap.
	 */
	const EdgeIdMap &getEdgeIdMap() const;

	/** Get read-only reference to edge vector.
	 */
	const Edges &getEdges() const;

private:
	int edgeId; //!< Running edge id for tracing.

	Edges edges; //!< Traced edges.

	EdgeIdMap edgeIdMap; //!< Image with edge ids.

	ConfigParser configParser; //!< ConfigParser with config data structure

	/** Get direct neighboring pixels around point p clockwise from top left.
	 * 	@img			Input Image
	 * 	@p				Point p
	 */
	std::vector<cv::Point> getDirectNeighbours(cv::Mat &img, cv::Point p);

	/** Create a binary code for the 8-neighbourhood around point p
	 * 	@img			Input Image
	 * 	@p				Point p
	 */
	uint8_t getNeighboursBinary(cv::Mat &img, cv::Point p);

	/** Preprocesses the picture and search for special points mentioned in the config
	 * 	@img 			Input Image
	 */
	void preprocessCluster(cv::Mat &img);

	/** Tracing function which is called recursively.
	 * 	@img			Input Image
	 * 	@startPoint		Current point that is traced
	 * 	@edge			Current edge to which the point belongs
	 */
	void traceEdge(cv::Mat &img, cv::Point startPoint, std::vector<cv::Point> &edge);

	/** Function to merge two edges together
	 *  @firstId		Id of first edge
	 *  @secondId		Id of second edge
	 */
	void mergeEdges(int firstId, int secondId);
};

#endif /* EDGEPROCESSOR_H_ */
