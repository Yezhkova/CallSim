include(FetchContent)
include(FindProtobuf)

find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(proto)
 
add_executable(client_echo
    client/main.cpp
    client/client.cpp
    client/client.h
)
target_include_directories(client_echo PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(client_echo PRIVATE pthread fmt::fmt proto_lib)

add_executable(server_echo
    server/main.cpp
    server/server.cpp
)
target_include_directories(server_echo PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server_echo PRIVATE pthread fmt::fmt proto_lib)
