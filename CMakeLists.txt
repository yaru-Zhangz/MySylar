cmake_minimum_required(VERSION 3.10)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 禁用 GNU 扩展，确保标准 C++20

# 分开调试和发布配置
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wall -Wno-deprecated -Wno-unused-function")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# 仅对动态库添加 -rdynamic（如果需要）
set(CMAKE_SHARED_LINKER_FLAGS "-rdynamic")
# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-force_redefine")
set(CMAKE_C_COMPILER "/usr/local/gcc-13.2/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/local/gcc-13.2/bin/g++")

# include_directories(/usr/local/gcc-13.2/include/c++/13.2.0)
# include_directories(/usr/local/gcc-13.2/include/c++/13.2.0/x86_64-pc-linux-gnu)
# link_directories(/usr/local/gcc-13.2/lib64)

include_directories(.)
include_directories(/usr/local/include/yaml-cpp)
link_directories(/usr/local/lib64)

set(LIB_SRC
    sylar/src/log.cpp
    sylar/src/util.cpp
    sylar/src/config.cpp
    sylar/src/thread.cpp
    sylar/src/fiber.cpp)

add_library(sylar SHARED ${LIB_SRC})
target_include_directories(sylar PUBLIC ${PROJECT_SOURCE_DIR}/sylar/include)

#add_library(sylar_static STATIC ${LIB_SRC})
#force_redefine_file_macro_for_sources(sylar) #__FILE__
#SET_TARGET_PROPERTIES (sylar_static PROPETIES OUTPUT_NAME "sylar")

set(LIB_LIB
    sylar
    pthread
    yaml-cpp)

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log sylar)
target_link_libraries(test_log sylar ${LIB_LIB})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar ${LIB_LIB})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread sylar)
target_link_libraries(test_thread sylar ${LIB_LIB})

add_executable(test_util tests/test_util.cpp)
add_dependencies(test_util sylar)
target_link_libraries(test_util sylar ${LIB_LIB})

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber sylar)
target_link_libraries(test_fiber sylar ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)