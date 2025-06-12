enable_testing()
find_package(GTest REQUIRED)
find_package(fmt REQUIRED)

add_executable(turnstile_test turnstile_test.cpp)
target_link_libraries(turnstile_test 
    GTest::GTest 
    GTest::Main 
    fmt::fmt
)
target_include_directories(turnstile_test PRIVATE ${CMAKE_SOURCE_DIR}/turnstileEnum)

add_test(NAME TurnstileTest COMMAND turnstile_test)
