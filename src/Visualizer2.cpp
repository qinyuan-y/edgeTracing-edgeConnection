#include "Visualizer2.h"

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

void Visualizer2::saveConnectedEdgesAsSVG(cv::Mat &img, const EdgeIdMap &edgeIdMap, std::vector<std::vector<cv::Point>> completeEdgeCombinationSequences, std::string filename)
{
	// Generate a color for each edge
	std::vector<cv::Scalar> rgbValues = generateRgbValues(completeEdgeCombinationSequences.size());

	// Open new SVG file
	FILE *file;
	filename.erase(filename.begin() + 40, filename.end());
	std::string svgName = filename + ".svg";
	file = fopen(svgName.c_str(), "w");

	// Setup SVG canvas
	fprintf(file, "<svg width=\"%d\" height=\"%d\">\n", img.cols, img.rows);
	fprintf(file, "<rect width=\"100%%\" height=\"100%%\" fill=\"black\" />\n");

	// Data structure to mark ambiguous points
	std::vector<std::pair<cv::Point, std::vector<int>>> ambiguousPoints;
	cv::Mat sharedPixels(img.size(), CV_8UC1, 0.0);

	// Draw pixels of each edge (i = edgeId)
	for (auto &edge : completeEdgeCombinationSequences)
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

	for (int i = 0; i < (int)completeEdgeCombinationSequences.size(); i++)
	{
		for (int j = 0; j < (int)completeEdgeCombinationSequences[i].size(); j++)
		{
			for (int k = 0; k < (int)ambiguousPoints.size(); k++)
			{
				if (ambiguousPoints[k].first == completeEdgeCombinationSequences[i][j])
				{
					ambiguousPoints[k].second.push_back(i);
				}
			}
		}
	}

	for (int i = 0; i < (int)completeEdgeCombinationSequences.size(); i++)
	{
		for (int j = 0; j < (int)completeEdgeCombinationSequences[i].size(); j++)
		{
			// Position
			int x = completeEdgeCombinationSequences[i][j].x;
			int y = completeEdgeCombinationSequences[i][j].y;
			// Only print usual pixel here
			if (!(sharedPixels.at<uchar>(y, x) > 1.0))
			{

				// Color
				int r = (int)rgbValues.at(i).val[0];
				int g = (int)rgbValues.at(i).val[1];
				int b = (int)rgbValues.at(i).val[2];

				fprintf(file, "<rect x=\"%d\" y=\"%d\" width=\"1\" height=\"1\" style=\"fill:rgb(%d,%d,%d);\" />\n", x, y, r, g, b);

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

	std::cout << "File connectedEdges.svg written.\n";
}


