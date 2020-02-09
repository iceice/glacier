# add_executable(date_test date_test.cc)
# target_link_libraries(date_test glacier_base)

# add_executable(timezone_test timezone_test.cc)
# target_link_libraries(timezone_test glacier_base)

# add_executable(timestamp_test timestamp_test.cc)
# target_link_libraries(timestamp_test glacier_base)

# add_executable(logstream_test logstream_test.cc)
# target_link_libraries(logstream_test glacier_base)

# add_executable(logging_test logging_test.cc)
# target_link_libraries(logging_test glacier_base)

# add_executable(logfile_test logfile_test.cc)
# target_link_libraries(logfile_test glacier_base)

add_executable(thread_pool_test thread_pool_test.cc)
target_link_libraries(thread_pool_test glacier_base)