#include "EdgeProcessor.h"

// Mask to find four-clusters in corners
#define UPPER_LEFT 193  	// 11000001
#define UPPER_RIGHT 112 	// 01110000
#define LOWER_RIGHT 28  	// 00011100
#define LOWER_LEFT 7	 	// 00000111

// Constructor
EdgeProcessor::EdgeProcessor()
{
	std::cout << "Object created: EdgeProcessor\n";
}

void EdgeProcessor::traceEdges(cv::Mat &img)
{
	// Reset
	edgeId = 0;
	edges.clear();

	// Initialize edgeIdMap for edge indices
	edgeIdMap.init(img.rows, img.cols);
	configParser.readConfig("./../config/config.csv");

	// Preprocessing: Searching for clusters or ambiguous points
	preprocessCluster(img);

	// Iterative part: search for unvisited edge pixels
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			// edgeIdMap.getSize(x, y) == 0 means unvisited
			if (img.at<uchar>(y, x) > 0 && edgeIdMap.getSize(x, y) == 0 && edgeIdMap.getCluster(x, y).size() == 0)
			{
				// Start new edge as pixel is unvisited (not traced before)
				std::vector<cv::Point> edge;

				// Main tracing function
				traceEdge(img, cv::Point(x, y), edge);
			}
		}
	}
}

void EdgeProcessor::preprocessCluster(cv::Mat &img)
{
	// Search for clusters and process them

	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			if (img.at<uchar>(y, x) > 0 && edgeIdMap.getCluster(x, y).size() == 0) // Only check white and unclustered pixel
			{
				cv::Point point = cv::Point(x, y);
				uint8_t neighboursBinary = getNeighboursBinary(img, point);
				std::vector<cv::Point> neighbours = getDirectNeighbours(img, point);
				std::vector<cv::Point> cPoints;
				// Identify four-cluster or ambiguous point
				if ((neighboursBinary & UPPER_LEFT) == UPPER_LEFT ||
					(neighboursBinary & UPPER_RIGHT) == UPPER_RIGHT ||
					(neighboursBinary & LOWER_RIGHT) == LOWER_RIGHT ||
					(neighboursBinary & LOWER_LEFT) == LOWER_LEFT ||
					neighbours.size() > 2)
				{
					cPoints.push_back(point);
					int c = 0;

					// Identify all pixel of the cluster
					while (c < (int)cPoints.size())
					{
						std::vector<cv::Point> neighbours = getDirectNeighbours(img, cPoints[c]);
						for (auto n : neighbours)
						{
							if (std::find(cPoints.begin(), cPoints.end(), n) == cPoints.end())
							{
								uint8_t nBin = getNeighboursBinary(img, n);
								std::vector<cv::Point> nNeighbours = getDirectNeighbours(img, n);
								if ((nBin & UPPER_LEFT) == UPPER_LEFT ||
									(nBin & UPPER_RIGHT) == UPPER_RIGHT ||
									(nBin & LOWER_RIGHT) == LOWER_RIGHT ||
									(nBin & LOWER_LEFT) == LOWER_LEFT ||
									nNeighbours.size() > 2)
								{
									cPoints.push_back(n);
								}
							}
						}
						c++;
					}

					// Save information about cluster in all points of cluster
					for (auto i : cPoints)
					{
						edgeIdMap.pushBackCluster(i.x, i.y, cPoints);
					}
				}
			}
		}
	}
}

