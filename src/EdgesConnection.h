#ifndef EDGESCONNECTION_H
#define EDGESCONNECTION_H

// General includes
#include <iostream>
#include <math.h>

// OpenCV
#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "EdgeIdMap.h"
#include "Edges.h"

class EdgesConnection
{
public:
	/** Constructor.
	 */
	EdgesConnection();

	/** Main function for edge connection.
	 *  @img 			Input Image
	 *  @edges			Vector with traced edges from EdgeProcessor
	 *  @edgeIdMap		Indices of traced edges from EdgeProcessor
	 */
	std::vector<std::vector<cv::Point>> edgesConnectionG(cv::Mat &img, Edges &edges, EdgeIdMap &edgeIdMap);

private:

	std::vector<std::vector<int>> edgeCombinations; // Edge combinations found in the recursive function "edgeCombine"
	std::vector<int> tempEdgeCombination; // Temporary edge combination in the recursive function "edgeCombine"
	std::vector<int> combinedEdgeIds; // Each element is the edgeId that is already combined in "completeEdgeCombinations"

	/** Recursive function for finding all the edge combinations starting with "targetEdgeId" and "targetPixel"
	 *  @targetEdgeId		The starting edge of the recursive function
	 *  @targetPixel 		The starting pixel of the recursive funtion (the first or the end pixel of the "targetEdge")
	 *  @img				Input image
	 *  @edges				Vector with traced edges from EdgeProcessor
	 *  @edgeIdMap			Indices of traced edges from EdgeProcessor
	 */
	void edgeCombine(int targetEdgeId, cv::Point targetPixel, cv::Mat &img, Edges &edges, EdgeIdMap &edgeIdMap);

	/** This function return all the possibilities of combining edge combinations in "firstPartEdgeCombinations" and "secondPartEdgeCombinations"
	 *  @firstPartEdgeCombinations
	 *  @secondPartEdgeCombinations
	 *  @edges 							Vector with traced edges from EdgeProcessor
	 */
	std::vector<std::vector<int>> CombinePartialEdgeCombinations(std::vector<std::vector<int>> firstPartEdgeCombinations, std::vector<std::vector<int>> secondPartEdgeCombinations, Edges edges);

	/** Compute costs of each edge combination.
	 *  @completeEdgeCombinations	Found edge combinations (each element is a vector of edgeIds)
	 *  @edges						Vector with traced edges from EdgeProcessor
	 */
	std::vector<double> ComputeCosts(cv::Mat &img, std::vector<std::vector<int>> completeEdgeCombinations, Edges edges);

	/*	Delete combined edges in "completeEdgeCombinations" which contains edgeID
	 *  that already exist in lower cost edge combinations.
	 *  @completeEdgeCombinations	Found edge combinations (each element is a vector of edgeIds)
	 *  @connectedEdgeIds           A vector to store edgeIds that are already exist in lower cost edge combinations.
	 */
	void SiftEdgeCombinations(std::vector<std::vector<int>> &completeEdgeCombinations, std::vector<int> &connectedEdgeIds);

	/** Compute pixel coordinate sequences of edge combinations in "completeEdgeCombinations"
	 * 	@completeEdgeCombinations	Found edge combinations (each element is a vector of edgeIds)
	 *  @edges						Vector with traced edges from EdgeProcessor
	 */
	std::vector<std::vector<cv::Point>> ComputeEdgePixelSequences(std::vector<std::vector<int>> completeEdgeCombinations, Edges edges);

	/** Sort the costs of combinations from small to large
	 *
	 */
	int Partition(std::vector<double> &costs, std::vector<std::vector<int>> &completeEdgeCombinations, int start, int end);
	void Quicksort(std::vector<double> &costs, std::vector<std::vector<int>> &completeEdgeCombinations, int start, int end);

	/** The maximum gray value difference when searching for edge combinations.
	 *  Two edges around a ambiguous point whose gray value difference is smaller than "grayValueThreshold" could be taken into consideration
	 */
	int grayValueThreshold = 80;

	/** Length(in pixel) as a parameter for computing the edge direction cost
	 */
	std::size_t length = 30;

	/** Weights for computing costs during edge connection
	 *  L: edge length            lengthCost = L * length (in pixel)
	 *  D: edge direction         directionCost = D * cos ("cos" is the cosine value of the angle between two edges)
	 *  G: gray value difference  grayValueCost = G * grayValueDifference
	 *  C: closed edge            closedEdgeCost = C, if the edge is a closed edge
	 */
	double L = -0.005;
	double D = 0.25;
	double G = 0.1;
	double C = -2;
};
#endif // EDGESCONNECTION_H

