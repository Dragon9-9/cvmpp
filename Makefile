# CVM++ — convenience targets (CMake is the canonical build)
CXX      ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
SRCS     := src/main.cpp src/lexer.cpp src/parser.cpp src/compiler.cpp \
            src/bytecode.cpp src/opcode.cpp src/vm.cpp src/value.cpp \
            src/ast.cpp src/ast_print.cpp src/compile.cpp src/diagnostic.cpp \
            src/ui.cpp src/token.cpp src/source_loc.cpp
TARGET   := build/cvmpp

.PHONY: all clean test repl verify cmake docs

all: $(TARGET)

$(TARGET): $(SRCS) | build
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)

build:
	mkdir -p build

cmake:
	cmake -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build

clean:
	rm -f $(TARGET)

verify: $(TARGET)
	./scripts/verify.sh

repl: $(TARGET)
	./$(TARGET)

docs:
	./scripts/build-docs.sh

test: verify
