cmake_minimum_required(VERSION 3.10)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  

set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall -Wno-deprecated -Wno-unused-function")

set(CMAKE_SHARED_LINKER_FLAGS "-rdynamic")
set(CMAKE_C_COMPILER "/usr/local/gcc-13.2/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/gcc-13.2/bin/g++")

include_directories(.)
include_directories(/usr/local/include/yaml-cpp)
link_directories(/usr/local/lib64)

set(LIB_SRC
    sylar/src/log.cpp
    sylar/src/util.cpp
    sylar/src/config.cpp
    sylar/src/thread.cpp
    sylar/src/fiber.cpp
    sylar/src/scheduler.cpp
    sylar/src/iomanager.cpp
    sylar/src/timer.cpp
    sylar/src/hook.cpp
    sylar/src/fd_manager.cpp
    sylar/src/address.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
target_include_directories(sylar PUBLIC ${PROJECT_SOURCE_DIR}/sylar/include)

set(LIB_LIB sylar pthread yaml-cpp dl)

set(TESTS
    test_log
    test_config
    test_thread
    test_util
    test_fiber
    test_scheduler
    test_iomanager
    test_hook
    test_address
)

foreach(test ${TESTS})
    add_executable(${test} tests/${test}.cpp)
    target_link_libraries(${test} sylar ${LIB_LIB})
endforeach()

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)