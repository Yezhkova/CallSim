find_package(fmt REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_executable(turnstileApp turnstileApp.cpp)
target_link_libraries(turnstileApp PUBLIC fmt::fmt magic_enum::magic_enum)
target_include_directories(turnstileApp PUBLIC
    ../magic_enum/include/magic_enum
)