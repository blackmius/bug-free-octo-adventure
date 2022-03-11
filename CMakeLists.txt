cmake_minimum_required(VERSION 3.0.0)
project(ping VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(ping src/main.cpp src/Pinger.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)