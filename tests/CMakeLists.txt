include_directories(${PROJECT_SOURCE_DIR})

add_executable(acceptor_test acceptor_test.cc)
target_link_libraries(acceptor_test glacier)