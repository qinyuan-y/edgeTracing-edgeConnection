#include "Edges.h"

// General includes
#include <iostream>

Edges::Edges()
{
	std::cout << "Object created: Edges\n";
}

void Edges::clear()
{
	data.clear();
}

void Edges::pushBack(std::vector<cv::Point> edge)
{
	data.push_back(edge);
}

void Edges::insert(int position, std::vector<cv::Point> edge)
{
	data.insert(data.begin() + position, edge);
}

void Edges::popBack()
{
	data.pop_back();
}

int Edges::size() const
{
	return data.size();
}

void Edges::eraseEdge(std::vector<cv::Point> edge)
{
	data.erase(std::find(data.begin(), data.end(), edge));
}

const std::vector<cv::Point>& Edges::getEdge (int index) const
{

	return data[index];
}

const std::vector<std::vector<cv::Point>> &Edges::getEdges() const
{
	return data;
}
