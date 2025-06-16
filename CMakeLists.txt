cmake_minimum_required(VERSION 3.5)

project(CallSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

enable_testing()

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(proto)
add_subdirectory(echo)