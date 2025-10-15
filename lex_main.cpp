#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexer.hpp"

// Helper function to get string representation of a TokenType
std::string token_type_to_string(const Token& token) {
    std::string lexeme(token.first, token.last - token.first);
    switch (token.token_type) {
        // case TokenType::Error: return "Error(" + lexeme + ")";
        case TokenType::Error: {
            // Check if lexeme ends with newline, if so add extra newline before closing paren
            if (!lexeme.empty() && lexeme.back() == '\n') {
                return "Error(" + lexeme + "\n)";
            }
            return "Error(" + lexeme + ")";
        }
        case TokenType::Num: return "Num(" + lexeme + ")";
        case TokenType::Id: return "Id(" + lexeme + ")";

        // Keywords
        case TokenType::Int:       return "Int";
        case TokenType::Struct:    return "Struct";
        case TokenType::Nil:       return "Nil";
        case TokenType::Break:     return "Break";
        case TokenType::Continue:  return "Continue";
        case TokenType::Return:    return "Return";
        case TokenType::If:        return "If";
        case TokenType::Else:      return "Else";
        case TokenType::While:     return "While";
        case TokenType::New:       return "New";
        case TokenType::Let:       return "Let";
        case TokenType::Extern:    return "Extern";
        case TokenType::Fn:        return "Fn";
        case TokenType::And:       return "And";
        case TokenType::Or:        return "Or";
        case TokenType::Not:       return "Not";

        // Punctuation and Operators
        case TokenType::Colon:        return "Colon";
        case TokenType::Semicolon:    return "Semicolon";
        case TokenType::Comma:        return "Comma";
        case TokenType::Arrow:        return "Arrow";
        case TokenType::Ampersand:    return "Ampersand";
        case TokenType::Plus:         return "Plus";
        case TokenType::Dash:         return "Dash";
        case TokenType::Star:         return "Star";
        case TokenType::Slash:        return "Slash";
        case TokenType::Equal:        return "Equal";
        case TokenType::NotEq:        return "NotEq";
        case TokenType::Lt:           return "Lt";
        case TokenType::Lte:          return "Lte";
        case TokenType::Gt:           return "Gt";
        case TokenType::Gte:          return "Gte";
        case TokenType::Dot:          return "Dot";
        case TokenType::Gets:         return "Gets";
        case TokenType::OpenParen:    return "OpenParen";
        case TokenType::CloseParen:   return "CloseParen";
        case TokenType::OpenBracket:  return "OpenBracket";
        case TokenType::CloseBracket: return "CloseBracket";
        case TokenType::OpenBrace:    return "OpenBrace";
        case TokenType::CloseBrace:   return "CloseBrace";
        case TokenType::QuestionMark: return "QuestionMark";
    }
    return "Unknown";
}


int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input-file>" << std::endl;
        return 1;
    }

    std::ifstream input_file(argv[1]);
    if(!input_file) {
        std::cerr << "Could not open file: " << argv[1] << std::endl;
        return 1;
    }

    // Read the entire file into a string
    std::stringstream buffer;
    buffer << input_file.rdbuf();
    std::string source_code = buffer.str();

    // Lex the source code
    const char* first = source_code.c_str();
    const char* last = first + source_code.length();

    std::vector<Token> tokens = lex(first, last);

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        std::cout << token_type_to_string(token);
        if (i != tokens.size() - 1) {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
    // std::cout << std::endl;

    return 0;
}