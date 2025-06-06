cmake_minimum_required(VERSION 3.5)

project(CallSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

enable_testing()

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(proto)
