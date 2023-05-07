#include "Visualizer.h"

namespace
{
	/** Generate one color for each edge.
	 *  @numberOfValues Number of RGB values to generate.
	 */
	std::vector<cv::Scalar> generateRgbValues(int numberOfValues)
	{
		cv::RNG rng(31231); // OpenCV random number generator

		std::vector<cv::Scalar> rgbValues;
		rgbValues.reserve(numberOfValues);

		for (int i = 0; i < numberOfValues; i++)
		{
			rgbValues.emplace_back(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			// std::cout << rgbValues.at(i)->val[0] << std::endl;
		}

		// std::cout << "Generated " << rgbValues.size() << " random RGB values." << std::endl;
		return rgbValues;
	}

} // end namespace

void Visualizer::saveEdgesAsSVG(cv::Mat &img, const Edges &edges, const EdgeIdMap &edgeIdMap, bool markStartEnd, bool showInput)
{
	// Generate a color for each edge
	std::vector<cv::Scalar> rgbValues = generateRgbValues(edges.size());

	// Open new SVG file
	FILE *file;
	file = fopen("./tracedEdges.svg", "w");

	// Setup SVG canvas
	fprintf(file, "<svg width=\"%d\" height=\"%d\">\n", img.cols, img.rows);
	fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"black\" />\n");

	// Data structure to mark ambiguous points
	std::vector<std::pair<cv::Point, std::vector<int>>> ambiguousPoints;
	cv::Mat sharedPixels(img.size(), CV_8UC1, 0.0);

	// Draw pixels of input image
	if (showInput)
	{
		for (int y = 0; y < img.rows; y++)
		{
			for (int x = 0; x < img.cols; x++)
			{
				if (img.at<uchar>(y, x) > 0)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" fill=\"grey\" />\n", x, y);
				}
			}
		}
	}

	// Draw pixels of each edge (i = edgeId)
	const std::vector<std::vector<cv::Point>> &edgesData = edges.getEdges();

	for (auto &edge : edgesData)
	{
		for (auto &point : edge)
		{
			sharedPixels.at<uchar>(point)++;
		}
	}

	for (int y = 0; y < img.rows; y++)
	{
		for (int x = 0; x < img.cols; x++)
		{
			auto cluster = edgeIdMap.getCluster(x, y);
			if (sharedPixels.at<uchar>(y, x) > 1.0 || cluster.size() > 0)
			{
				std::pair<cv::Point, std::vector<int>> tmpPair;
				tmpPair.first = cv::Point(x, y);
				ambiguousPoints.push_back(tmpPair);
			}
		}
	}

	for (int i = 0; i < (int)edgesData.size(); i++)
	{
		for (int j = 0; j < (int)edgesData[i].size(); j++)
		{
			for (int k = 0; k < (int)ambiguousPoints.size(); k++)
			{
				if (ambiguousPoints[k].first == edgesData[i][j])
				{
					ambiguousPoints[k].second.push_back(i);
				}
			}
		}
	}

	for (int i = 0; i < (int)edgesData.size(); i++)
	{
		for (int j = 0; j < (int)edgesData[i].size(); j++)
		{
			// Position
			int x = edgesData[i][j].x;
			int y = edgesData[i][j].y;
			// Only print usual pixel here
			if (!(sharedPixels.at<uchar>(y, x) > 1.0))
			{

				// Color
				int r = (int)rgbValues.at(i).val[0];
				int g = (int)rgbValues.at(i).val[1];
				int b = (int)rgbValues.at(i).val[2];

				fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y, r, g, b);

				if (markStartEnd)
				{
					// Mark startpoint
					if (j == 0)
					{
						fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.25\" stroke=\"grey\" stroke-width=\"0.1\" fill=\"none\" />\n", x + 0.5, y + 0.5);
					}

					// Mark endpoint
					if (j == (int)edgesData[i].size() - 1)
					{
						fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.1\" fill=\"grey\" />\n", x + 0.5, y + 0.5);
					}
				}

				// Print edge number
				if (true)
				{
					fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.15, i, j);
				}

				// Print coordinates
				if (true)
				{
					fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.95, x, y);
				}
			}
		}
	}

	// Go through all ambiguous points and set colors
	for (auto &point : ambiguousPoints)
	{
		// coordinates
		int x = point.first.x;
		int y = point.first.y;
		for (int i = 0; i < (int)point.second.size(); i++)
		{
			double scale = (double)1 / point.second.size();

			// Color
			int j = (int)point.second[i];
			int r = (int)rgbValues.at(j).val[0];
			int g = (int)rgbValues.at(j).val[1];
			int b = (int)rgbValues.at(j).val[2];

			fprintf(file, "<rect x=\"%d\" y=\"%f\" width=\"1\" height=\"%f\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y + scale * i, scale, r, g, b);

			// Print edge number
			auto tempEdge = edgesData[j];
			int index = std::find(tempEdge.begin(), tempEdge.end(), cv::Point(x, y)) - tempEdge.begin();
			fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.15 + scale * i, j, index);

			// Mark startpoint
			if (index == 0)
			{
				fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.15\" stroke=\"grey\" stroke-width=\"0.1\" fill=\"none\" />\n", x + 0.5, y + 0.5 * scale + scale * i);
			}

			// Mark endpoint
			if (index == (int)tempEdge.size() - 1)
			{
				fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"0.1\" fill=\"grey\" />\n", x + 0.5, y + 0.5 * scale + scale * i);
			}
		}

		auto cluster = edgeIdMap.getCluster(x, y);
		if (cluster.size() == 1)
		{
			fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", x, y, 255, 255, 255);
		}
		else
		{
			for (auto p : cluster)
			{
				fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", p.x, p.y, 255, 0, 0);
			}
		}
	}

	// End and close SVG file
	fprintf(file, "</svg>");
	fclose(file);

	std::cout << "File tracedEdges.svg written.\n";
}

