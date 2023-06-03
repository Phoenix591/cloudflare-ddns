#include <iostream>
#include <cstdlib> // For getenv()
#include <curl/curl.h>
#include <libconfig.h++>
#include <json/json.h>
#include <csignal>
#include <iostream>
#include <string>
#include <unordered_map>
#include "config.h"

// Callback function to write response data into a string
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Function to retrieve the ID of a subdomain in a given zone
std::string getSubdomainId(const std::string& zoneId, const std::string& subdomain, const std::string& recordType, const std::string& zoneName, struct curl_slist* headers)
{   std::string subdomainId;
    std::string apiUrl = "https://api.cloudflare.com/client/v4/zones/" + zoneId + "/dns_records";

    // Construct the request URL with the specified subdomain and record type
    std::string name = subdomain + "." + zoneName;
    apiUrl += "?type=" + recordType + "&name=" + name;
    std::cout << name << std::endl;

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());

        // Set the headers for the request
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the callback function to write response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Parse the JSON response
            Json::CharReaderBuilder jsonReader;
            Json::Value jsonData;
            std::string error;
            std::istringstream responseStream(response);
            if (Json::parseFromStream(jsonReader, responseStream, &jsonData, &error)) {
                // Check if any results are found
                if (jsonData["success"].asBool() && jsonData["result_info"]["count"].asInt() > 0) {
                    // Output the subdomain and ID
                    std::cout << "Subdomain: " << subdomain << std::endl;
                    std::cout << "Subdomain ID: " << jsonData["result"][0]["id"].asString() << std::endl;
                    subdomainId = jsonData["result"][0]["id"].asString();
                } else {
                    // Output the entire response
                    std::cout << "No results found for the specified subdomain." << std::endl;
                    std::cout << "API Response: " << response << std::endl;
                }
            } else {
                std::cerr << "Error parsing JSON response: " << error << std::endl;
            }
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
     return subdomainId;
}
int main()
{
    std::vector<std::string> subdomainIdsA;
    std::vector<std::string> subdomainIdsAAAA;
    // Get the user's home directory
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << "Error: Unable to get the user's home directory." << std::endl;
        return 1;
    }

    // Construct the configuration file path
    std::string configFilePath = std::string(homeDir) + "/.cloudflare/app.cfg";
    std::cout << configFilePath << std::endl;
 std::unordered_map<std::string, std::string> configValues;
 readConfigFile(configFilePath, configValues);
    std::cout << "Config Values:" << std::endl;
    for (const auto& kvp : configValues) {
        std::cout << kvp.first << " = " << kvp.second << std::endl;
    }

   std::string authKey = configValues["token"];
   std::string zoneName = configValues["zone_name"];
   std::string subdomainsStr = configValues["subdomains"];
   std::cout << "subdomains: " << subdomainsStr << std::endl;
   std::string useIPv6Str = configValues["useIPv6"];
    bool useIPv6 = (useIPv6Str == "1");
    std::cout << "config useipv6: " << configValues["useIPv6" ]<< " useIPv6: "<< useIPv6 << std::endl;

    // Split the subdomains string into individual subdomains
    std::vector<std::string> subdomains = splitString(subdomainsStr, ' ');
    std::cout << subdomainsStr << std::endl;

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.cloudflare.com/client/v4/zones");

        // Set the required headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + authKey;
        headers = curl_slist_append(headers, authHeader.c_str());

        // Set the headers for the request
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the callback function to write response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Parse the JSON response
            Json::CharReaderBuilder jsonReader;
            Json::Value jsonData;
            std::string error;
            std::istringstream responseStream(response);
            if (Json::parseFromStream(jsonReader, responseStream, &jsonData, &error)) {
                // Find the zone with the specified name
                for (const auto& zone : jsonData["result"]) {
                    std::string currentZoneName = zone["name"].asString();
                    std::string currentZoneId = zone["id"].asString();

                    // Check if the current zone matches the desired zone name
                    if (currentZoneName == zoneName) {
                        // Output the zone name and ID
                        std::cout << "Zone Name: " << currentZoneName << std::endl;
                        std::cout << "Zone ID: " << currentZoneId << std::endl;

                        // Iterate through the subdomains
                        for (const auto& subdomain : subdomains) {
                            // std::cout << subdomain  << std::endl;
                            // Get the subdomain ID for record type 'A'
                            std::string subdomainIdA = getSubdomainId(currentZoneId, subdomain, "A", zoneName, headers);
                             if (!subdomainIdA.empty()) {
                                    subdomainIdsA.push_back(subdomainIdA);
                                }

                            // If useIPv6 is true, get the subdomain ID for record type 'AAAA'
                            if (useIPv6) {
                                std::cout << "Ipv6" << std::endl;
                                 std::string subdomainIdAAAA = getSubdomainId(currentZoneId, subdomain, "AAAA", zoneName, headers);
                                  if (!subdomainIdAAAA.empty()) {
                                    subdomainIdsAAAA.push_back(subdomainIdAAAA);
                                }
                            }
                        }

                        break;  // Exit the loop after finding the desired zone
                    }
                }
            }
            else {
                std::cerr << "Error parsing JSON response: " << error << std::endl;
            }
        }

        // Clean up
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return 0;
}
