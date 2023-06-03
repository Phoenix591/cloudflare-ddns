#ifndef CONFIG_H
std::unordered_map<std::string, std::string> readConfigFile(const std::string& filePath, std::unordered_map<std::string, std::string>& configValues);
std::vector<std::string> splitString(const std::string& input, char delimiter);
#define CONFIG_H
#endif
