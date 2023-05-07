#include "EdgesConnection.h"

// Constructor
EdgesConnection::EdgesConnection()
{
	std::cout << "Object created: EdgesConnection\n";
}

// Main function for edge connection
std::vector<std::vector<cv::Point>> EdgesConnection::edgesConnectionG(cv::Mat &img, Edges &edges, EdgeIdMap &edgeIdMap)
{
	std::vector<std::vector<int>> completeEdgeCombinations; // Each element is a vector of edgeIds
	std::vector<std::vector<cv::Point>> completeEdgeCombinationSequences; // Each element is a vector of pixel coordinates

	// Search for all edge combinations.
	clock_t t1 = clock();
	// Iterative part: start with startEdge, find edge combinations.
	for (int startEdgeId = 0; startEdgeId < edges.size(); startEdgeId++)
	{
		// Only search for non-combined edges as start edge
		if (std::find(combinedEdgeIds.begin(), combinedEdgeIds.end(), startEdgeId) == combinedEdgeIds.end())
		{
			std::vector<std::vector<int>> firstPartEdgeCombinations;
			std::vector<std::vector<int>> secondPartEdgeCombinations;

			// Search for edge combinations from two sides of the start edge
			edgeCombine(startEdgeId, edges.getEdge(startEdgeId)[0], img, edges, edgeIdMap);
			firstPartEdgeCombinations = edgeCombinations;
			edgeCombinations.clear();
			tempEdgeCombination.clear();

			edgeCombine(startEdgeId, edges.getEdge(startEdgeId).back(), img, edges, edgeIdMap);
			secondPartEdgeCombinations = edgeCombinations;
			edgeCombinations.clear();
			tempEdgeCombination.clear();

			/* Check the size of "firstPartEdgeCombinations" and "secondPartEdgeCombinations"
			 * in case they are so large that will consume all the RAMs of the computer.
			 * If this happens, consider filtering the input edge images.
			*/
			if (firstPartEdgeCombinations.size() > 10000 || secondPartEdgeCombinations.size() > 10000)
			{
				std::vector<std::vector<cv::Point>> a;
				std::cout << "firstPartEdgeCombinations or secondPartEdgeCombinations is oversize, considering filtering the input edge images" << std::endl;
				return a;
			}

	//		std::cout << "firstPartEdgeCombinations of startEdgeId =" << startEdgeId <<std::endl;
	//		for (std::size_t i = 0; i < firstPartEdgeCombinations.size(); i++)
	//		{
	//			for (int element : firstPartEdgeCombinations[i])
	//			{
	//				std::cout << element << ",";
	//			}
	//				std::cout << std::endl;
	//		}
	//
	//		std::cout << "secondPartEdgeCombinations of startEdgeId =" << startEdgeId <<std::endl;
	//		for (std::size_t i = 0; i < secondPartEdgeCombinations.size(); i++)
	//		{
	//			for (int element : secondPartEdgeCombinations[i])
	//		    {
	//				std::cout << element << ",";
	//			}
	//				std::cout << std::endl;
	//		}

			// Combine edges in "firstPartEdgeCombinations" and "secondPartEdgeCombinations" because they are traced from the two sides of the same startedge
			std::vector<std::vector<int>> result = CombinePartialEdgeCombinations(firstPartEdgeCombinations, secondPartEdgeCombinations, edges);
			completeEdgeCombinations.reserve(completeEdgeCombinations.size() + result.size());
			completeEdgeCombinations.insert(completeEdgeCombinations.end(), result.begin(), result.end());
		}
		/* Check the size of "completeEdgeCombinations"
		 * in case it is so large that will consume all the RAMs of the computer.
		 * If this happens, consider filtering the input edge images.
		*/
		if (completeEdgeCombinations.size() >= 15000000)
		{
			std::vector<std::vector<cv::Point>> a;
			std::cout << "completeEdgeCombinations is oversize, considering filtering the input edge images" << std::endl;
			return a;
		}
	}
	clock_t t2 = clock();
	std::cout << " The size of found edge combinations " << completeEdgeCombinations.size() << std::endl;
	std::cout << "Finding edge combination took: " << double(t2 - t1) / CLOCKS_PER_SEC << "s" << std::endl;


	// Delete duplicates in "completeEdgeCombinations"
	clock_t t9 = clock();
	std::sort(completeEdgeCombinations.begin(), completeEdgeCombinations.end());
	completeEdgeCombinations.erase(std::unique(completeEdgeCombinations.begin(), completeEdgeCombinations.end()), completeEdgeCombinations.end());
	std::cout << "The size of edge combination after deleting duplicate = " << completeEdgeCombinations.size() << std::endl;
	clock_t t10 = clock();
	std::cout << "Delete duplicates took: " << double(t10 - t9) / CLOCKS_PER_SEC << "s" << std::endl;

//	std::cout << "completeEdgeCombinations" << std::endl;
//	for (std::size_t i = 0; i < completeEdgeCombinations.size(); i++)
//	{
//		for (int element : completeEdgeCombinations[i])
//		{
//			std::cout << element << ",";
//		}
//			std::cout << std::endl;
//	}


	// Compute costs of each combined edge.
	clock_t t3 = clock();
	std::vector<double> costs;
	costs = ComputeCosts(img, completeEdgeCombinations, edges);
	clock_t t4 = clock();
	std::cout << "Calculate costs took: " << double(t4 - t3) / CLOCKS_PER_SEC << "s" << std::endl;
//	std::cout << " completeEdgeCombinations.size = " << completeEdgeCombinations.size() << std::endl;
//	std::cout << " completeEdgeCombinationSequences.size = " << completeEdgeCombinationSequences.size() << std::endl;
//	std::cout << " costs.size = " << costs.size() << std::endl;


	// Sort elements in "completeEdgeCombinations" from smallest costs to largest costs
	clock_t t5 = clock();
	// Quick sort
	Quicksort(costs, completeEdgeCombinations, 0, costs.size() - 1);
	clock_t t6 = clock();
	std::cout << "Sorting took: " << double(t6 - t5) / CLOCKS_PER_SEC << "s" << std::endl;
//	for (std::size_t i = 0; i < costs.size(); i++)
//	{
//
//		std::cout << " The edge = " << std::endl;
//		for (int element : completeEdgeCombinations[i])
//		{
//			std::cout << element << ",";
//		}
//		std::cout << std::endl;
//		std::cout << " cost = " << costs[i] << std::endl;
//	}


	// Delete high cost edges in "completeEdgeCombinations" (high cost edges are not so important,
	// deleting them to reduce the size of "completeEdgeCombinations" for efficiency)
	if (completeEdgeCombinations.size() >= 100000)
	{
		completeEdgeCombinations.erase(completeEdgeCombinations.begin() + 20000, completeEdgeCombinations.end());
	}

	else if (completeEdgeCombinations.size() < 100000 && completeEdgeCombinations.size() > 20000)
	{
		completeEdgeCombinations.erase(completeEdgeCombinations.begin() + (int)(0.2 * completeEdgeCombinations.size()), completeEdgeCombinations.end());
	}


	// Delete combined edges in "completeEdgeCombinations" which contains edgeID that already exist in lower cost edge combinations.
	clock_t t7 = clock();
	std::vector<int> connectedEdgeIds; // Store edgeIDs that are already connected
	SiftEdgeCombinations(completeEdgeCombinations, connectedEdgeIds);
	clock_t t8 = clock();
	std::cout << "Sift edges took: " << double(t8 - t7) / CLOCKS_PER_SEC << "s" << std::endl;


	// Add missed edges (Because some edge combinations in "completeEdgeCombinations" are deleted before
	// add these edges that are not in the remaining edge combinations in "completeEdgeCombinations")
	for (int i = 0; i < edges.size(); i++)
	{
		if (std::find(connectedEdgeIds.begin(), connectedEdgeIds.end(), i) == connectedEdgeIds.end())
		{
			completeEdgeCombinations.push_back(std::vector<int> {i});
		}
	}

	// Compute pixel coordinate sequences of edge combinations in "completeEdgeCombinations"
	completeEdgeCombinationSequences = ComputeEdgePixelSequences(completeEdgeCombinations, edges);


	std::cout << "Number of traced edges after connection: " << completeEdgeCombinations.size() << std::endl;
//	for (std::size_t i = 0; i < completeEdgeCombinations.size(); i++)
//	{
//		std::cout << " The edge = ";
//		for (int element : completeEdgeCombinations[i])
//		{
//			std::cout << element << ",";
//		}
//		std::cout << std::endl;
//
//		std::cout << "The edge pixel = ";
//		for(cv::Point y  : completeEdgeCombinationSequences[i])
//		{
//			std::cout << y.x << ", " << y.y << "|";
//		}
//		std::cout << std::endl;
//	}
	return completeEdgeCombinationSequences;
}

