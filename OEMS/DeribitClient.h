// DeribitClient.h
#ifndef DERIBITCLIENT_H
#define DERIBITCLIENT_H

#include <string>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <mutex>
#include <curl/curl.h>  // Example of HTTP client library if needed

// Custom exception class for API-related errors
class DeribitClientException : public std::runtime_error {
public:
    explicit DeribitClientException(const std::string& message)
        : std::runtime_error("DeribitClient Error: " + message) {}
};

class DeribitClient {
public:
    // Constructor with client credentials
    DeribitClient(const std::string& clientId, const std::string& clientSecret);

    // Delete copy constructor and assignment operator to prevent copying
    DeribitClient(const DeribitClient&) = delete;
    DeribitClient& operator=(const DeribitClient&) = delete;

    // Authenticate with Deribit API to retrieve an access token
    bool authenticate();

    // Place an order with given parameters
    nlohmann::json placeOrder(const std::string& instrumentName, double amount, double price,
                              const std::string& type, const std::string& side);

    // Cancel an order by ID
    nlohmann::json cancelOrder(const std::string& orderId);

    // Modify an existing order
    nlohmann::json modifyOrder(const std::string& orderId, double amount, double price);

    // Retrieve the order book for a given instrument
    nlohmann::json getOrderBook(const std::string& instrumentName);

    // Retrieve the current positions
    nlohmann::json getPositions();

    // Optionally, get the access token (useful for debugging or verification)
    std::optional<std::string> getAccessToken() const;

private:
    // Client credentials
    std::string clientId;
    std::string clientSecret;

    // Access token for authenticated API requests
    std::string accessToken;

    // Base URL for Deribit API
    const std::string baseUrl = "https://www.deribit.com/api/v2";

    // Mutex for thread safety
    mutable std::mutex accessTokenMutex;

    // Helper function to send HTTP requests, used internally
    nlohmann::json sendRequest(const std::string& method, const std::string& endpoint,
                               const nlohmann::json& params = nlohmann::json::object());

    // Helper function for handling HTTP response and error checking
    void handleHttpError(const CURLcode& curlResult);

    // Helper function for token refresh logic (if required)
    bool refreshAccessToken();
};

#endif // DERIBITCLIENT_H

