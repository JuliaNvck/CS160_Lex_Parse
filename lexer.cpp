#include <cstring>
#include <cctype>
#include <optional>
#include <utility> 

#include "lexer.hpp"


static bool substr_eq(const char* first, const char* last, const char* pattern);
static std::pair<const char*, std::optional<Token>> skip_whitespace_and_comments(const char* first, const char* last);
static const char* identifier_end(const char* first, const char* last);
static const char* numeric_end(const char* first, const char* last);
static const char* error_end(const char* first, const char* last);

// helpers
static bool starts_two_char_token(const char* p, const char* last) {
    if (p + 1 >= last) return false;
    return (p[0] == '!' && p[1] == '=') ||
           (p[0] == '<' && p[1] == '=') ||
           (p[0] == '>' && p[1] == '=') ||
           (p[0] == '-' && p[1] == '>') ||
           (p[0] == '=' && p[1] == '=') ||
           // comment starters (so we stop before the skipper handles them)
           (p[0] == '/' && (p[1] == '/' || p[1] == '*'));
}

static bool starts_one_char_token(char c) {
    switch (c) {
        case ':': case ';': case ',': case '&': case '+': case '-':
        case '*': case '/': case '<': case '>': case '.': case '=':
        case '(': case ')': case '[': case ']': case '{': case '}':
        case '?':
            return true;
        default:
            return false;
    }
}

static bool starts_token(const char* p, const char* last) {
    if (p >= last) return false;
    if (std::isalpha(static_cast<unsigned char>(*p))) return true; // Id/keyword
    if (std::isdigit(static_cast<unsigned char>(*p))) return true; // Num
    if (starts_two_char_token(p, last)) return true;
    if (starts_one_char_token(*p)) return true;
    return false;
}



/**
 * The entry point of our lexer
 *
 * `first` is a pointer to the first character of the source code string.
 * `last` is a pointer to one-past-the-end of the source code.
 */
std::vector<Token> lex(const char* first, const char* last) {
    std::vector<Token> tokens{};

    const char* curr = first;
    while(curr != last) {
        auto [next_char, opt_error_token] = skip_whitespace_and_comments(curr, last);
        curr = next_char;
        // Check if the skipper returned an error token.
        if (opt_error_token) {
            // If it did, add it to our list of tokens.
            tokens.push_back(*opt_error_token);
            // An unclosed comment error consumes the rest of the file, so we stop.
            break;
        }
        // If we're at the end of the file after skipping, we're done.
        if (curr == last) {
            break;
        }
        Token tok = munch_token(curr, last);
        tokens.push_back(tok);

        curr = tok.last;
    }
    return tokens;
}

/**
 * Lex one token from the source code.
 * The function will try to lex a token beginning at `first`.
 */
