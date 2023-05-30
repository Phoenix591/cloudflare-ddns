#include <iostream>
#include <curl/curl.h>
#include <libconfig.h++>
#include <json/json.h>

// Callback function to write response data into a string
size_t WriteCallback(char* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append(ptr, size * nmemb);
    return size * nmemb;}
int main(){
    // Get the user's home directory
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << "Error: Unable to get the user's home directory." << std::endl;
        return 1;
    }
   std::string configFilePath = std::string(homeDir) + "/.cloudflare/app.cfg";
    // Load the configuration file
    libconfig::Config cfg;
    try {
        cfg.readFile(configFilePath.c_str());
    }
    catch (const libconfig::FileIOException& e) {
        std::cerr << "Error reading the configuration file: " << e.what() << std::endl;
        return 1;
    }
    catch (const libconfig::ParseException& e) {
        std::cerr << "Error parsing the configuration file: " << e.getError() << " on line " << e.getLine() << std::endl;
        return 1;
    }
    // Get the authentication key from the configuration
    std::string authKey;
    std::string zoneName;

    try {
        authKey = cfg.lookup("token").c_str();
        zoneName = cfg.lookup("zone_name").c_str();

    }
    catch (const libconfig::SettingNotFoundException& e) {
        std::cerr << "Error: Authentication key not found in the configuration file." << std::endl;
        return 1;    }
    CURL* curl = curl_easy_init();
    if (curl) {        std::string response;
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
		            	std::cout << "Zone Name: " << currentZoneName << std::endl;
		                std::cout << "Zone ID: " << currentZoneId << std::endl;
		                break;  // Exit the loop after finding the desired zone        
		                }
		     }
		} else {
		    std::cerr << "Error parsing JSON response: " << error << std::endl;
		}
	}
        curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}

		return 0;
}
