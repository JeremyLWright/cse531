cmake_minimum_required(VERSION 2.8)
if(UNIX)
set(CMAKE_CXX_FLAGS "-Wall -Werror -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-pg -g3 -ggdb -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS "-Wall -Werror")
set(CMAKE_C_FLAGS_DEBUG "-pg -g3 -ggdb -fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS_RELEASE "-O3")
else()
    set(CMAKE_CXX_FLAGS "/W4 /EHsc /MT ")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /D_ITERATOR_DEBUG_LEVEL=1")
set(CMAKE_CXX_FLAGS_RELEASE "-Ox")
set(CMAKE_C_FLAGS "/W4 /MT")
set(CMAKE_C_FLAGS_DEBUG "/Od /D_ITERATOR_DEBUG_LEVEL=1")
set(CMAKE_C_FLAGS_RELEASE "/Ox")
endif(UNIX)
include_directories(.)
include_directories(threads)

add_executable(thread_test thread_test.c)
add_executable(example examples/example.c)
add_executable(read_write_s examples/read_write_s.c)

add_subdirectory(p-threads)

include(ExternalProject)
# Add gtest
ExternalProject_Add(
googletest
URL https://googletest.googlecode.com/files/gtest-1.7.0.zip
TIMEOUT 10
CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
# Disable install step
INSTALL_COMMAND ""
)

ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)
ExternalProject_Get_Property(googletest binary_dir)
link_directories(${binary_dir})
file(GLOB test_srcs "tests/*.test.cpp")
add_executable(tests ${test_srcs})
add_dependencies(tests googletest)
if(UNIX)
target_link_libraries(tests
gtest_main
gtest
pthread)
else()
target_link_libraries(tests
gtest_main
gtest)
endif()
enable_testing()
add_test(googletest tests)

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_NAME "wrightj_welchm.zip")



INCLUDE(CPack)

