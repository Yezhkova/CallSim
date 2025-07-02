include(FetchContent)
include(FindProtobuf)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
)
FetchContent_MakeAvailable(fmt)

add_subdirectory(proto)

file(GLOB CLIENT_SOURCES
    client/*.cpp
    client/*.h
) 
add_executable(client ${CLIENT_SOURCES})

target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(client pthread fmt::fmt proto_lib magic_enum::magic_enum)

file(GLOB SERVER_SOURCES
    server/*.cpp
    server/*.h
) 
add_executable(server ${SERVER_SOURCES})
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server pthread fmt::fmt proto_lib magic_enum::magic_enum)

