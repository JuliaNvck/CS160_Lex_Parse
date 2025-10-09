#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <vector>
#include <string>

/**
 * An Enum to represent each token we have in our language.
 *
 * Note: `enum class` makes each enumerator require you to write `TokenType::`
 * in order to use it. (It just helps keep your global namespace clean.)
 */
enum class TokenType {
    Error,
    Num,
    Id,
    Int,
    Struct,
    Nil,
    Break,
    Continue,
    Return,
    If,
    Else,
    While,
    New,
    Let,
    Extern,
    Fn,
    And,
    Or,
    Not,
    Colon,
    Semicolon,
    Comma,
    Arrow,
    Ampersand,
    Plus,
    Dash,
    Star,
    Slash,
    Equal,
    NotEq,
    Lt,
    Lte,
    Gt,
    Gte,
    Dot,
    Gets,
    OpenParen,
    CloseParen,
    OpenBracket,
    CloseBracket,
    OpenBrace,
    CloseBrace,
    QuestionMark,
};

/**
 * A token
 */
struct Token {
    TokenType token_type;
    const char* first;  // the first character of the token
    const char* last;  // one past the last character of the token
};

// Note: there are many ways to represent a Token with different "variants".
// The implementation above is something you might see in a C program.
// In C++, you can use class inheritance and polymorphism, `union`s, or
// `std::variant`s to accomplish the same goal.


std::vector<Token> lex(const char* first, const char* last);


Token munch_token(const char* first, const char* last);
#endif