// This function is called recursively
void EdgeProcessor::traceEdge(cv::Mat &img, cv::Point startPoint, std::vector<cv::Point> &edge)
{
	// Add startPoint to new edge
	edge.push_back(startPoint);
	edgeIdMap.pushBack(startPoint.x, startPoint.y, edgeId);

	// Get edge pixels around point clockwise from top left
	std::vector<cv::Point> neighbours = getDirectNeighbours(img, startPoint);
	std::vector<cv::Point> unvisitedNeighbours;

	// Only consider all unvisited neighbours
	for (auto point : neighbours)
	{
		auto cluster = edgeIdMap.getCluster(point.x, point.y);
		auto tempIds = edgeIdMap.getEdgeIds(point.x, point.y);
		if (cluster.size() > 1)
		{
			if (edge.size() == 1)
			{
				edge.insert(edge.begin(), point);
			}
			else
			{
				edge.push_back(point);
			}
			edgeIdMap.pushBack(point.x, point.y, edgeId);
		}
		else if (edgeIdMap.getSize(point.x, point.y) == 0)
		{
			unvisitedNeighbours.push_back(point);
		}
	}

	if (edge.size() == 1) // New edge (point was just found)
	{
		if (unvisitedNeighbours.size() == 2) // Go into both directions
		{
			// Save id of first edge
			std::vector<cv::Point> edgePartOne;
			std::vector<cv::Point> edgePartTwo;
			edgePartOne.push_back(startPoint);

			// Trace edges in both directions
			traceEdge(img, unvisitedNeighbours[0], edgePartOne);
			int firstId = edgeIdMap.getEdgeIds(unvisitedNeighbours[0].x, unvisitedNeighbours[0].y).front();
			int secondId = -1;

			// Dont trace both edges if edges are converging
			if (edgeIdMap.getSize(unvisitedNeighbours[1].x, unvisitedNeighbours[1].y) == 0)
			{
				edgePartTwo.push_back(startPoint);
				edgeIdMap.pushBack(startPoint.x, startPoint.y, edgeId);
				traceEdge(img, unvisitedNeighbours[1], edgePartTwo);
				secondId = edgeIdMap.getEdgeIds(unvisitedNeighbours[1].x, unvisitedNeighbours[1].y).back();
			}

			// Merge edges if not converging edges
			if (secondId != -1)
			{
				mergeEdges(firstId, secondId);
			}
		}
		else if (unvisitedNeighbours.size() == 1) // Follow continuity
		{
			traceEdge(img, unvisitedNeighbours[0], edge);
		}
		else if (unvisitedNeighbours.size() == 0) // End edge
		{
			cv::Point addedNeighbour;
			std::vector<int> idsEndpoint = edgeIdMap.getEdgeIds(startPoint.x, startPoint.y);
			// Check if end of edge is connected to another edge
			for (auto point : neighbours)
			{
				std::vector<cv::Point> edgeTemp = edge;
				std::vector<int> idsNeighbour = edgeIdMap.getEdgeIds(point.x, point.y);
				// Check if both point don't have common ids
				if (std::find_first_of(idsEndpoint.begin(), idsEndpoint.end(),
									   idsNeighbour.begin(), idsNeighbour.end()) == idsEndpoint.end())
				{
					edgeTemp.push_back(point);
					edgeIdMap.pushBack(point.x, point.y, edgeId);
					addedNeighbour = point;
					// Finish current edge to be able to process further
					edges.insert(edgeId, edgeTemp);
					edgeId++;

					std::vector<int> ids = edgeIdMap.getEdgeIds(addedNeighbour.x, addedNeighbour.y);

					// Two edges can be merged only
					if (ids.size() == 2)
					{
						std::vector<cv::Point> firstEdge = edges.getEdge(ids.front());
						std::vector<cv::Point> secondEdge = edges.getEdge(ids.back());

						// Only merge start- and endpoints
						if ((addedNeighbour == firstEdge.front() || addedNeighbour == firstEdge.back()) &&
							(addedNeighbour == secondEdge.front() || addedNeighbour == secondEdge.back()))
						{
							mergeEdges(ids.front(), ids.back());
						}
					}
				}
			}
			std::vector<int> ids = edgeIdMap.getEdgeIds(startPoint.x, startPoint.y);

			// Two edges can be merged only
			if (ids.size() == 2)
			{
				std::vector<cv::Point> firstEdge = edges.getEdge(ids.front());
				std::vector<cv::Point> secondEdge = edges.getEdge(ids.back());

				// Only merge start- and endpoints

				mergeEdges(ids.front(), ids.back());
			}
		}
	}

	else if (edge.size() != 1) // Already existing edge
	{
		if (unvisitedNeighbours.size() > 1)
		{
			edges.pushBack(edge);
			edgeId++;
			for (auto i : unvisitedNeighbours)
			{
				if (edgeIdMap.getSize(i.x, i.y) == 0)
				{
					// Create a new edge for each neighbour and trace
					std::vector<cv::Point> newEdge;
					newEdge.push_back(startPoint);
					edgeIdMap.pushBack(startPoint.x, startPoint.y, edgeId);
					traceEdge(img, i, newEdge);
				}
			}

			// Compare with config
			uint8_t neighboursBinary = getNeighboursBinary(img, startPoint);
			std::vector<std::string> configLine = configParser.getData(neighboursBinary);
			std::vector<int> idsOfConfigEdge;
			for (auto configEdge : configLine)
			{
				idsOfConfigEdge = {};
				std::vector<cv::Point> edge;
				for (char i : configEdge)
				{
					// Substract '0' from char to get corresponding digit
					int p = i - '0';
					// Translate config to x,y coordinates of image
					int cx = startPoint.x - 1 + p % 3;
					int cy = startPoint.y - 1 + int(p / 3);
					cv::Point currentPoint = cv::Point(cx, cy);
					std::vector<int> cPid = edgeIdMap.getEdgeIds(currentPoint.x, currentPoint.y);
					if (cPid.size() == 1)
					{
						if (std::find(idsOfConfigEdge.begin(), idsOfConfigEdge.end(), cPid.front()) == idsOfConfigEdge.end())
						{
							idsOfConfigEdge.push_back(edgeIdMap.getEdgeIds(currentPoint.x, currentPoint.y).front());
						}
					}
					if (idsOfConfigEdge.size() == 2)
					{
						// Merge if different information in config
						mergeEdges(idsOfConfigEdge.front(), idsOfConfigEdge.back());
					}
				}
			}
		}
		else if (unvisitedNeighbours.size() == 1) // Follow continuity
		{
			traceEdge(img, unvisitedNeighbours[0], edge);
		}

		else if (unvisitedNeighbours.size() == 0) // Edge finished, check for connections to other edges
		{

			cv::Point addNeighbourEnd;
			std::vector<int> idsEndpoint = edgeIdMap.getEdgeIds(startPoint.x, startPoint.y);
			// Check if end of edge is connected to another edge
			for (auto point : neighbours)
			{
				std::vector<int> idsNeighbour = edgeIdMap.getEdgeIds(point.x, point.y);

				// Check if both points don't have common ids
				if (std::find_first_of(idsEndpoint.begin(), idsEndpoint.end(),
									   idsNeighbour.begin(), idsNeighbour.end()) == idsEndpoint.end())
				{
					edge.push_back(point);
					edgeIdMap.pushBack(point.x, point.y, edgeId);
					addNeighbourEnd = point;
				}
			}

			cv::Point addNeighbourBegin;

			std::vector<int> idsStartpoint = edgeIdMap.getEdgeIds(edge.front().x, edge.front().y);
			std::vector<cv::Point> neighboursOfStart = getDirectNeighbours(img, edge.front());
			// Check if start of edge is connected to another edge
			for (auto point : neighboursOfStart)
			{
				std::vector<int> idsNeighbour = edgeIdMap.getEdgeIds(point.x, point.y);
				if (idsNeighbour.size() > 0)
				{
					// Check if both points don't have common ids
					if (std::find_first_of(idsStartpoint.begin(), idsStartpoint.end(),
										   idsNeighbour.begin(), idsNeighbour.end()) == idsStartpoint.end())
					{
						edge.insert(edge.begin(), point);
						edgeIdMap.pushBack(point.x, point.y, edgeId);
						addNeighbourBegin = point;
					}
				}
			}
			// Finish current edge to be able to process further
			edges.insert(edgeId, edge);
			edgeId++;

			if (addNeighbourEnd.x != 0 && addNeighbourEnd.y != 0)
			{
				std::vector<int> ids = edgeIdMap.getEdgeIds(addNeighbourEnd.x, addNeighbourEnd.y);

				// Two edges can be merged only
				if (ids.size() == 2)
				{
					std::vector<cv::Point> firstEdge = edges.getEdge(ids.front());
					std::vector<cv::Point> secondEdge = edges.getEdge(ids.back());

					// Only merge start- and endpoints
					if ((addNeighbourEnd == firstEdge.front() || addNeighbourEnd == firstEdge.back()) &&
						(addNeighbourEnd == secondEdge.front() || addNeighbourEnd == secondEdge.back()))
					{
						mergeEdges(ids.front(), ids.back());
					}
				}
			}

			if (addNeighbourBegin.x != 0 && addNeighbourBegin.y != 0)
			{
				std::vector<int> ids = edgeIdMap.getEdgeIds(addNeighbourBegin.x, addNeighbourBegin.y);

				// Two edges can be merged only
				if (ids.size() == 2)
				{
					std::vector<cv::Point> firstEdge = edges.getEdge(ids.front());
					std::vector<cv::Point> secondEdge = edges.getEdge(ids.back());

					// Only merge start- and endpoints
					if ((addNeighbourBegin == firstEdge.front() || addNeighbourBegin == firstEdge.back()) &&
						(addNeighbourBegin == secondEdge.front() || addNeighbourBegin == secondEdge.back()))
					{
						mergeEdges(ids.front(), ids.back());
					}
				}
			}
		}
	}
}

