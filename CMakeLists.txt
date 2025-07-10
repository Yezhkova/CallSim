cmake_minimum_required(VERSION 3.5)
find_package(Boost REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)
FetchContent_MakeAvailable(fmt)

project(CallSim LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_THREAD_LIBS_INIT "-pthread")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


enable_testing()

add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(proto)
# add_subdirectory(turnstileEnum)
# add_subdirectory(turnstileEnum/test)
# add_subdirectory(turnstileOOP)
# add_subdirectory(turnstileOOP/test)
# add_subdirectory(echo)