include(FindProtobuf)
find_package(Protobuf REQUIRED)
set(PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/message.proto)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILE})

add_library(proto_lib ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(proto_lib PUBLIC ${Protobuf_INCLUDE_DIRS})
target_link_libraries(proto_lib PUBLIC protobuf::libprotobuf)
