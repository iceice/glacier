include_directories(${PROJECT_SOURCE_DIR})

add_executable(server_test server_test.cc)
target_link_libraries(server_test glacier)