#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>
#include <string>
#include <iostream>
#include <memory>

// Typedef for server to make the code more readable
typedef websocketpp::server<websocketpp::config::asio> server;

class WebSocketServer {
public:
    // Constructor to initialize the server with the specified port
    WebSocketServer(uint16_t port);

    // Start the server in a separate thread
    void run();

    // Broadcast a message to all connected clients
    void broadcast(const std::string& message);

private:
    server wsServer;  // The WebSocket server instance
    uint16_t port;    // Port number on which the server runs
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;  // Set of connections
    std::mutex connectionMutex;  // Mutex to synchronize access to the connections set

    // Callback function called when a new connection is opened
    void onOpen(websocketpp::connection_hdl hdl);

    // Callback function called when a connection is closed
    void onClose(websocketpp::connection_hdl hdl);

    // Callback function called when a message is received
    void onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg);

    // Helper function to log messages with timestamps
    static void log(const std::string& msg) {
        std::cout << "[" << std::time(nullptr) << "] " << msg << std::endl;
    }
};

// Custom exception class for WebSocketServer errors
class WebSocketServerException : public std::exception {
public:
    explicit WebSocketServerException(const std::string& message) : message(message) {}
    virtual const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};

#endif // WEBSOCKETSERVER_H

