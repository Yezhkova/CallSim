find_package(fmt REQUIRED)

add_executable(turnstileOOPApp 
    turnstileOOP.cpp
    turnstileOOP.h
    turnstileOOPApp.cpp
)
target_link_libraries(turnstileOOPApp PUBLIC fmt::fmt)
target_include_directories(turnstileOOPApp PUBLIC
    ../magic_enum/include/magic_enum
)
