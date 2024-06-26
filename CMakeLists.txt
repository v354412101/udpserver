project(udpserver)
cmake_minimum_required(VERSION 3.20)

include_directories(${PROJECT_SOURCE_DIR}/3rdparty/spdlog/include/)
link_directories(${PROJECT_SOURCE_DIR}/3rdparty/spdlog/lib)

add_executable(udpserver main.cpp UDPServer.cpp)

add_executable(udpclient UDPClient.cpp)

target_link_libraries(udpserver spdlog pthread)
target_link_libraries(udpclient spdlog)