std::vector<cv::Point> EdgeProcessor::getDirectNeighbours(cv::Mat &img, cv::Point p)
{
	std::vector<cv::Point> v;

	// Only access neighbours inside the image
	if (p.x - 1 >= 0 && p.y - 1 >= 0) // top left
	{
		// Only add corner pixel as neighbour if there is no shorter connection
		if (img.at<uchar>(p.y - 1, p.x - 1) > 0 && !(img.at<uchar>(p.y - 1, p.x) > 0 || img.at<uchar>(p.y, p.x - 1) > 0))
		{
			v.push_back(cv::Point(p.x - 1, p.y - 1));
		}
	}
	if (p.y - 1 >= 0) // top center
	{
		if (img.at<uchar>(p.y - 1, p.x) > 0)
		{
			v.push_back(cv::Point(p.x, p.y - 1));
		}
	}
	if (p.x + 1 < img.cols && p.y - 1 >= 0) // top right
	{
		if (img.at<uchar>(p.y - 1, p.x + 1) > 0 && !(img.at<uchar>(p.y - 1, p.x) > 0 || img.at<uchar>(p.y, p.x + 1) > 0))
		{
			v.push_back(cv::Point(p.x + 1, p.y - 1));
		}
	}
	if (p.x + 1 < img.cols) // middle right
	{
		if (img.at<uchar>(p.y, p.x + 1) > 0)
		{
			v.push_back(cv::Point(p.x + 1, p.y));
		}
	}
	if (p.x + 1 < img.cols && p.y + 1 < img.rows) // bottom right
	{
		if (img.at<uchar>(p.y + 1, p.x + 1) > 0 && !(img.at<uchar>(p.y, p.x + 1) > 0 || img.at<uchar>(p.y + 1, p.x) > 0))
		{
			v.push_back(cv::Point(p.x + 1, p.y + 1));
		}
	}
	if (p.y + 1 < img.rows) // bottom center
	{
		if (img.at<uchar>(p.y + 1, p.x) > 0)
		{
			v.push_back(cv::Point(p.x, p.y + 1));
		}
	}
	if (p.x - 1 >= 0 && p.y + 1 < img.rows) // bottom left
	{
		if (img.at<uchar>(p.y + 1, p.x - 1) > 0 && !(img.at<uchar>(p.y + 1, p.x) > 0 || img.at<uchar>(p.y, p.x - 1) > 0))
		{
			v.push_back(cv::Point(p.x - 1, p.y + 1));
		}
	}
	if (p.x - 1 >= 0) // middle left
	{
		if (img.at<uchar>(p.y, p.x - 1) > 0)
		{
			v.push_back(cv::Point(p.x - 1, p.y));
		}
	}

	return v;
}

