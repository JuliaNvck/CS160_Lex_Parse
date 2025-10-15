# =============================================================================
# Makefile for Cflat Lexer and Parser (Revised)
# =============================================================================

# 1. Configuration
CXX = g++
# ADDED -fsanitize=address for memory safety!
CXXFLAGS = -std=c++17 -Wall -g -fsanitize=address
EXECUTABLES = lex parse

# Define object files for each executable
LEX_OBJS = lex_main.o lexer.o
PARSE_OBJS = parse_main.o parser.o

# 2. Default Target
.PHONY: all
all: $(EXECUTABLES)

# 3. Linking Rules (Simplified using automatic variables)
lex: $(LEX_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# REMOVED lexer.o dependency
parse: $(PARSE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 4. Compilation Rule (Replaced with a single pattern rule)
# This generic rule tells make how to build any .o from a .cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5. Dependencies
# Explicitly list header dependencies here.
# If a header changes, the corresponding .o file will be rebuilt.
lex_main.o: lexer.hpp
parse_main.o: parser.hpp ast.hpp
parser.o: parser.hpp ast.hpp
lexer.o: lexer.hpp

# 6. Cleanup Rule
.PHONY: clean
clean:
	rm -f $(EXECUTABLES) *.o