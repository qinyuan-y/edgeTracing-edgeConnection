#ifndef EDGES_H
#define EDGES_H

// OpenCV
#include <opencv2/core/types.hpp>

class Edges
{
public:
	/** Constructor.
	 */
	Edges();

	void clear();

	void pushBack(std::vector<cv::Point> edge);

	void insert(int position, std::vector<cv::Point> edge);

	void popBack();

	void eraseEdge(std::vector<cv::Point> edge);

	const std::vector<cv::Point> &getEdge(int index) const;

	int size() const;

	const std::vector<std::vector<cv::Point>> &getEdges() const;

private:
	/*  Vector with traced edges. Each edge is a vector of points (std::vector<cv::Point>).
	 *  Position of each edge in edges corresponds to edgeId in EdgeIdMap.
	 */
	std::vector<std::vector<cv::Point>> data;
};

#endif // EDGES_H