Token munch_token(const char* first, const char* last) {
    // Return a single Token.
    // If we haven't returned, we have not yet found a token.

    if(first == last) {
        return Token{TokenType::Error, first, last};
    }

    // Try to munch an identifier or keyword

    if(std::isalpha(static_cast<unsigned char>(*first))) {
        const char* id_end = identifier_end(first, last);
        // Prioritize keywords
        if(substr_eq(first, id_end, "int")) return Token{TokenType::Int, first, id_end};
        if(substr_eq(first, id_end, "struct")) return Token{TokenType::Struct, first, id_end};
        if(substr_eq(first, id_end, "nil")) return Token{TokenType::Nil, first, id_end};
        if(substr_eq(first, id_end, "break")) return Token{TokenType::Break, first, id_end};
        if(substr_eq(first, id_end, "continue")) return Token{TokenType::Continue, first, id_end};
        if(substr_eq(first, id_end, "return")) return Token{TokenType::Return, first, id_end};
        if(substr_eq(first, id_end, "if")) return Token{TokenType::If, first, id_end};
        if(substr_eq(first, id_end, "else")) return Token{TokenType::Else, first, id_end};
        if(substr_eq(first, id_end, "while")) return Token{TokenType::While, first, id_end};
        if(substr_eq(first, id_end, "new")) return Token{TokenType::New, first, id_end};
        if(substr_eq(first, id_end, "let")) return Token{TokenType::Let, first, id_end};
        if(substr_eq(first, id_end, "extern")) return Token{TokenType::Extern, first, id_end};
        if(substr_eq(first, id_end, "fn")) return Token{TokenType::Fn, first, id_end};
        if(substr_eq(first, id_end, "and")) return Token{TokenType::And, first, id_end};
        if(substr_eq(first, id_end, "or")) return Token{TokenType::Or, first, id_end};
        if(substr_eq(first, id_end, "not")) return Token{TokenType::Not, first, id_end};
        // If not a keyword, it's an identifier
        return Token{TokenType::Id, first, id_end};
    }

    // Try to munch a number
    if(std::isdigit(static_cast<unsigned char>(*first))) {
        const char* num_end = numeric_end(first, last);
        return Token{TokenType::Num, first, num_end};
    }

    // Try to munch operators and punctuation
    // Check longest operators first for max munch
    if (last - first >= 2) {
        if (substr_eq(first, first + 2, "!=")) {
            return Token{TokenType::NotEq, first, first + 2};
        }
        if (substr_eq(first, first + 2, "<=")) {
            return Token{TokenType::Lte, first, first + 2};
        }
        if (substr_eq(first, first + 2, ">=")) {
            return Token{TokenType::Gte, first, first + 2};
        }
        if (substr_eq(first, first + 2, "->")) {
            return Token{TokenType::Arrow, first, first + 2};
        }
        if (substr_eq(first, first + 2, "==")) {
            return Token{TokenType::Equal, first, first + 2};
        }
        // C-style comments
        // if (substr_eq(first, first + 2, "/*")) {
        //     // not skipped by skip_whitespace_and_comments so unterminated c-style comment is an error
        //     return Token{TokenType::Error, first, last};
        // }
    }
    switch(*first) {
        case ':': return {TokenType::Colon, first, first + 1};
        case ';': return {TokenType::Semicolon, first, first + 1};
        case ',': return {TokenType::Comma, first, first + 1};
        case '&': return {TokenType::Ampersand, first, first + 1};
        case '+': return {TokenType::Plus, first, first + 1};
        case '-': return {TokenType::Dash, first, first + 1};
        case '*': return {TokenType::Star, first, first + 1};
        case '/': return {TokenType::Slash, first, first + 1};
        case '<': return {TokenType::Lt, first, first + 1};
        case '>': return {TokenType::Gt, first, first + 1};
        case '.': return {TokenType::Dot, first, first + 1};
        case '=': return {TokenType::Gets, first, first + 1};
        case '(': return {TokenType::OpenParen, first, first + 1};
        case ')': return {TokenType::CloseParen, first, first + 1};
        case '[': return {TokenType::OpenBracket, first, first + 1};
        case ']': return {TokenType::CloseBracket, first, first + 1};
        case '{': return {TokenType::OpenBrace, first, first + 1};
        case '}': return {TokenType::CloseBrace, first, first + 1};
        case '?': return {TokenType::QuestionMark, first, first + 1};
    }

    // Nothing matched, so we have an error token.
    // We will consume characters until we reach whitespace or a character
    const char* err_end = error_end(first, last);
    return Token{TokenType::Error, first, err_end};
}

/**
 * Look for the next character which is either a space or the beginning of the
 * next (possibly) valid token.
 */
// const char* error_end(const char* first, const char* last) {
//     // find the end of the error
//     const char* err_end = first + 1;
//     for(; err_end != last; ++err_end) {
//         // find next space or possible start of token
//         if(' ' == *err_end || isalpha(*err_end) || '=' == *err_end || '+' == *err_end || ';' == *err_end) {
//             break;
//         }
//     }
//     return err_end;
// }
// Consume a maximal run of non-token-start characters, INCLUDING whitespace,
// stopping right before the next valid token (or end of input).
static const char* error_end(const char* first, const char* last) {
    const char* it = first;
    // Always consume at least one char
    if (it < last) ++it;

    while (it < last) {
        if (starts_token(it, last)) break;   // stop BEFORE next token
        ++it;                                 // keep absorbing (incl. spaces/newlines)
    }
    return it; // one-past-end of Error(...) lexeme
}



/**
 * Return one-past-the-end of an identifier sequence of characters: [a-zA-Z]([a-zA-Z0-9_])⋆
 */
