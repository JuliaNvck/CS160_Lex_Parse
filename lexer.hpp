#ifndef LEXER_HPP_
#define LEXER_HPP_

#include <vector>
#include <string>

/**
 * An Enum to represent each token we have in our language.
 *
 * Note: `enum class` makes each enumerator require you to write `TokenType::`
 * in order to use it.
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

std::vector<Token> lex(const char* first, const char* last);


Token munch_token(const char* first, const char* last);
#endif
