cmake_minimum_required(VERSION 3.16)
project(cvmpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cvmpp
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/bytecode.cpp
    src/opcode.cpp
    src/vm.cpp
    src/value.cpp
    src/ast.cpp
    src/ast_print.cpp
    src/compile.cpp
    src/diagnostic.cpp
    src/ui.cpp
    src/token.cpp
    src/source_loc.cpp
)

target_include_directories(cvmpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(MSVC)
    target_compile_options(cvmpp PRIVATE /W4 /permissive-)
else()
    target_compile_options(cvmpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