// Recursive part of finding edge combinations starting from the starting edge (targetEdgeId) and starting point (targetPixel)
void EdgesConnection::edgeCombine(int targetEdgeId, cv::Point targetPixel, cv::Mat &img, Edges &edges, EdgeIdMap &edgeIdMap)
{
	//std::cout << "targetEdgeId = " << targetEdgeId << std::endl;
	// Only work with ambiguous point.
	if (edgeIdMap.getCluster(targetPixel.x, targetPixel.y).size() == 1)
	{
		combinedEdgeIds.push_back(targetEdgeId);
		// Find the neighbour pixel of targetPixel in targetEdge for computing gray value of the edge.
		auto it = find(edges.getEdge(targetEdgeId).begin(), edges.getEdge(targetEdgeId).end(), targetPixel);
		std::size_t index = it - edges.getEdge(targetEdgeId).begin();
		cv::Point targetNeighbour;
		if (index == 0)
		{
			targetNeighbour = edges.getEdge(targetEdgeId)[1];
		}

		else if (index == edges.getEdge(targetEdgeId).size()-1)
		{
			targetNeighbour = edges.getEdge(targetEdgeId)[edges.getEdge(targetEdgeId).size()-2];
		}

		// Recursively find out all combinations of edges whose edge intense difference is smaller than a threshold.
		for (int element : edgeIdMap.getEdgeIds(targetPixel.x, targetPixel.y))
		{
			// Only check element different from "targetEdgeId"
			if (element != targetEdgeId)
			{
				// Put the start edgeId into the sequence.
				if (tempEdgeCombination.size() == 0)
				{
					tempEdgeCombination.push_back(targetEdgeId);
				}

				// Find the index of "targetPixel" in "element"
				auto it = find(edges.getEdge(element).begin(), edges.getEdge(element).end(), targetPixel);
				std::size_t index = it - edges.getEdge(element).begin();
				// Find out the other endpoint of this element other than the "targetPixel"
				cv::Point endPoint;
				if (index == 0)
				{
					endPoint = edges.getEdge(element).back();
				}
				else if (index == edges.getEdge(element).size()-1)
				{
					endPoint = edges.getEdge(element)[0];
				}

				// Check if there is a closed edge
				bool closedEdge = false;
				if (tempEdgeCombination.size() > 1)
				{
					for (std::size_t i = 0; i < tempEdgeCombination.size()-1; i++)
					{
						// The element has same endpoint as edges in "tempEdgeCombination", there is a closed edge.
						if (edges.getEdge(tempEdgeCombination[i])[0] == endPoint || edges.getEdge(tempEdgeCombination[i]).back() == endPoint)
						{
							closedEdge = true;
							if (edges.getEdge(tempEdgeCombination[i + 1])[0] == endPoint || edges.getEdge(tempEdgeCombination[i + 1]).back() == endPoint)
							{
								// Save the closed edge in "edgeCombination"
								tempEdgeCombination.push_back(element);
								std::vector<int> temp = tempEdgeCombination;
								temp.push_back(tempEdgeCombination[i + 1]);
								temp.erase(temp.begin(), temp.begin() + i + 1);
								edgeCombinations.push_back(temp);
								tempEdgeCombination.pop_back();
								break;
							}
							else
							{

								tempEdgeCombination.push_back(element);
								std::vector<int> temp = tempEdgeCombination;
								temp.push_back(tempEdgeCombination[i]);
								temp.erase(temp.begin(), temp.begin() + i);
								edgeCombinations.push_back(temp);
								tempEdgeCombination.pop_back();
								break;
							}
						}
					}
				}

				else if (tempEdgeCombination.size() == 1)
				{
					if ((edges.getEdge(tempEdgeCombination[0])[0] == edges.getEdge(element)[0] && edges.getEdge(tempEdgeCombination[0]).back() == edges.getEdge(element).back()) || (edges.getEdge(tempEdgeCombination[0])[0] == edges.getEdge(element).back() && edges.getEdge(tempEdgeCombination[0]).back() == edges.getEdge(element)[0]))
					{
						closedEdge = true;
						tempEdgeCombination.push_back(element);
						edgeCombinations.push_back(tempEdgeCombination);
						tempEdgeCombination.pop_back();
					}
				}

				// Recursively find out all combinations of edges whose edge intensity difference is smaller than a threshold
				if (index == 0 && closedEdge == false && abs(img.at<uchar>(edges.getEdge(element)[1].y, edges.getEdge(element)[1].x) - img.at<uchar>(targetNeighbour.y, targetNeighbour.x)) <= grayValueThreshold)
				{
					tempEdgeCombination.push_back(element);
					edgeCombine(element, edges.getEdge(element).back(), img, edges, edgeIdMap);

				}
				else if (index == edges.getEdge(element).size()-1  && closedEdge == false && abs(img.at<uchar>(edges.getEdge(element)[edges.getEdge(element).size()-2].y, edges.getEdge(element)[edges.getEdge(element).size() - 2].x) - img.at<uchar>(targetNeighbour.y, targetNeighbour.x)) <= grayValueThreshold)
				{
					tempEdgeCombination.push_back(element);
					edgeCombine(element, edges.getEdge(element)[0], img, edges, edgeIdMap);
				}
			}
		}

		if (tempEdgeCombination.size() > 0)
		{
			edgeCombinations.push_back(tempEdgeCombination);
			tempEdgeCombination.pop_back();
		}
	}

	// When the "targetPixel" is not an ambiguous pixel, the recursive part end at this edge. Save the "tempEdgeCombination" in "edgeCombinations"
	else if (edgeIdMap.getCluster(targetPixel.x, targetPixel.y).size() != 1 && tempEdgeCombination.size() > 0)
	{
		edgeCombinations.push_back(tempEdgeCombination);
		tempEdgeCombination.pop_back();
	}
}