const char* identifier_end(const char* first, const char* last) {
    if (first == last || !(std::isalpha(static_cast<unsigned char>(*first)))) {
        return first; // not a valid identifier start
    }
    for(const char* it = first; it != last; ++it) {
        if(!(std::isalnum(static_cast<unsigned char>(*it)) || '_' == *it)) {
            return it;
        }
    }
    return last;
}

/**
 * Return one-past-the-end of a numeric sequence of characters: [0-9]+: [0-9]([0-9])⋆
 */
const char* numeric_end(const char* first, const char* last) {
    if (first == last || !std::isdigit(static_cast<unsigned char>(*first))) {
        return first; // not a valid numeric start
    }
    for(const char* it = first; it != last; ++it) {
        if(!(std::isdigit(static_cast<unsigned char>(*it)))) {
            return it;
        }
    }
    return last;
}

/** *
 * Skip whitespace and comments
 */
// std::pair<const char*, std::optional<Token>> skip_whitespace_and_comments(const char* first, const char* last) {
//     const char* it = first;
//     while (true) {
//         const char* start_loop = it;
//         while (it != last && isspace(*it)) {
//             ++it;
//         }

//         if (it + 1 < last && *it == '/' && *(it + 1) == '/') {
//             // Single-line c++-comment
//             it += 2;
//             while (it != last && *it != '\n') {
//                 ++it;
//             }
//             continue; // Continue to skip more whitespace/comments
//         } else if (it + 1 < last && *it == '/' && *(it + 1) == '*') {
//             // Multi-line comment
//             const char* start_comment = it;
//             it += 2;
//             while (it + 1 < last && !(*it == '*' && *(it + 1) == '/')) {
//                 ++it;
//             }
//             if (it + 1 < last) {
//                 it += 2; // found the closing */
//             } else {
//                 Token err_tok = Token{TokenType::Error, start_comment, last}; // Unterminated comment
//                 return {last, err_tok};
//             }
//             continue; // Continue to skip more whitespace/comments
//         } else {
//             break; // No more whitespace or comments
//         }

//         if (it == start_loop) {
//             break; // No progress made, exit loop
//         }
//     }
//     return {it, std::nullopt};
// }

std::pair<const char*, std::optional<Token>>
skip_whitespace_and_comments(const char* first, const char* last) {
    const char* it = first;

    for (;;) {
        // 1) whitespace
        while (it != last && std::isspace(static_cast<unsigned char>(*it))) {
            ++it;
        }

        // 2) C++-style // comment  -> consume until newline OR EOF
        if (it + 1 < last && it[0] == '/' && it[1] == '/') {
            const char* start_comment = it;    // for Error token if needed
            it += 2;                           // skip //
            while (it != last && *it != '\n') {
                ++it;
            } // eat until newline
            if (it == last) {
                Token err_tok{TokenType::Error, start_comment, last};
                return {last, err_tok};
            }
            else if (it != last && *it == '\n')     // eat the newline itself
                ++it;
            continue;                          // loop to skip more ws/comments
        }

        // 3) C-style /* ... */ comment  -> error if unterminated
        if (it + 1 < last && it[0] == '/' && it[1] == '*') {
            const char* start_comment = it;    // for Error token if needed
            it += 2;
            bool closed = false;
            while (it + 1 < last) {
                if (it[0] == '*' && it[1] == '/') {
                    it += 2;
                    closed = true;
                    break;
                }
                ++it;
            }
            if (!closed) {
                // Unterminated comment is a lexer error: Error("/*...<EOF>")
                Token err_tok{TokenType::Error, start_comment, last};
                return {last, err_tok};
            }
            continue;                          // there may be more ws/comments
        }

        // nothing more to skip
        break;
    }

    return {it, std::nullopt};
}

/**
 * Check if a string pointed to by `first` and `last`
 * is equal to a null-terminated string.
 */
bool substr_eq(const char* first, const char* last, const char* pattern) {
    const char* pattern_it = pattern;
    for(const char* it = first; it != last; ++it) {
        if('\0' == *pattern_it) {
            return false;  // we hit end of pattern before finishing substr.
        }

        if(*it != *pattern_it) {
            return false;
        }

        ++pattern_it;
    }
    // return false if pattern is longer than the string.
    return '\0' == *pattern_it;
}
