# =============================================================================
# Makefile for Cflat Lexer and Parser
# =============================================================================

# 1. Configuration
# -----------------------------------------------------------------------------
# Use g++ as the C++ compiler
CXX = g++

# Compiler flags:
# -std=c++17 : Use the C++17 standard
# -Wall      : Enable all common warnings (good practice)
# -g         : Include debugging symbols in the executables
CXXFLAGS = -std=c++17 -Wall -g

# The final executable programs we want to build
EXECUTABLES = lex parse


# =============================================================================
# 2. Default Target
# -----------------------------------------------------------------------------
# The 'all' target is the default. Running 'make' will build all executables.
# .PHONY means 'all' is a command, not a file to be created.
.PHONY: all
all: $(EXECUTABLES)


# =============================================================================
# 3. Linking Rules (How to build the final executables)
# -----------------------------------------------------------------------------
# These rules tell make how to combine the compiled object files (.o)
# into the final executables.

# Rule to link the 'lex' executable
lex: lex_main.o lexer.o
	$(CXX) $(CXXFLAGS) -o lex lex_main.o lexer.o

# Rule to link the 'parse' executable
# Note: The parser depends on the lexer's object code.
parse: parse_main.o parser.o lexer.o
	$(CXX) $(CXXFLAGS) -o parse parse_main.o parser.o lexer.o


# =============================================================================
# 4. Compilation Rules (How to build object files)
# -----------------------------------------------------------------------------
# These rules tell make how to compile individual .cpp source files into
# .o object files. Make is smart enough to re-compile only what has changed.

# An object file depends on its .cpp file and any .hpp files it includes.
lex_main.o: lex_main.cpp lexer.hpp
	$(CXX) $(CXXFLAGS) -c lex_main.cpp

parse_main.o: parse_main.cpp parser.hpp lexer.hpp
	$(CXX) $(CXXFLAGS) -c parse_main.cpp

parser.o: parser.cpp parser.hpp lexer.hpp
	$(CXX) $(CXXFLAGS) -c parser.cpp

lexer.o: lexer.cpp lexer.hpp
	$(CXX) $(CXXFLAGS) -c lexer.cpp


# =============================================================================
# 5. Cleanup Rule
# -----------------------------------------------------------------------------
# The 'clean' target removes all generated files (executables and objects).
# This is useful for starting a fresh build.
.PHONY: clean
clean:
	rm -f $(EXECUTABLES) *.o
