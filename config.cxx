#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

#include <iostream>

std::unordered_map<std::string, std::string> readConfigFile(const std::string& filePath, std::unordered_map<std::string, std::string>& configValues)
{



    std::ifstream configFile(filePath);
    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = line.substr(0, delimiterPos);
                std::string value = line.substr(delimiterPos + 1);
                // Trim leading spaces from the value
                value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](int ch) {
                    return std::isspace(ch) == 0;
                }));
                // Trailing spaces from key
                key.erase(std::find_if(key.rbegin(), key.rend(), [](int ch) {
                    return std::isspace(ch) == 0;
                }).base(), key.end());

                configValues[key] = value;
            }
        }
        configFile.close();
    }

    return configValues;
}

std::vector<std::string> splitString(const std::string& input, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(input);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
