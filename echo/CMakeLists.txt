include(FetchContent)
include(FindProtobuf)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(proto)
 
add_executable(client
    client/main.cpp
    client/client.cpp
    client/client.h
)
target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(client pthread fmt::fmt proto_lib)

add_executable(server
    server/main.cpp
    server/server.cpp
)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server pthread fmt::fmt proto_lib)

