#include <iostream>
#include <cstdlib> // For getenv()
#include <json/json.h>
#include <csignal>
#include <iostream>
#include <string>
#include <unordered_map>
#include "config.h"
#include "mycurl.h"
#include "getip.h"

using namespace std;

// Function to retrieve the ID of a subdomain in a given zone
std::string getSubdomainId(const std::string& zoneId, const std::string& subdomain, const std::string& recordType, const std::string& zoneName, const std::vector<std::string>& headers)
{   std::string subdomainId;
    std::string apiUrl = "https://api.cloudflare.com/client/v4/zones/" + zoneId + "/dns_records";

    // Construct the request URL with the specified subdomain and record type
    std::string name;
    if (!subdomain.empty()) {
        name = subdomain + "." + zoneName;
    } else {
        name = zoneName;
    }
    apiUrl += "?type=" + recordType + "&name=" + name;
    std::cout << name << std::endl;

    std::string dnsRecordsResponse = httpRequest("get", apiUrl, headers, "");

    // Parse the JSON response
    Json::CharReaderBuilder jsonReader;
    Json::Value jsonData;
    std::string error;
    std::istringstream responseStream(dnsRecordsResponse);
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
            std::cout << "API Response: " << dnsRecordsResponse << std::endl;
        }
    } else {
        std::cerr << "Error parsing JSON response: " << error << std::endl;
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
    std::string configFilePath = std::string(homeDir) + "/.cloudflare/ddns.cfg";
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
    // std::cout << subdomainsStr << std::endl;

    std::vector<std::string> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Authorization: Bearer " + authKey);
    // get external ip
    int ipAddressType = 4; // or 6 for IPv6
    std::string externalIPAddress = getExternalIPAddress(ipAddressType);
    std::string externalIP6Address;
    if (useIPv6) {
        int ipAddressType = 6;
        externalIP6Address = getExternalIPAddress(ipAddressType);
    }

    std::string zoneUrl = "https://api.cloudflare.com/client/v4/zones";
    std::string zoneResponse = httpRequest("get", zoneUrl, headers, "");

    // Parse the JSON response
    Json::CharReaderBuilder jsonReader;
    Json::Value jsonData;
    std::string error;
    std::istringstream responseStream(zoneResponse);
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
                string zoneRecordIdA = getSubdomainId(currentZoneId, "","A", zoneName,headers);
                cout << zoneRecordIdA << endl;
                // Construct the URL for the PUT request
                std::string putUrlA = "https://api.cloudflare.com/client/v4/zones/" + currentZoneId + "/dns_records/" + zoneRecordIdA;
                std::string putDataA = R"({
                            "content": ")" + externalIPAddress + R"(",
                            "type": "A",
                            "name": ")" + zoneName + R"("
                })";
                        std::string putResponseA = httpRequest("put", putUrlA, headers, putDataA);
                        std::cout << "PUT Response for root domain " << zoneName << " (A): " << putResponseA << std::endl;
                        if (useIPv6) {
                            string zoneRecordIdAAAA = getSubdomainId(currentZoneId, "","AAAA", zoneName,headers);
                        cout << zoneRecordIdAAAA << endl;
                        // Construct the URL for the PUT request
                        std::string putUrlAAAA = "https://api.cloudflare.com/client/v4/zones/" + currentZoneId + "/dns_records/" + zoneRecordIdAAAA;
                        std::string putDataAAAA = R"({
                            "content": ")" + externalIP6Address + R"(",
                            "type": "AAAA",
                            "name": ")" + zoneName + R"("
                        })";
                        std::string putResponseAAAA = httpRequest("put", putUrlA, headers, putDataA);
                        std::cout << "PUT Response for root domain " << zoneName << " (AAAA): " << putResponseAAAA << std::endl;
                        }
                        // Iterate through the subdomains
                        for (const auto& subdomain : subdomains) {
    std::string subdomainIdA = getSubdomainId(currentZoneId, subdomain, "A", zoneName, headers);
    if (!subdomainIdA.empty()) {
        subdomainIdsA.push_back(subdomainIdA);

        // Construct the URL for the PUT request
        std::string putUrlA = "https://api.cloudflare.com/client/v4/zones/" + currentZoneId + "/dns_records/" + subdomainIdA;

        // Combine subdomain and zone name to form the complete name
        std::string fullName = subdomain + "." + zoneName;

        // Construct the data for the PUT request
        std::string putDataA = R"({
            "content": ")" + externalIPAddress + R"(",
            "type": "A",
            "name": ")" + fullName + R"("
        })";
        cout << putDataA << endl;
        cout << externalIPAddress << endl;
        // Make the PUT request for record type 'A'
        std::string putResponseA = httpRequest("put", putUrlA, headers, putDataA);
        std::cout << "PUT Response for subdomain " << fullName << " (A): " << putResponseA << std::endl;
    }

    if (useIPv6) {
        std::string subdomainIdAAAA = getSubdomainId(currentZoneId, subdomain, "AAAA", zoneName, headers);
        std::string fullName = subdomain + "." + zoneName;
        if (!subdomainIdAAAA.empty()) {
            subdomainIdsAAAA.push_back(subdomainIdAAAA);

            // Construct the URL for the PUT request
            std::string putUrlAAAA = "https://api.cloudflare.com/client/v4/zones/" + currentZoneId + "/dns_records/" + subdomainIdAAAA;

            // Construct the data for the PUT request
            std::string putDataAAAA = R"({
                "content": ")" + externalIP6Address + R"(",
                "type": "AAAA",
                "name": ")" + fullName + R"("
            })";

            // Make the PUT request for record type 'AAAA'
            std::string putResponseAAAA = httpRequest("put", putUrlAAAA, headers, putDataAAAA);
            std::cout << "PUT Response for subdomain " << fullName << " (AAAA): " << putResponseAAAA << std::endl;
        }
    }
                        }


                        break;  // Exit the loop after finding the desired zone
            }
        }
    } // Json parsing
            else {
                std::cerr << "Error parsing JSON response: " << error << std::endl;
            }

    return 0;
}
