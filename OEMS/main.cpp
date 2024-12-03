// main.cpp
#include <iostream>
#include <thread>
#include <csignal>
#include <chrono>
#include "DeribitClient.h"
#include "WebSocketServer.h"
#include "spdlog/spdlog.h"

// Global WebSocket server instance for signal handling
WebSocketServer* wsServerPtr = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    spdlog::warn("Interrupt signal ({}) received. Shutting down...", signum);
    if (wsServerPtr) {
        wsServerPtr->stop();
    }
    exit(signum);
}

int main() {
    // Register signal handler
    std::signal(SIGINT, signalHandler);

    // Load configuration (replace with a proper configuration file loader if available)
    const std::string clientId = "YOUR_CLIENT_ID";
    const std::string clientSecret = "YOUR_CLIENT_SECRET";
    const int wsPort = 8080;

    // Initialize the Deribit API client
    DeribitClient deribitClient(clientId, clientSecret);

    // Retry mechanism for authentication
    constexpr int MAX_RETRIES = 5;
    bool isAuthenticated = false;

    for (int attempt = 0; attempt < MAX_RETRIES; ++attempt) {
        spdlog::info("Attempting to authenticate with Deribit API (Attempt {})...", attempt + 1);
        if (deribitClient.authenticate()) {
            isAuthenticated = true;
            spdlog::info("Successfully authenticated with Deribit API.");
            break;
        }
        spdlog::error("Authentication failed. Retrying...");
        std::this_thread::sleep_for(std::chrono::seconds(2 << attempt)); // Exponential backoff
    }

    if (!isAuthenticated) {
        spdlog::error("Failed to authenticate after {} attempts. Exiting...", MAX_RETRIES);
        return -1;
    }

    // Start the WebSocket server in a separate thread
    WebSocketServer wsServer(wsPort);
    wsServerPtr = &wsServer;
    std::thread wsThread([&wsServer]() {
        spdlog::info("Starting WebSocket server on port {}...", wsServer.getPort());
        wsServer.run();
    });

    // Example usage of order management functions
    std::string instrumentName = "BTC-PERPETUAL";
    double amount = 10.0; // Amount in contracts
    double price = 50000.0; // Price in USD

    // Place an order
    auto orderResult = deribitClient.placeOrder(instrumentName, amount, price, "limit", "buy");
    if (orderResult.is_null()) {
        spdlog::error("Order placement failed.");
    } else {
        spdlog::info("Order placed successfully: {}", orderResult.dump());
    }

    // Get the order book
    auto orderBook = deribitClient.getOrderBook(instrumentName);
    if (orderBook.is_null()) {
        spdlog::error("Failed to get order book.");
    } else {
        spdlog::info("Order book: {}", orderBook.dump());
    }

    // View current positions
    auto positions = deribitClient.getPositions();
    if (positions.is_null()) {
        spdlog::error("Failed to get positions.");
    } else {
        spdlog::info("Current positions: {}", positions.dump());
    }

    // Keep the main thread alive until WebSocket server shuts down
    if (wsThread.joinable()) {
        wsThread.join();
    }

    spdlog::info("Application shutting down.");
    return 0;
}

