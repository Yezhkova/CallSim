add_library(client_lib
    src/clientTransport.cpp
    src/UiController.cpp
    src/clientFsm.cpp
)
target_include_directories(client_lib
    PUBLIC inc ${CMAKE_SOURCE_DIR}/proto
)
target_link_libraries(client_lib
    PUBLIC pthread fmt::fmt proto_lib magic_enum::magic_enum
)

add_executable(client
    src/main.cpp
)
target_include_directories(client
    PRIVATE inc ${CMAKE_SOURCE_DIR}/proto
)
target_link_libraries(client
    PRIVATE client_lib 
)

enable_testing()
find_package(GTest REQUIRED)

add_executable(client_tests
    test/ClientTest.cpp
)
target_link_libraries(client_tests
    PRIVATE client_lib
            GTest::GTest
            GTest::Main
)
add_test(NAME ClientTest COMMAND client_tests)
