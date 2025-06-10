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
add_subdirectory(test)
add_subdirectory(fmt)

add_library(turnstile STATIC turnstile.cpp)
target_link_libraries(turnstile PUBLIC fmt::fmt)
target_include_directories(turnstile PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include/magic_enum
)

add_executable(turnstileApp turnstileApp.cpp)
target_link_libraries(turnstileApp PUBLIC fmt::fmt turnstile)
target_include_directories(turnstileApp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/magic_enum/include/magic_enum
)