// Combine edge combinations in "firstPartEdgeCombinations" and "secondPartEdgeCombinations" because they are traced from the two sides of the same startedge
std::vector<std::vector<int>> EdgesConnection::CombinePartialEdgeCombinations(std::vector<std::vector<int>> firstPartEdgeCombinations, std::vector<std::vector<int>> secondPartEdgeCombinations, Edges edges)
{
	std::vector<std::vector<int>> completeEdgeCombination; // Each element is a vector of edgeIds
	if (firstPartEdgeCombinations.size() > 0 && secondPartEdgeCombinations.size() == 0)
	{
		completeEdgeCombination.insert(completeEdgeCombination.end(), firstPartEdgeCombinations.begin(), firstPartEdgeCombinations.end());
	}

	else if(firstPartEdgeCombinations.size() == 0 && secondPartEdgeCombinations.size() > 0)
	{
		completeEdgeCombination.insert(completeEdgeCombination.end(), secondPartEdgeCombinations.begin(), secondPartEdgeCombinations.end());
	}

	else if (firstPartEdgeCombinations.size() > 0 && secondPartEdgeCombinations.size() > 0)
	{
		// Put all the element in "firstPartEdgeCombinations" and "secondPartEdgeCombinations" into "completeEdgeCombinations".
		completeEdgeCombination.insert(completeEdgeCombination.end(), firstPartEdgeCombinations.begin(), firstPartEdgeCombinations.end());
		completeEdgeCombination.insert(completeEdgeCombination.end(), secondPartEdgeCombinations.begin(), secondPartEdgeCombinations.end());

		// Connect open edges in "firstPartEdgeCombinations" and "secondPartEdgeCombinations".
		for (std::vector<int> firstElement : firstPartEdgeCombinations)
		{
			// Closed edges are put into "completeEdgeCombinations". "firstElement.size() > 1" to prevent one edge element.
			if (firstElement[0] == firstElement[firstElement.size()-1] && firstElement.size() > 1)
			{
				completeEdgeCombination.push_back(firstElement);
				continue;
			}

			for (std::vector<int> secondElement : secondPartEdgeCombinations)
			{
				// Closed edges are put into "completeEdgeCombinations".
				if (secondElement[0] == secondElement[secondElement.size()-1] && secondElement.size() > 1)
				{
					completeEdgeCombination.push_back(secondElement);
					continue;
				}

				// When the two parts have size greater than 1, connect them.
				else if (firstElement.size() > 1 && secondElement.size() > 1)
				{
					// Check if there is closed edges when connect firstElement and secondElement.
					bool closedEdge = false;
					for (std::size_t i = 1; i < firstElement.size(); i++)
					{
						for (std::size_t j = 1; j < secondElement.size(); j++)
						{
							// If there are common endpoints of the two edges, there are closed edge.

							if (edges.getEdge(firstElement[i])[0] == edges.getEdge(secondElement[j])[0] || edges.getEdge(firstElement[i])[0] == edges.getEdge(secondElement[j]).back() || edges.getEdge(firstElement[i]).back() == edges.getEdge(secondElement[j])[0] || edges.getEdge(firstElement[i]).back() == edges.getEdge(secondElement[j]).back())
							{
								closedEdge = true;
								break;
							}
						}

						if (closedEdge == true)
						{
							break;
						}
					}
					// If no closed edge after connecting firstElement and secondElement, then connect them.
					if (closedEdge == false)
					{
						// Delete the first element of "firstElement" which is actually the "startEdgeId". Because it is also in the "secondElement".
						std::vector<int> temp = firstElement;
						temp.erase(temp.begin());
						std::reverse(secondElement.begin(), secondElement.end());
						std::vector<int> combinedElement = secondElement;
						combinedElement.insert(combinedElement.end(), temp.begin(), temp.end());
						completeEdgeCombination.push_back(combinedElement);
					}
				}
			}
		}
	}
	return completeEdgeCombination;
}

