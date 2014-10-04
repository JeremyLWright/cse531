cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "-Wall -Werror -std=gnu++0x")
set(CMAKE_CXX_FLAGS_DEBUG "-pg -g3 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
include_directories(.)

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
# Specify include dir
ExternalProject_Get_Property(googletest source_dir)
include_directories(${source_dir}/include)
ExternalProject_Get_Property(googletest binary_dir)
link_directories(${binary_dir})
file(GLOB test_srcs "*.test.cpp")
add_executable(tests ${test_srcs})
add_dependencies(tests googletest)
target_link_libraries(tests
gtest_main
gtest
pthread)
enable_testing()
add_test(cse531_tests tests)

SET(CPACK_GENERATOR "ZIP")
SET(CPACK_PACKAGE_NAME "wrightj_welchm.zip")



INCLUDE(CPack)

