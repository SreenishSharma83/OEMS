#include "Utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace Utils {

    size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        try {
            size_t totalSize = size * nmemb;
            if (totalSize == 0) {
                throw std::runtime_error("Received empty data chunk.");
            }

            std::string* response = static_cast<std::string*>(userp);
            response->append(static_cast<char*>(contents), totalSize);
            return totalSize;
        } catch (const std::exception& e) {
            std::cerr << "Error in writeCallback: " << e.what() << std::endl;
            return 0;  // Indicate an error occurred by returning 0
        }
    }

    std::string buildQuery(const nlohmann::json& params) {
        try {
            std::ostringstream queryStream;
            bool first = true;

            for (auto it = params.begin(); it != params.end(); ++it) {
                if (!first) {
                    queryStream << "&";
                }
                first = false;
                // Ensure key-value pairs are URL-encoded properly
                queryStream << it.key() << "=" << encodeURIComponent(it.value().dump());
            }
            return queryStream.str();
        } catch (const std::exception& e) {
            std::cerr << "Error in buildQuery: " << e.what() << std::endl;
            throw;  // Re-throw the exception after logging
        }
    }

    std::string encodeURIComponent(const std::string& str) {
        std::ostringstream encoded;
        for (unsigned char c : str) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                encoded << c;
            } else {
                encoded << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            }
        }
        return encoded.str();
    }
}

