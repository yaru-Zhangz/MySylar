cmake_minimum_required(VERSION 3.10)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    sylar/src/log.cpp
    sylar/src/util.cpp)

add_library(sylar SHARED ${LIB_SRC})
target_include_directories(sylar PUBLIC ${PROJECT_SOURCE_DIR}/sylar/include)

#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPETIES OUTPUT_NAME "sylar")

add_executable(test tests/test.cpp)
add_dependencies(test sylar)
target_link_libraries(test sylar)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)