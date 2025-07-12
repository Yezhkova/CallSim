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
find_package(Protobuf REQUIRED)

enable_testing()

set(TEST_SOURCES
    test/connectedStateTest.cpp
    # test/registeredStateTest.cpp
    # test/callingStateTest.cpp
    # test/answeringStateTest.cpp
    src/sessionFsm.cpp
)

add_executable(server_unit_tests
    ${TEST_SOURCES}
    test/mockSession.h
    ../proto/message.pb.cc
)

target_include_directories(server_unit_tests
    PUBLIC inc ${CMAKE_SOURCE_DIR}/proto
)
target_link_libraries(server_unit_tests
    gmock gtest gtest_main
    protobuf::libprotobuf
    fmt::fmt
)
include(GoogleTest)
gtest_discover_tests(server_unit_tests)