// Delete combined edges in "completeEdgeCombinations" which contains edgeID that already exist in lower cost edge combinations.
void EdgesConnection::SiftEdgeCombinations(std::vector<std::vector<int>> &completeEdgeCombinations, std::vector<int> &connectedEdgeIds)
{

	for (std::size_t i = 0; i < completeEdgeCombinations.size(); i++)
	{
		// Check if there are edges in this element that are already connected
		bool isConnectedEdges = false;
		for (std::size_t j = 0; j < completeEdgeCombinations[i].size(); j++)
		{
			if (std::find(connectedEdgeIds.begin(), connectedEdgeIds.end(), completeEdgeCombinations[i][j]) != connectedEdgeIds.end())
			{
				isConnectedEdges = true;
				break;
			}
		}

		// If the edge element in this edge combination is already connected, delete this edge combination
		if (isConnectedEdges == true)
		{
			completeEdgeCombinations.erase(completeEdgeCombinations.begin() + i);
			i = i - 1;
		}

		// If no edge element in this edge combination is connected, keep this edge combination and put all edge element in this combination into "connectedEdgeIds"
		if (isConnectedEdges == false)
		{
			for (std::size_t k = 0; k < completeEdgeCombinations[i].size(); k++)
			{
				connectedEdgeIds.push_back(completeEdgeCombinations[i][k]);
			}
		}
	}
}

