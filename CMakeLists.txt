cmake_minimum_required(VERSION 2.8)
project(test)

add_library(addr2line STATIC addr2line.c) 


set(CMAKE_C_FLAGS "-std=c99")
add_executable(test test.c)
target_link_libraries(test addr2line bfd iberty dl z)
set(CMAKE_BUILD_TYPE "Debug")

install(TARGETS addr2line DESTINATION lib)
install(FILES addr2line.h DESTINATION include)

# enable_testing()
# add_test(test_run test)