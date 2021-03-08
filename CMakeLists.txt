cmake_minimum_required(VERSION 3.0)

project (paracl)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} -O2 -fstack-usage")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMON_CXX_FLAGS} -g -O0")

set(SRC_LIST inode.cc ast.cc driver.cc)

find_package(BISON)
BISON_TARGET(Parser grammar.yy ${CMAKE_CURRENT_BINARY_DIR}/grammar.tab.cc VERBOSE COMPILE_FLAGS "-Wall -rall")

find_package(FLEX)
FLEX_TARGET(Scanner lexer.ll ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
include_directories(${PROJECT_BINARY_DIR} ".")

add_executable(driver.out ${SRC_LIST} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
