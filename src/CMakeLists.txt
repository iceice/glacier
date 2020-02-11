aux_source_directory(. DIR_SRCS)
include_directories(${PROJECT_SOURCE_DIR})

add_library(glacier ${DIR_SRCS})
target_link_libraries(glacier glacier_base)
