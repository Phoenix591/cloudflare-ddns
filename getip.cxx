#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdlib>

std::string getExternalIPAddress(int ipType) {
    std::string digCommand = "dig -" + std::to_string(ipType) + " +short TXT o-o.myaddr.l.google.com @ns1.google.com";
    FILE* pipe = popen(digCommand.c_str(), "r");
    if (pipe == nullptr) {
        return "";
    }

    char buffer[128];
    std::string result;
    while (feof(pipe) == 0) {
        if (fgets(buffer, 128, pipe) != NULL) {
            result += buffer;
        }
    }

    pclose(pipe);

    // Trim leading and trailing quotes from the IP address
    result = result.substr(1, result.size() - 3);

    return result;
}