void Visualizer::saveEdgeIdMapAsSVG(const EdgeIdMap &edgeIdMap)
{
	int rows = edgeIdMap.getRows();
	int cols = edgeIdMap.getCols();

	// Generate a color for each edge
	std::vector<cv::Scalar> rgbValues = generateRgbValues(edgeIdMap.getMaxEdgeId() + 1);

	// Open new SVG file
	FILE *file;
	file = fopen("./edgeIdMap.svg", "w");

	// Setup SVG canvas
	fprintf(file, "<svg width=\"%d\" height=\"%d\">\n", cols, rows);
	fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"black\" />\n");

	for (int y = 0; y < rows; y++)
	{
		for (int x = 0; x < cols; x++)
		{
			const std::vector<int> &edgeIds = edgeIdMap.getEdgeIds(x, y);

			// At most positions edgeIds.size() is 0 (no edge), loop is skipped then
			for (int i = 0; i < (int)edgeIds.size(); i++)
			{
				double scale = (double)1 / edgeIds.size();
				// Color
				int r = (int)rgbValues.at(edgeIds[i]).val[0];
				int g = (int)rgbValues.at(edgeIds[i]).val[1];
				int b = (int)rgbValues.at(edgeIds[i]).val[2];

				fprintf(file, "<rect x=\"%d\" y=\"%f\" width=\"1\" height=\"%f\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y + scale * i, scale, r, g, b);
			}

			// Print coordinates
			if (edgeIds.size() > 0)
			{
				// fprintf(file, "<text x=\"%f\" y=\"%f\" style=\"fill:grey; font-size:0.15px;\">[%d,%d]</text>\n", x + 0.03, y + 0.95, x, y);
			}
			auto cluster = edgeIdMap.getCluster(x, y);
			// Additional marker for ambiguous points
			if (cluster.size() == 1)
			{
				fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", x, y, 255, 255, 255);
			}
			else if (cluster.size() > 1)
			{
				for (auto p : cluster)
				{
					fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"stroke-width:0.1;stroke:rgb(%d,%d,%d);fill:none;\" />", p.x, p.y, 255, 0, 0);
				}
			}
		}
	}

	// End and close SVG file
	fprintf(file, "</svg>");
	fclose(file);

	std::cout << "File edgeIdMap.svg written.\n";
}
