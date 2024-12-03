#include "WebSocketServer.h"
#include <iostream>
#include <sstream>

WebSocketServer::WebSocketServer(uint16_t port) : port(port) {
    wsServer.init_asio();
    wsServer.set_open_handler(std::bind(&WebSocketServer::onOpen, this, std::placeholders::_1));
    wsServer.set_close_handler(std::bind(&WebSocketServer::onClose, this, std::placeholders::_1));
    wsServer.set_message_handler(std::bind(&WebSocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));

    WebSocketServer::log("WebSocketServer initialized on port " + std::to_string(port));
}

void WebSocketServer::run() {
    try {
        wsServer.listen(port);
        wsServer.start_accept();
        WebSocketServer::log("Server is running and accepting connections.");
        wsServer.run();
    } catch (const std::exception& e) {
        WebSocketServer::log("Error running WebSocket server: " + std::string(e.what()));
    }
}

void WebSocketServer::broadcast(const std::string& message) {
    try {
        std::lock_guard<std::mutex> lock(connectionMutex);
        for (auto& hdl : connections) {
            wsServer.send(hdl, message, websocketpp::frame::opcode::text);
        }
        WebSocketServer::log("Broadcast message sent: " + message);
    } catch (const std::exception& e) {
        WebSocketServer::log("Error broadcasting message: " + std::string(e.what()));
    }
}

void WebSocketServer::onOpen(websocketpp::connection_hdl hdl) {
    try {
        std::lock_guard<std::mutex> lock(connectionMutex);
        connections.insert(hdl);
        WebSocketServer::log("Client connected.");
    } catch (const std::exception& e) {
        WebSocketServer::log("Error handling client connection: " + std::string(e.what()));
    }
}

void WebSocketServer::onClose(websocketpp::connection_hdl hdl) {
    try {
        std::lock_guard<std::mutex> lock(connectionMutex);
        connections.erase(hdl);
        WebSocketServer::log("Client disconnected.");
    } catch (const std::exception& e) {
        WebSocketServer::log("Error handling client disconnection: " + std::string(e.what()));
    }
}

void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    try {
        WebSocketServer::log("Received message from client: " + msg->get_payload());
        // Example of echoing back the received message
        wsServer.send(hdl, msg->get_payload(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        WebSocketServer::log("Error handling incoming message: " + std::string(e.what()));
    }
}

void WebSocketServer::log(const std::string& msg) {
    std::ostringstream logMsg;
    logMsg << "[" << std::time(nullptr) << "] " << msg;
    std::cout << logMsg.str() << std::endl;
}

