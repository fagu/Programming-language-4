cmake_minimum_required(VERSION 2.8)

project(prog4)

set ( CMAKE_MODULE_PATH /usr/share/apps/cmake/modules )

find_package(LLVM)

include_directories( /usr/include/llvm )
link_directories( /usr/lib/llvm )

add_executable(prog4 main.cpp)

target_link_libraries(prog4 ${LLVM_JIT_LIBS})