// Compute pixel coordinate sequences of edge combinations in "completeEdgeCombinations"
std::vector<std::vector<cv::Point>> EdgesConnection::ComputeEdgePixelSequences(std::vector<std::vector<int>> completeEdgeCombinations, Edges edges)
{
	std::vector<std::vector<cv::Point>> completeEdgeCombinationSequences;
	for (std::vector<int> edgeCombination : completeEdgeCombinations)
		{
			std::vector<cv::Point> tempEdgeCombinationSequence;
			 // Single edge element
			if (edgeCombination.size() == 1)
			{
				completeEdgeCombinationSequences.push_back(edges.getEdge(edgeCombination[0]));
			}

			// Closed edge contains two edge segments
			else if (edgeCombination.front() == edgeCombination.back() && edgeCombination.size() == 3)
			{
				auto it = find(edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end(), edges.getEdge(edgeCombination[0])[0]);
				std::size_t index = it - edges.getEdge(edgeCombination[1]).begin();
				if (index == 0)
				{
					tempEdgeCombinationSequence = edges.getEdge(edgeCombination[0]);
					std::reverse(tempEdgeCombinationSequence.begin(), tempEdgeCombinationSequence.end());
					tempEdgeCombinationSequence.pop_back();
					tempEdgeCombinationSequence.erase(tempEdgeCombinationSequence.begin());
					tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).rbegin(), edges.getEdge(edgeCombination[1]).rend());
					completeEdgeCombinationSequences.push_back(tempEdgeCombinationSequence);
				}

				else if (index == edges.getEdge(edgeCombination[1]).size() - 1)
				{
					tempEdgeCombinationSequence = edges.getEdge(edgeCombination[0]);
					tempEdgeCombinationSequence.pop_back();
					tempEdgeCombinationSequence.erase(tempEdgeCombinationSequence.begin());
					tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end());
					completeEdgeCombinationSequences.push_back(tempEdgeCombinationSequence);
				}
			}

			// Other edge combinations
			else
			{
				for (std::size_t i = 0; i < edgeCombination.size() - 1; i++)
				{
					if (i == 0)
					{
						tempEdgeCombinationSequence = edges.getEdge(edgeCombination[0]);
					}

					else if (i == 1)
					{
						if (edges.getEdge(edgeCombination[0])[0] == edges.getEdge(edgeCombination[1])[0])
						{
							std::reverse(tempEdgeCombinationSequence.begin(), tempEdgeCombinationSequence.end());
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end());
						}

						if (edges.getEdge(edgeCombination[0])[0] == edges.getEdge(edgeCombination[1]).back())
						{
							std::reverse(tempEdgeCombinationSequence.begin(), tempEdgeCombinationSequence.end());
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).rbegin(), edges.getEdge(edgeCombination[1]).rend());
						}

						if (edges.getEdge(edgeCombination[0]).back() == edges.getEdge(edgeCombination[1])[0])
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end());
						}

						if (edges.getEdge(edgeCombination[0]).back() == edges.getEdge(edgeCombination[1]).back())
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[1]).rbegin(), edges.getEdge(edgeCombination[1]).rend());
						}
					}

					else if (i > 1)
					{
						if (tempEdgeCombinationSequence.back() == edges.getEdge(edgeCombination[i])[0])
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i]).begin(), edges.getEdge(edgeCombination[i]).end());
						}

						else if (tempEdgeCombinationSequence.back() == edges.getEdge(edgeCombination[i]).back())
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i]).rbegin(), edges.getEdge(edgeCombination[i]).rend());
						}
					}

					// For an unclosed edge, connect the last edge element
					if (edgeCombination[0] != edgeCombination.back() && i == edgeCombination.size() - 2)
					{
						if (tempEdgeCombinationSequence.back() == edges.getEdge(edgeCombination[i + 1])[0])
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i + 1]).begin(), edges.getEdge(edgeCombination[i + 1]).end());
						}

						else if (tempEdgeCombinationSequence.back() == edges.getEdge(edgeCombination[i + 1]).back())
						{
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i + 1]).rbegin(), edges.getEdge(edgeCombination[i + 1]).rend());
						}

						else if (tempEdgeCombinationSequence[0] == edges.getEdge(edgeCombination[i + 1])[0])
						{
							std::reverse(tempEdgeCombinationSequence.begin(), tempEdgeCombinationSequence.end());
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i + 1]).begin(), edges.getEdge(edgeCombination[i + 1]).end());
						}

						else if (tempEdgeCombinationSequence[0] == edges.getEdge(edgeCombination[i + 1]).back())
						{
							std::reverse(tempEdgeCombinationSequence.begin(), tempEdgeCombinationSequence.end());
							tempEdgeCombinationSequence.pop_back();
							tempEdgeCombinationSequence.insert(tempEdgeCombinationSequence.end(), edges.getEdge(edgeCombination[i + 1]).rbegin(), edges.getEdge(edgeCombination[i + 1]).rend());
						}
						completeEdgeCombinationSequences.push_back(tempEdgeCombinationSequence);
					}

					// For an closed edge, don't connect the last edge element
					else if (edgeCombination[0] == edgeCombination.back() && i == edgeCombination.size() - 2)
					{
						tempEdgeCombinationSequence.pop_back();
						completeEdgeCombinationSequences.push_back(tempEdgeCombinationSequence);
					}
				}
			}
		}
	return completeEdgeCombinationSequences;
}

// Part of the Quicksort function
int EdgesConnection::Partition(std::vector<double> &costs, std::vector<std::vector<int>> &completeEdgeCombinations, int start, int end)
{
	int pivot = end;
	int j = start;
	for(int i = start; i<end; ++i)
	{
		if (costs[i] < costs[pivot])
		{
			double costTemp = costs[j];
			std::vector<int> edgeTemp = completeEdgeCombinations[j];
			costs[j] = costs[i];
			costs[i] = costTemp;
			completeEdgeCombinations[j] = completeEdgeCombinations[i];
			completeEdgeCombinations[i] = edgeTemp;
			++j;
		}
	}
	double costTemp = costs[j];
	std::vector<int> edgeTemp = completeEdgeCombinations[j];
	costs[j] = costs[pivot];
	costs[pivot] = costTemp;
	completeEdgeCombinations[j] = completeEdgeCombinations[pivot];
	completeEdgeCombinations[pivot] = edgeTemp;
	return j;
}

