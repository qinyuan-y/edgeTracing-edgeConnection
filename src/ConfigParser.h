#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

// I/O includes
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <string>

class ConfigParser
{
public:
    /** Constructor.
	 */
    ConfigParser();

    /** Read Config from csv File
     *  @file Path and name of config file.
     */
    void readConfig(std::string file);

    /** Return config data of given line i
	 */
    std::vector<std::string> getData(int i);

private:
    std::vector<std::vector<std::string>> configData;
};

#endif // CONFIGPARSER_H
