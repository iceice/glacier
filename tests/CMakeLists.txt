include_directories(${PROJECT_SOURCE_DIR})

add_executable(timer_queue_test timer_queue_test.cc)
target_link_libraries(timer_queue_test glacier)