add_library(server_lib
    src/server.cpp
    src/session.cpp 
    src/sessionFsm.cpp
)
target_include_directories(server_lib
    PUBLIC inc ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/StateMachine
)
target_link_libraries(server_lib
    PUBLIC pthread fmt::fmt proto_lib magic_enum::magic_enum
)

add_executable(server
    src/main.cpp
)
target_include_directories(server
    PRIVATE inc ${CMAKE_SOURCE_DIR}/proto ${CMAKE_SOURCE_DIR}/StateMachine
)
target_link_libraries(server
    PRIVATE server_lib
)

enable_testing()
find_package(GTest REQUIRED)

add_executable(server_tests
    test/ServerTest.cpp
)
target_link_libraries(server_tests
    PRIVATE server_lib
            GTest::GTest
            GTest::Main
)
add_test(NAME ServerTest COMMAND server_tests)
