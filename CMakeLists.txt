cmake_minimum_required(VERSION 3.5)

# set the project name
project(glacier)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
    -g           # Turn on debugging (so GDB gives more friendly output).
    -Wall        # Turn on most warnings.
    -Wextra      # Enable extra flags not enabled by -Wall
    -Werror      # Make all warnings into errors.
    -Wconversion # Do warn about implicit conversions.
    -Wno-unused-parameter # Do not warn if a parameter is unused.
    -Wold-style-cast      # Warn if an old-style (C-style) cast to a non-void type
    -Woverloaded-virtual  # Warn when a function declaration hides virtual functions from a base class
    -Wpointer-arith       # Warn about anything that depends on the “size of” a function type or of void.
    -Wshadow              # Warn whenever a local variable or type declaration shadows another variable
    -Wwrite-strings       # Warn about the deprecated conversion from string literals to char *.
    -std=c++11            # ISO C++11
)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(base)
add_subdirectory(src)
add_subdirectory(tests)