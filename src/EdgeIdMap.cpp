#include "EdgeIdMap.h"

EdgeIdMap::EdgeIdMap()
{
	std::cout << "Object created: EdgeIdMap\n";
}

void EdgeIdMap::init(int rows, int cols)
{
	// Reset
	data.clear();
	dataCluster.clear();

	// Save number of cols to convert between 1D and 2D
	EdgeIdMap::cols = cols;
	EdgeIdMap::rows = rows;

	// Initialize data structure
	data.resize(rows * cols);
	dataCluster.resize(rows * cols);
}

int EdgeIdMap::getSize(int x, int y) const
{
	// Number of indices at given position
	return data[x + y * cols].size();
}

const std::vector<int> &EdgeIdMap::getEdgeIds(int x, int y) const
{
	return data[x + y * cols];
}

const std::vector<cv::Point> &EdgeIdMap::getCluster(int x, int y) const
{
	return dataCluster[x + y * cols];
}

int EdgeIdMap::getCols() const
{
	return cols;
}

int EdgeIdMap::getRows() const
{
	return rows;
}

int EdgeIdMap::getMaxEdgeId() const
{
	int maxId = 0;

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			for (int i = 0; i < EdgeIdMap::getSize(x, y); i++)
			{
				if (data[x + y * cols][i] > maxId)
				{
					maxId = data[x + y * cols][i];
				}
			}
		}
	}

	return maxId;
}

void EdgeIdMap::pushBack(int x, int y, int edgeId)
{
	if (dataCluster[x + y * cols].size() == 0)
	{
		// Push back edge index at given position
		data[x + y * cols].push_back(edgeId);
	}
	else
	{
		// Only Push Back edgeId if not already in cluster
		if (std::find(data[x + y * cols].begin(), data[x + y * cols].end(), edgeId) == data[x + y * cols].end())
		{
			for (auto p : dataCluster[x + y * cols])
			{
				data[p.x + p.y * cols].push_back(edgeId);
			}
		}
	}
}

void EdgeIdMap::pushBackCluster(int x, int y, std::vector<cv::Point> cluster)
{
	// Push back edge index at given position
	dataCluster[x + y * cols] = cluster;
}

void EdgeIdMap::eraseEdgeId(int x, int y, int edgeId)
{
	if (dataCluster[x + y * cols].size() == 0)
	{
		if (std::find(data[x + y * cols].begin(), data[x + y * cols].end(), edgeId) != data[x + y * cols].end())
		{
			data[x + y * cols].erase(std::find(data[x + y * cols].begin(), data[x + y * cols].end(), edgeId));
		}
	}
	else
	{
		for (auto p : dataCluster[x + y * cols])
		{
			if (std::find(data[p.x + p.y * cols].begin(), data[p.x + p.y * cols].end(), edgeId) != data[p.x + p.y * cols].end())
			{
				data[p.x + p.y * cols].erase(std::find(data[p.x + p.y * cols].begin(), data[p.x + p.y * cols].end(), edgeId));
			}
		}
	}
}