void EdgeProcessor::mergeEdges(int firstId, int secondId)
{
	// firstId has to be the smaller edgeId
	if (secondId < firstId)
	{
		int tempId = secondId;
		secondId = firstId;
		firstId = tempId;
	}

	std::vector<cv::Point> firstEdge = edges.getEdge(firstId);
	std::vector<cv::Point> secondEdge = edges.getEdge(secondId);

	edges.eraseEdge(firstEdge);
	edges.eraseEdge(secondEdge);

	bool lastEdge = true;

	// Check if second edge is last edge in list to adjust ids later
	if (!(secondId == edgeId - 1))
	{
		lastEdge = false;
	}

	// Adjust the edgeIds of both edges to be the same
	for (auto &point : secondEdge)
	{
		std::vector<int> currentIds = edgeIdMap.getEdgeIds(point.x, point.y);
		edgeIdMap.eraseEdgeId(point.x, point.y, secondId);
		if (std::find(currentIds.begin(), currentIds.end(), firstId) == currentIds.end()) // Only add if firstId not already in edge point
		{
			edgeIdMap.pushBack(point.x, point.y, firstId);
		}
		currentIds = edgeIdMap.getEdgeIds(point.x, point.y);
	}

	// Merge edges with same starting point together
	if (firstEdge.front() == secondEdge.front())
	{
		secondEdge.erase(secondEdge.begin());
		// Delete if edges meet at both ends
		if (firstEdge.back() == secondEdge.back())
		{
			secondEdge.pop_back();
		}
		firstEdge.insert(firstEdge.begin(), secondEdge.rbegin(), secondEdge.rend());
	}

	// Merge edges where the start point of the first edge equals the end point of the second edge
	else if (firstEdge.front() == secondEdge.back())
	{
		secondEdge.pop_back();
		// Delete if edges meet at both ends
		if (firstEdge.back() == secondEdge.front())
		{
			firstEdge.pop_back();
		}
		firstEdge.insert(firstEdge.begin(), secondEdge.begin(), secondEdge.end());
	}

	// Merge edge where the end point of the first edge equals the start point of the second edge
	else if (firstEdge.back() == secondEdge.front())
	{
		secondEdge.erase(secondEdge.begin());
		firstEdge.insert(firstEdge.end(), secondEdge.begin(), secondEdge.end());
	}

	// Merge edges with same end point together
	else if (firstEdge.back() == secondEdge.back())
	{
		secondEdge.pop_back();
		firstEdge.insert(firstEdge.end(), secondEdge.rbegin(), secondEdge.rend());
	}

	edges.insert(firstId, firstEdge);
	edgeId--;

	if (!lastEdge) // Change all edgeIds that have been moved
	{
		std::vector<cv::Point> edgeTemp;
		std::vector<int> oldIds;
		for (int i = secondId; i < edges.size(); i++)
		{
			edgeTemp = edges.getEdge(i);
			for (auto &point : edgeTemp)
			{
				edgeIdMap.eraseEdgeId(point.x, point.y, i + 1);
				edgeIdMap.pushBack(point.x, point.y, i);
			}
		}
	}
}

