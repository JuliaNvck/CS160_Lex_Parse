# Makefile for Cflat Lexer and Parser

# Configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -fsanitize=address
EXECUTABLES = lex parse

# Define object files for each executable
LEX_OBJS = lex_main.o lexer.o
PARSE_OBJS = parse_main.o parser.o

# Default Target
.PHONY: all
all: $(EXECUTABLES)

# Linking Rules
lex: $(LEX_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

parse: $(PARSE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation Rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Dependencies
lex_main.o: lexer.hpp
parse_main.o: parser.hpp ast.hpp
parser.o: parser.hpp ast.hpp
lexer.o: lexer.hpp

# Cleanup Rule
.PHONY: clean
clean:
	rm -f $(EXECUTABLES) *.o