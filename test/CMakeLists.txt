enable_testing()
find_package(GTest REQUIRED)

add_executable(turnstile_test turnstile_test.cpp)
target_link_libraries(turnstile_test GTest::GTest GTest::Main turnstile)
target_include_directories(turnstile_test PRIVATE ${CMAKE_SOURCE_DIR})

add_test(NAME TurnstileTest COMMAND turnstile_test)
