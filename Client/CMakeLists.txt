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
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

find_package(Protobuf REQUIRED)

set(TEST_SOURCES
    test/connectedStateTest.cpp
    test/registeredStateTest.cpp
    # test/callingStateTest.cpp
    # test/answeringStateTest.cpp
    src/clientFsm.cpp
)

add_executable(client_unit_tests
    ${TEST_SOURCES}
    test/mockStateMachine.h
    ../proto/message.pb.cc
)

target_include_directories(client_unit_tests
    PUBLIC inc ${CMAKE_SOURCE_DIR}/proto
)
target_link_libraries(client_unit_tests
    gmock gtest gtest_main
    protobuf::libprotobuf
    fmt::fmt
)
include(GoogleTest)
gtest_discover_tests(client_unit_tests)
