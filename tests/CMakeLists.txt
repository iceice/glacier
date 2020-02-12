include_directories(${PROJECT_SOURCE_DIR})

add_executable(inet_test inet_test.cc)
target_link_libraries(inet_test glacier)