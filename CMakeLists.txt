cmake_minimum_required(VERSION 2.8)

project(prog4)

set ( CMAKE_MODULE_PATH /usr/share/apps/cmake/modules )

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -O3 -fomit-frame-pointer -fno-exceptions -fPIC -Woverloaded-virtual -Wcast-qual -std=gnu++11" )

find_package(LLVM)

include_directories( /usr/include/llvm )
link_directories( /usr/lib/llvm )

include (bison.cmake)

add_flex_files(lexer.flex)
add_bison_files(parser.y)

set(CPPFILES
	main.cpp
	location.cpp
	expression.cpp
	bison_parser.cpp
	flex_lexer.cpp
)

add_executable(prog4 ${CPPFILES})

target_link_libraries(prog4 ${LLVM_JIT_LIBS} fl pthread ffi dl m)
