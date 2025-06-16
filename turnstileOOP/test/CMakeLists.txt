enable_testing()
find_package(GTest REQUIRED)
find_package(fmt REQUIRED)

add_executable(turnstileOOP_test
    turnstileOOPTest.cpp
    ${CMAKE_SOURCE_DIR}/turnstileOOP/turnstileOOP.cpp
)
target_link_libraries(turnstileOOP_test
    GTest::GTest 
    GTest::Main    
    fmt::fmt
)
target_include_directories(turnstileOOP_test PRIVATE ${CMAKE_SOURCE_DIR}/turnstileOOP)
add_test(NAME TurnstileOOPTest COMMAND turnstileOOP_test)

