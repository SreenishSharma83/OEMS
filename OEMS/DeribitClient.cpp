// DeribitClient.cpp
#include "DeribitClient.h"
#include <curl/curl.h>
#include <stdexcept>
#include <iostream>
#include "Utils.h"

DeribitClient::DeribitClient(const std::string& clientId, const std::string& clientSecret)
    : clientId(clientId), clientSecret(clientSecret), baseUrl("https://test.deribit.com/api/v2/") {}

bool DeribitClient::authenticate() {
    nlohmann::json params = {
        {"grant_type", "client_credentials"},
        {"client_id", clientId},
        {"client_secret", clientSecret}
    };
    try {
        auto response = sendRequest("POST", "public/auth", params);
        if (response.contains("result") && response["result"].contains("access_token")) {
            std::lock_guard<std::mutex> lock(accessTokenMutex);
            accessToken = response["result"]["access_token"];
            return true;
        } else {
            std::cerr << "Authentication failed: " << response.dump() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error during authentication: " << e.what() << std::endl;
    }
    return false;
}

nlohmann::json DeribitClient::placeOrder(const std::string& instrumentName, double amount, double price,
                                         const std::string& type, const std::string& side) {
    nlohmann::json params = {
        {"instrument_name", instrumentName},
        {"amount", amount},
        {"type", type},
        {"price", price},
        {"side", side}
    };
    try {
        return sendRequest("POST", "private/" + side, params);
    } catch (const std::exception& e) {
        std::cerr << "Error placing order: " << e.what() << std::endl;
        return nullptr;
    }
}

nlohmann::json DeribitClient::cancelOrder(const std::string& orderId) {
    nlohmann::json params = {{"order_id", orderId}};
    try {
        return sendRequest("POST", "private/cancel", params);
    } catch (const std::exception& e) {
        std::cerr << "Error canceling order: " << e.what() << std::endl;
        return nullptr;
    }
}

nlohmann::json DeribitClient::modifyOrder(const std::string& orderId, double amount, double price) {
    nlohmann::json params = {
        {"order_id", orderId},
        {"amount", amount},
        {"price", price}
    };
    try {
        return sendRequest("POST", "private/edit", params);
    } catch (const std::exception& e) {
        std::cerr << "Error modifying order: " << e.what() << std::endl;
        return nullptr;
    }
}

nlohmann::json DeribitClient::getOrderBook(const std::string& instrumentName) {
    nlohmann::json params = {{"instrument_name", instrumentName}};
    try {
        return sendRequest("GET", "public/get_order_book", params);
    } catch (const std::exception& e) {
        std::cerr << "Error fetching order book: " << e.what() << std::endl;
        return nullptr;
    }
}

nlohmann::json DeribitClient::getPositions() {
    try {
        return sendRequest("GET", "private/get_positions");
    } catch (const std::exception& e) {
        std::cerr << "Error fetching positions: " << e.what() << std::endl;
        return nullptr;
    }
}

nlohmann::json DeribitClient::sendRequest(const std::string& method, const std::string& endpoint,
                                          const nlohmann::json& params) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw DeribitClientException("Failed to initialize CURL.");
    }

    std::string url = baseUrl + endpoint;
    std::string responseStr;
    struct curl_slist* headers = nullptr;

    if (!accessToken.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    }

    if (method == "GET" && !params.empty()) {
        url += "?" + Utils::buildQuery(params);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Utils::writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);  // Set to true for security in production

    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    if (method == "POST") {
        std::string postData = params.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error (" << res << "): " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        if (headers) {
            curl_slist_free_all(headers);
        }
        throw DeribitClientException("Failed to send request.");
    }

    curl_easy_cleanup(curl);
    if (headers) {
        curl_slist_free_all(headers);
    }

    try {
        return nlohmann::json::parse(responseStr);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing JSON response: " << e.what() << std::endl;
        throw DeribitClientException("Failed to parse response JSON.");
    }
}

