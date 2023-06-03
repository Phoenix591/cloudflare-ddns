#include <curl/curl.h>
#include <iostream>
#include <string>
#include <vector>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string httpRequest(const std::string& requestType, const std::string& url, const std::vector<std::string>& headers, const std::string& data = "")
{
    std::string response;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headerList = nullptr;
        for (const std::string& header : headers)
            headerList = curl_slist_append(headerList, header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        if (requestType == "post")
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
        else if (requestType == "delete")
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

        if (!data.empty())
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            std::cerr << "Failed to perform HTTP request: " << curl_easy_strerror(res) << std::endl;

        curl_easy_cleanup(curl);
        curl_slist_free_all(headerList);
    }

    return response;
}
