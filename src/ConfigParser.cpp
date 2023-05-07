#include "ConfigParser.h"

ConfigParser::ConfigParser()
{
    std::cout << "Read Config...\n";
}

void ConfigParser::readConfig(std::string file)
{
    std::vector<int> config;

    // Open csv file
    std::ifstream configFile(file);

    std::string line, colname;
    std::string val;
    int colIdx = 0;

    // Read line after line
    while (std::getline(configFile, line))
    {
        std::istringstream ss(line);
        configData.push_back(std::vector<std::string>());
        // Split after every ','
        while (std::getline(ss, val, ','))
        {
            configData[colIdx].push_back(val);
        }
        colIdx++;
    }

    configFile.close();
}

std::vector<std::string> ConfigParser::getData(int i)
{
    return configData[i];
}
