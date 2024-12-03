cmake_minimum_required(VERSION 3.10)
project(DeribitTradingSystem VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Include directories for external libraries
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    /usr/local/include  # Ensure WebSocket++ and nlohmann/json paths are correct
)

# Add executable
add_executable(DeribitTradingSystem
    main.cpp
    DeribitClient.cpp
    WebSocketServer.cpp
    Utils.cpp
)

# Link libraries
target_link_libraries(DeribitTradingSystem
    PRIVATE
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    pthread
    ssl
    crypto
)

# Ensure proper include path for external headers if needed
target_include_directories(DeribitTradingSystem
    PRIVATE
    /usr/local/include  # Ensure this is where WebSocket++ and nlohmann/json are located
)

# Optionally set a version for the target
set_target_properties(DeribitTradingSystem PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Add compile options for additional safety and warnings
target_compile_options(DeribitTradingSystem PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# If needed, add definitions for linking with specific libraries
# target_compile_definitions(DeribitTradingSystem PRIVATE YOUR_DEFINITION)

