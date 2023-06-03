#ifndef MYCURL_H
std::string httpRequest(const std::string& requestType, const std::string& url, const std::vector<std::string>& headers, const std::string& data = "");
#define MYCURL_H
#endif
