#pragma once

#include "ast.hpp"
#include <vector>
#include <string>
#include <stdexcept>

// A simple struct to represent a token from the lexer output
struct Token {
    std::string type;
    std::string value; // Only used for Id and Num
    size_t index;      // The token's position in the input stream
};

class Parser {
public:
    // Takes the vector of tokens from the lexer
    explicit Parser(std::vector<Token> tokens);

    // The main entry point to start parsing.
    // Returns the root of the AST, the Program node.
    std::unique_ptr<Program> parse();

private:
    std::vector<Token> m_tokens;
    size_t m_current_pos = 0;

    // --- Helper Methods ---

    // Checks if we've consumed all tokens.
    bool is_at_end() const;
    // Returns the current token without consuming it.
    const Token& peek() const;
    // Returns the previous token.
    const Token& previous() const;
    // Consumes and returns the current token, advancing the parser.
    Token advance();
    // Consumes the current token only if it matches the expected type.
    // Throws an error if it doesn't match.
    Token consume(const std::string& expected_type, const std::string& error_message);
    // Checks if the current token is of a given type.
    bool check(const std::string& type) const;
    // Checks if the current token is one of several types.
    bool check_any(const std::vector<std::string>& types) const;
    // Formats and throws a runtime error for the main function to catch.
    void error(const std::string& message) const;

    // --- Parsing Methods for Each Grammar Rule ---

    std::unique_ptr<Program> parse_program();
    std::unique_ptr<StructDef> parse_struct_def();
    std::unique_ptr<Decl> parse_extern_def();
    std::unique_ptr<FunctionDef> parse_function_def();
    std::unique_ptr<Decl> parse_decl();
    std::unique_ptr<Stmt> parse_let();

    // Statement Parsing
    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<Stmt> parse_if_stmt();
    std::unique_ptr<Stmt> parse_while_stmt();
    std::unique_ptr<Stmt> parse_return_stmt();
    std::vector<std::unique_ptr<Stmt>> parse_block();

    // Type Parsing
    std::unique_ptr<Type> parse_type();
    std::unique_ptr<Type> parse_funtype();

    // Expression Parsing (by precedence)
    std::unique_ptr<Exp> parse_exp();        // Precedence: ?: (Select)
    std::unique_ptr<Exp> parse_exp1();       // Precedence: or, and
    std::unique_ptr<Exp> parse_exp2();       // Precedence: ==, !=, <, <=, >, >=
    std::unique_ptr<Exp> parse_exp3();       // Precedence: +, -
    std::unique_ptr<Exp> parse_exp4();       // Precedence: *, /
    std::unique_ptr<Exp> parse_exp5();       // Precedence: Unary operators (-, not)
    std::unique_ptr<Exp> parse_exp6();       // Precedence: Call, Array/Struct Access
    std::unique_ptr<Exp> parse_exp7();       // Precedence: Primary (literals, id, grouping)
};