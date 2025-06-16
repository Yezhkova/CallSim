find_package(Boost REQUIRED)

add_executable(client
    client/main.cpp
    client/client.cpp
    client/client.h
)
target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(client pthread)

add_executable(server
    server/main.cpp
    server/server.cpp
)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server pthread)