void EdgesConnection::Quicksort(std::vector<double> &costs, std::vector<std::vector<int>> &completeEdgeCombinations, int start, int end)
{
	if (start < end)
	{
		int p = Partition(costs, completeEdgeCombinations, start, end);
		Quicksort(costs, completeEdgeCombinations, start, p-1);
		Quicksort(costs, completeEdgeCombinations, p+1, end);
	}
}

std::vector<double> EdgesConnection::ComputeCosts(cv::Mat &img, std::vector<std::vector<int>> completeEdgeCombinations, Edges edges)
{
	std::vector<double> costs;
	for (std::vector<int> edgeCombination : completeEdgeCombinations)
		{
			double lengthCost = 0;
			int totalLength = 0;
			double directionCost = 0;
			double grayValueCost = 0;
			double closedEdgeCost = 0;

			if (edgeCombination.size() == 1)
			{
				lengthCost = L * edges.getEdge(edgeCombination[0]).size();
				directionCost = 0;
				grayValueCost = 0;
				closedEdgeCost = 0;
				costs.push_back(lengthCost + directionCost + grayValueCost + closedEdgeCost);

	//			std::cout << "The edge = " <<  std::endl;
	//			for(int x : edgeCombination)
	//			{
	//				std::cout << x << ", ";
	//			}
	//			std::cout << std::endl;
	//			std::cout << "lengthCost = " << lengthCost << ", " << "directionCost = " << directionCost << ", " << "grayValueCost = " << grayValueCost << ", " << "closedEdgeCost = " << closedEdgeCost << std::endl;
	//			std::cout << "cost = " << lengthCost + directionCost + grayValueCost + closedEdgeCost << std::endl;
	//			std::cout << "The edge pixel = " <<  std::endl;
	//			for(cv::Point y  : edges.getEdge(edgeCombination[0]))
	//			{
	//				std::cout << y.x << ", " << y.y << ";;";
	//			}
	//			std::cout << std::endl;
			}

			else if (edgeCombination.size() > 1)
			{
	//			std::cout << "The edge = " <<  std::endl;
	//			for(int x : edgeCombination)
	//			{
	//				std::cout << x << ", ";
	//			}
	//			std::cout << std::endl;

				for (std::size_t i = 0; i < edgeCombination.size() - 1; i++)
				{
					cv::Point ambPoint; // The ambiguous point between two edges

					// Compute the total length of the edge
					totalLength = totalLength + edges.getEdge(edgeCombination[i]).size();

					// A closed edge
					if (edgeCombination[0] == edgeCombination.back())
					{
						//Find ambiguous point, compute gray value cost and compute combined edge sequences of closed contour which consists of two edge segments.
						if (edgeCombination.size() == 3 && i == 0)
						{
							// Find ambiguous point
							ambPoint = edges.getEdge(edgeCombination[0])[0];
							auto it = find(edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end(), ambPoint);
							std::size_t index = it - edges.getEdge(edgeCombination[1]).begin();
							if (index == 0)
							{
								grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[0])[1].y, edges.getEdge(edgeCombination[0])[1].x) - img.at<uchar>(edges.getEdge(edgeCombination[1])[1].y, edges.getEdge(edgeCombination[1])[1].x));
	//							std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[0])[1].x << " , " << edges.getEdge(edgeCombination[0])[1].y << std::endl;
	//							std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[i + 1])[1].x << " , " << edges.getEdge(edgeCombination[i + 1])[1].y << std::endl;
							}

							else if (index == edges.getEdge(edgeCombination[1]).size() - 1)
							{
								grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[0])[1].y, edges.getEdge(edgeCombination[0])[1].x) - img.at<uchar>(edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].y, edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].x));
	//							std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[0])[1].x << " , " << edges.getEdge(edgeCombination[0])[1].y << std::endl;
	//							std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].x << " , " << edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].y << std::endl;
							}
						}

						else if (edgeCombination.size() == 3 && i == 1)
						{
							ambPoint = edges.getEdge(edgeCombination[0]).back();
							auto it = find(edges.getEdge(edgeCombination[1]).begin(), edges.getEdge(edgeCombination[1]).end(), ambPoint);
							std::size_t index = it - edges.getEdge(edgeCombination[1]).begin();
							if (index == 0)
							{
								grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[0])[edges.getEdge(edgeCombination[0]).size() - 2].y, edges.getEdge(edgeCombination[0])[edges.getEdge(edgeCombination[0]).size() - 2].x) - img.at<uchar>(edges.getEdge(edgeCombination[1])[1].y, edges.getEdge(edgeCombination[1])[1].x));

							}

							else if (index == edges.getEdge(edgeCombination[1]).size() - 1)
							{
								grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[0])[edges.getEdge(edgeCombination[0]).size() - 2].y, edges.getEdge(edgeCombination[0])[edges.getEdge(edgeCombination[0]).size() - 2].x) - img.at<uchar>(edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].y, edges.getEdge(edgeCombination[1])[edges.getEdge(edgeCombination[1]).size() - 2].x));
							}
						}
					}

					// Find ambiguous point, compute gray value cost and compute combined edge sequences of edge combinations which are not a "closed contour with two edge segments".
					if (!(edgeCombination[0] == edgeCombination.back() && edgeCombination.size() == 3))
					{
						// Compute gray value costs and ambPoint.
						if (edges.getEdge(edgeCombination[i])[0] == edges.getEdge(edgeCombination[i + 1])[0])
						{
							grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[i])[1].y, edges.getEdge(edgeCombination[i])[1].x) - img.at<uchar>(edges.getEdge(edgeCombination[i + 1])[1].y, edges.getEdge(edgeCombination[i + 1])[1].x));
							ambPoint = edges.getEdge(edgeCombination[i])[0];
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[i])[1].x << " , " << edges.getEdge(edgeCombination[i])[1].y << std::endl;
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[i + 1])[1].x << " , " << edges.getEdge(edgeCombination[i + 1])[1].y << std::endl;
	//						int a = img.at<uchar>(edges.getEdge(edgeCombination[i])[1].y, edges.getEdge(edgeCombination[i])[1].x);
	//						int b = img.at<uchar>(edges.getEdge(edgeCombination[i+1])[1].y, edges.getEdge(edgeCombination[i+1])[1].x);
	//						std::cout << "gray value of coordinate of edgeId = " << edgeCombination[i] << " = " << a << std::endl;
	//						std::cout << "gray value of coordinate of edgeId = " << edgeCombination[i + 1] << " = " << b << std::endl;
						}

						if (edges.getEdge(edgeCombination[i])[0] == edges.getEdge(edgeCombination[i + 1]).back())
						{
							grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[i])[1].y, edges.getEdge(edgeCombination[i])[1].x) - img.at<uchar>(edges.getEdge(edgeCombination[i + 1])[edges.getEdge(edgeCombination[i + 1]).size()-2].y, edges.getEdge(edgeCombination[i + 1])[edges.getEdge(edgeCombination[i + 1]).size()-2].x));
							ambPoint = edges.getEdge(edgeCombination[i])[0];
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[i])[1].x << " , " << edges.getEdge(edgeCombination[i])[1].y << std::endl;
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[i+1])[edges.getEdge(edgeCombination[i+1]).size() - 2].x << " , " << edges.getEdge(edgeCombination[i+1])[edges.getEdge(edgeCombination[i+1]).size() - 2].y << std::endl;
						}

						if (edges.getEdge(edgeCombination[i]).back() == edges.getEdge(edgeCombination[i + 1])[0])
						{
							grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[i + 1])[1].y, edges.getEdge(edgeCombination[i + 1])[1].x) - img.at<uchar>(edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size()-2].y, edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size()-2].x));
							ambPoint = edges.getEdge(edgeCombination[i]).back();
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size() - 2].x << " , " << edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size() - 2].y << std::endl;
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[i + 1])[1].x << " , " << edges.getEdge(edgeCombination[i + 1])[1].y << std::endl;
						}

						if (edges.getEdge(edgeCombination[i]).back() == edges.getEdge(edgeCombination[i+1]).back())
						{
							grayValueCost = grayValueCost + G * abs(img.at<uchar>(edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size()-2].y, edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size()-2].x) - img.at<uchar>(edges.getEdge(edgeCombination[i + 1])[edges.getEdge(edgeCombination[i + 1]).size()-2].y, edges.getEdge(edgeCombination[i + 1])[edges.getEdge(edgeCombination[i + 1]).size()-2].x));
							ambPoint = edges.getEdge(edgeCombination[i]).back();
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i] << " = " << edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size() - 2].x << " , " << edges.getEdge(edgeCombination[i])[edges.getEdge(edgeCombination[i]).size() - 2].y << std::endl;
	//						std::cout << "gray value coordinate of EdgeId = " << edgeCombination[i + 1] << " = " << edges.getEdge(edgeCombination[i+1])[edges.getEdge(edgeCombination[i+1]).size() - 2].x << " , " << edges.getEdge(edgeCombination[i+1])[edges.getEdge(edgeCombination[i+1]).size() - 2].y << std::endl;
						}
					}

					// Compute direction cost
					// Calculate edge segments of given length around the ambiguous point
					std::vector<std::vector<cv::Point>> edgeSegments; // Edge segments of certain length initiate with the ambiguous pixel
					for (std::size_t j = i; j <= i + 1; j++)
					{
						std::vector<cv::Point> edge = edges.getEdge(edgeCombination[j]);
						auto it = find(edge.begin(), edge.end(), ambPoint);
						std::size_t index = it - edge.begin();

						if (length <= edge.size())
						{
							if (index == 0)
							{
								edge.erase(edge.begin()+length, edge.end());
								edgeSegments.push_back(edge);
							}
							else if (index == edge.size() - 1)
							{
								std::reverse(edge.begin(), edge.end());
								edge.erase(edge.begin()+length, edge.end());
								edgeSegments.push_back(edge);
							}
						}

						else
						{
							if (index == 0)
							{
								edgeSegments.push_back(edge);
							}
							else if (index == edge.size() - 1)
							{
								std::reverse(edge.begin(), edge.end());
								edgeSegments.push_back(edge);
							}
						}
					}
	//				std::cout << "edgeSegments.size of EdgeId = " << edgeCombination[i] << " = " <<edgeSegments[0].size() << std::endl;
	//				std::cout << "edgeSegments.size of EdgeId = " << edgeCombination[i + 1] << " = " <<edgeSegments[1].size() << std::endl;

					// Calculate the linear estimation of edge segments with least squares
					std::vector<std::vector<double>> linearFunction; //  Linear estimation of each edge segment: a2 * y + a1 * x + a0 = 0  parameters are stored in the order:(a2, a1, a0)
					for (std::vector<cv::Point> elements : edgeSegments)
					{
						double X2 = 0; // (x0^2 + x1^2 + ... + xn^2)
						double X  = 0; // (x0 + x1 + ... + xn)
						double XY = 0; // (x0*y0 + x1*y1 + ... + xn*yn)
						double Y  = 0; // (y0 + y1 + ... +yn)
						std::vector<double> A; //(a2, a1, a0)   a2 * y + a1 * x + a0 = 0

						// The coordinates of the same pixel in images (xi, yi) and in opencv (xo, yo) have the relationship : xi = yo, yi = xo. Here the image coordinate is taken as a reference.
						for (cv::Point element : elements)
						{
							X2 = X2 + pow(element.y, 2);
							X  = X + element.y;
							XY = XY + element.x * element.y;
							Y  = Y + element.x;
						}

						// Linear function is vertical
						if ((X2 - X * X / elements.size()) == 0)
						{
							A.push_back(0); // a2 = 0
							A.push_back(elements.size());
							A.push_back(-X);
						}

						// Linear function is not vertical
						else
						{
							A.push_back(-1); // for simplicity set a2 = -1
							A.push_back((XY - Y * X / elements.size()) / (X2 - X * X / elements.size()));
							A.push_back(ambPoint.x - A[1] * ambPoint.y);
						}

						linearFunction.push_back(A);
					}

	//				std::cout << "linearFunction of EdgeId = " << edgeCombination[i] << " = " << linearFunction[0][0] << "," << linearFunction[0][1] << "," << linearFunction[0][2] << std::endl;
	//				std::cout << "linearFunction of EdgeId = " << edgeCombination[i + 1] << " = " << linearFunction[1][0] << "," << linearFunction[1][1] << "," << linearFunction[1][2] << std::endl;

					// Compute reference pixel coordinate of each function
					std::vector<cv::Point> refCoordinate; // Reference pixel coordinate of each linearFunction for computing the cosine
					for (std::size_t i = 0; i < edgeSegments.size(); i++)
					{
						// For vertical functions (a2 = 0), x = -a0 / a1; y = the y coordinate of the last pixel in the edge segment
						if (linearFunction[i][0] == 0)
						{
							refCoordinate.push_back(cv::Point((-linearFunction[i][2] / linearFunction[i][1]), edgeSegments[i].back().x));
						}
						// For non-vertical functions (a2 != 0),
						else
						{
							// If the y coordinate range is bigger than x, x = the x coordinate of the last pixel in the edge segment
							// y = (-a0 - a1 * x) / a2
							if (abs(edgeSegments[i].back().y - edgeSegments[i].front().y) > abs(edgeSegments[i].back().x - edgeSegments[i].front().x))
							{
								refCoordinate.push_back(cv::Point(edgeSegments[i].back().y, (-linearFunction[i][2] - linearFunction[i][1] * edgeSegments[i].back().y) / linearFunction[i][0]));
							}
							// If the x coordinate range is bigger than y, y = the y coordinate of the last pixel in the edge segment
							// x = (-a0 - a2 * y) / a1
							else
							{
								refCoordinate.push_back(cv::Point((-linearFunction[i][2] - linearFunction[i][0] * edgeSegments[i].back().x) / linearFunction[i][1], edgeSegments[i].back().x));
							}
						}
					}

					double cos = 0;
					// Compute the cosine value of each pair according to "refCoordinate" of each function
					double a2 = pow(refCoordinate[0].x - refCoordinate[1].x, 2) + pow(refCoordinate[0].y - refCoordinate[1].y, 2);
					double b2 = pow(refCoordinate[0].x - ambPoint.y, 2) + pow(refCoordinate[0].y - ambPoint.x, 2);
					double c2 = pow(refCoordinate[1].x - ambPoint.y, 2) + pow(refCoordinate[1].y - ambPoint.x, 2);
					cos = ((b2 + c2 - a2) / (2 * sqrt(b2 * c2)));
					directionCost = directionCost + D * cos;
				}

				// Compute the length cost and closedEdgeCost
				// Not a closed edge (First edge Id and the last edge Id are not the same)
				if (edgeCombination[0] != edgeCombination.back())
				{
					totalLength = totalLength + edges.getEdge(edgeCombination.back()).size();
				}
				// A closed edge
				else if (edgeCombination[0] == edgeCombination.back())
				{
					closedEdgeCost = C;
				}

				lengthCost = (totalLength - edgeCombination.size() + 1) * L;
				costs.push_back(lengthCost + directionCost + grayValueCost + closedEdgeCost);
	//			std::cout << "lengthCost = " << lengthCost << ", " << "directionCost = " << directionCost << ", " << "grayValueCost = " << grayValueCost << ", " << "closedEdgeCost = " << closedEdgeCost << std::endl;
	//			std::cout << "cost = " << lengthCost + directionCost + grayValueCost + closedEdgeCost << std::endl;

	//			std::cout << "The edge pixel = " <<  std::endl;
	//			for(cv::Point y  : tempEdgeCombinationSequence)
	//			{
	//				std::cout << y.x << ", " << y.y << ";;";
	//			}
	//			std::cout << std::endl;
			}
		}
	return costs;
}
