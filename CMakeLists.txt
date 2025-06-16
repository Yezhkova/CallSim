cmake_minimum_required(VERSION 3.5)

project(CallSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_THREAD_LIBS_INIT "-pthread")

enable_testing()

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(proto)
add_subdirectory(turnstileEnum)
add_subdirectory(turnstileEnum/test)
add_subdirectory(turnstileOOP)
add_subdirectory(turnstileOOP/test)
