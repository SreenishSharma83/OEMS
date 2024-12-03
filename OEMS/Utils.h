#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>

namespace Utils {
    /**
     * Callback function for handling data received from a cURL request.
     * 
     * @param contents Pointer to the received data.
     * @param size Size of each data chunk.
     * @param nmemb Number of data chunks.
     * @param userp User-defined pointer to the data where content is stored.
     * @return The number of bytes processed, or 0 if an error occurred.
     */
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
            return 0;  // Indicate an error occurred
        }
    }

    /**
     * Constructs a query string from a JSON object for use in a URL.
     * 
     * @param params JSON object containing key-value pairs for the query.
     * @return A formatted query string.
     * @throws std::invalid_argument if the JSON object contains invalid data.
     */
    std::string buildQuery(const nlohmann::json& params) {
        try {
            std::string query;
            for (auto it = params.begin(); it != params.end(); ++it) {
                if (!query.empty()) {
                    query += "&";
                }
                query += it.key() + "=" + it.value().dump();
            }
            return query;
        } catch (const std::exception& e) {
            std::cerr << "Error in buildQuery: " << e.what() << std::endl;
            throw;  // Re-throw the exception after logging
        }
    }
}

#endif // UTILS_H

