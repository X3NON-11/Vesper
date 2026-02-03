#include "../include/utils/urlEncoding.h"

std::string decodeURL(std::string &url) {
    std::ostringstream decoded;
    decoded.fill('0');

    for (size_t i = 0; i < url.length(); ++i) {
        if (url[i] == '%' && i + 2 < url.length()) {
            // Extract the two hex digits
            std::string hex = url.substr(i + 1, 2);
            int value = std::stoi(hex, nullptr, 16);
            decoded << static_cast<char>(value);
            // Skip the next two characters
            i += 2;
        } else if (url[i] == '+') {
            // Replace '+' with space
            decoded << ' ';
        } else {
            decoded << url[i];
        }
    }

    return decoded.str();
}