cmake_minimum_required(VERSION 3.8)
project(kdtreepp LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(CONAN_EXPORTED) # in conan local cache
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
else() # in user space
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake)
  conan_cmake_run(CONANFILE conanfile.py
                  BASIC_SETUP
				  CMAKE_TARGETS
				  BUILD missing)
endif()

find_package(Eigen3 REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(OPTIMIZATION_FLAGS "-DDEBUG")
  if(NOT MSVC)
    set(OPTIMIZATION_FLAGS "${OPTIMIZATION_FLAGS} -O0")
  endif()
  message("-- Configuring debug build")
else()
  set(OPTIMIZATION_FLAGS "-DNDEBUG -O2")
  message("-- Configuring release build")
endif()

if(MSVC)
  set(DESIRED_WARNINGS "-WX")
else()
  set(DESIRED_WARNINGS "-Wall -Wextra -Wconversion -Wunreachable-code -Wuninitialized -pedantic-errors -Wold-style-cast -Wno-error=unused-variable -Wshadow -Wfloat-equal")
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(DESIRED_WARNINGS "${DESIRED_WARNINGS} -Wmost")
  endif()
  if (WERROR)
    set(DESIRED_WARNINGS "${DESIRED_WARNINGS} -Werror")
  endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATION_FLAGS} ${DESIRED_WARNINGS}")

include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})

include_directories("${PROJECT_SOURCE_DIR}/include")

file(GLOB TEST_SOURCES test/*.cpp)
add_executable(unit-tests ${TEST_SOURCES})
target_link_libraries(unit-tests CONAN_PKG::catch2)

# libbenchmark.a supports threads and therefore needs pthread support
find_package(Threads REQUIRED)
file(GLOB BENCH_SOURCES bench/*.cpp)
add_executable(bench-tests ${BENCH_SOURCES})

# link benchmark static library to the bench-tests binary so the bench tests know where to find the benchmark impl code
target_link_libraries(bench-tests CONAN_PKG::benchmark ${CMAKE_THREAD_LIBS_INIT})
