#ifndef EDGEIDMAP_H
#define EDGEIDMAP_H

// General includes
#include <iostream>
#include <opencv2/core/types.hpp>

class EdgeIdMap
{
public:
	/** Constructor.
	 */
	EdgeIdMap();

	/** Initialize the data structure maintained by this class.
	 */
	void init(int rows, int cols);

	/**	Push back edge id at given position.
	 */
	void pushBack(int x, int y, int edgeId);

	/**	Push back cluster point at given position.
	 */
	void pushBackCluster(int x, int y, std::vector<cv::Point> cluster);

	/** Erase Edge Id on given position
	 */
	void eraseEdgeId(int x, int y, int edgeId);

	/** Number of indices at given position.
	 */
	int getSize(int x, int y) const;

	/** Number of image columns.
	 */
	int getCols() const;

	/** Number of image rows.
	 */
	int getRows() const;

	/** Get read-only reference to edge ids.
	 */
	const std::vector<int> &getEdgeIds(int x, int y) const;

	/** Get read-only reference to cluster points
	 */
	const std::vector<cv::Point> &getCluster(int x, int y) const;

	/** Number of different edges in edge id map.
	 */
	int getMaxEdgeId() const;

private:
	std::vector<std::vector<int>> data;				 //!< 1D data structure representing the 2D image with edge indices.
	std::vector<std::vector<cv::Point>> dataCluster; //!< 1D data structure representing the 2D image with points of cluster

	int cols; //!< Number of columns to convert between 1D (data) and 2D (x, y).
	int rows; //!< Number of rows
};

#endif // EDGEIDMAP_H