const EdgeIdMap &EdgeProcessor::getEdgeIdMap() const // Return value is read-only
{
	return edgeIdMap;
}

const Edges &EdgeProcessor::getEdges() const // Return value is read-only
{
	return edges;
}

void EdgeProcessor::printEdgeInfos(cv::Mat &img)
{
	std::cout << "Input image: " << img.rows << " rows x " << img.cols << " cols = " << img.total() << " px\n";

	// Count number of edge pixels
	int cnt = 0;
	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			if (img.at<uchar>(y, x) > 0)
			{
				cnt++;
			}
		}
	}

	std::cout << "Edge pixels in input image: " << cnt << " px\n";

	// Infos about traced edges
	std::cout << "Number of traced edges: " << edges.size() << "\n";
}

uint8_t EdgeProcessor::getNeighboursBinary(cv::Mat &img, cv::Point p)
{

	/*
	Returns 3x3 neighours as reversed binary code (0 = no neighbour, 1 = neighbour)
	with point p as input

	7 6 5
	0 p 4
	1 2 3

	*/
	uint8_t bin = 0;

	// Only access neighbours inside the image
	if (p.x - 1 >= 0 && p.y - 1 >= 0) // top left
	{
		if (img.at<uchar>(p.y - 1, p.x - 1) > 0)
		{
			bin = bin + 128;
		}
	}
	if (p.y - 1 >= 0) // top center
	{
		if (img.at<uchar>(p.y - 1, p.x) > 0)
		{
			bin = bin + 64;
		}
	}
	if (p.x + 1 < img.cols && p.y - 1 >= 0) // top right
	{
		if (img.at<uchar>(p.y - 1, p.x + 1) > 0)
		{
			bin = bin + 32;
		}
	}
	if (p.x + 1 < img.cols) // middle right
	{
		if (img.at<uchar>(p.y, p.x + 1) > 0)
		{
			bin = bin + 16;
		}
	}
	if (p.x + 1 < img.cols && p.y + 1 < img.rows) // bottom right
	{
		if (img.at<uchar>(p.y + 1, p.x + 1) > 0)
		{
			bin = bin + 8;
		}
	}
	if (p.y + 1 < img.rows) // bottom center
	{
		if (img.at<uchar>(p.y + 1, p.x) > 0)
		{
			bin = bin + 4;
		}
	}
	if (p.x - 1 >= 0 && p.y + 1 < img.rows) // bottom left
	{
		if (img.at<uchar>(p.y + 1, p.x - 1) > 0)
		{
			bin = bin + 2;
		}
	}
	if (p.x - 1 >= 0) // middle left
	{
		if (img.at<uchar>(p.y, p.x - 1) > 0)
		{
			bin = bin + 1;
		}
	}

	return bin;
}
