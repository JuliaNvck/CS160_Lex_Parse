#include "parser.hpp"

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<Program> Parser::parse() {
    return parse_program();
}

// --- Main Parsing Logic ---

// program ::= (struct | extern | function)+
std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    
    // Grammar requires at least one (struct | extern | function)
    if (is_at_end()) {
        error("unexpected end of token stream");
    }
    
    while (!is_at_end()) {
        if (check("Struct")) {
            program->structs.push_back(parse_struct_def());
        } else if (check("Extern")) {
            program->externs.push_back(parse_extern_def());
        } else if (check("Fn")) {
            program->functions.push_back(parse_function_def());
        } else {
            error("unexpected token at token " + std::to_string(peek().index));
        }
    }
    return program;
}

// function ::= `fn` id `(` LIST(decl) `)` `->` type `{` let⋆ stmt⋆ `}`
std::unique_ptr<FunctionDef> Parser::parse_function_def() {
    consume("Fn", "unexpected token at token " + std::to_string(peek().index));
    Token name = consume("Id", "unexpected token at token " + std::to_string(peek().index));

    auto func = std::make_unique<FunctionDef>();
    func->name = name.value;

    consume("OpenParen", "unexpected token at token " + std::to_string(peek().index));
    // Parse LIST(decl) for vector of parameters (decls)
    if (!check("CloseParen")) { // skip list if no params
        do {
            func->params.push_back(parse_decl());
        } while (check("Comma") && (advance(), true)); // Consume comma and continue
    }
    consume("CloseParen", "unexpected token at token " + std::to_string(peek().index));

    consume("Arrow", "unexpected token at token " + std::to_string(peek().index));
    func->rettype = parse_type();

    consume("OpenBrace", "unexpected token at token " + std::to_string(peek().index));

    // Parse local `let` declarations
    // let ::= `let` LIST(decl) `;`
    while (check("Let")) {
        consume("Let", "unexpected token at token " + std::to_string(peek().index));
        if (!check("Semicolon")) { // skip list if no locals
            do {
                func->locals.push_back(parse_decl());
            } while (check("Comma") && (advance(), true)); // Consume comma and continue
        }
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
    }

    // Parse statements in the body
    while (!check("CloseBrace") && !is_at_end()) {
        func->stmts.push_back(parse_stmt());
    }

    consume("CloseBrace", "unexpected token at token " + std::to_string(peek().index));
    return func;
}

// decl ::= id `:` type
std::unique_ptr<Decl> Parser::parse_decl() {
    Token name = consume("Id", "unexpected token at token " + std::to_string(peek().index));
    consume("Colon", "unexpected token at token " + std::to_string(peek().index));
    auto type = parse_type();
    return std::make_unique<Decl>(name.value, std::move(type));
}

// stmt ::= exp (`=` exp)? `;` | `if`... | `while`... | `break`... | `continue`... | `return`...
std::unique_ptr<Stmt> Parser::parse_stmt() {
    if (check("If")) return parse_if_stmt();
    if (check("While")) return parse_while_stmt();
    if (check("Return")) return parse_return_stmt();

    if (check("Break")) {
        advance();
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
        return std::make_unique<Break>();
    }
    if (check("Continue")) {
        advance();
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
        return std::make_unique<Continue>();
    }

    // exp (`=` exp)? `;`
    // The left-hand side of an assignment must be a Place.
    size_t start_token_index = peek().index;
    auto left_exp = parse_exp();

    if (check("Gets")) { // Assignment: exp = exp;
        advance(); // consume '='
        auto right_exp = parse_exp();
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));

        if (auto val = dynamic_cast<Val*>(left_exp.get())) {
            std::unique_ptr<Place> place_ptr = std::move(val->place);
            return std::make_unique<Assign>(std::move(place_ptr), std::move(right_exp));
        } else {
            error("left-hand side of assignment must be a place, starting at token " + std::to_string(start_token_index));
        }
    } else { // Standalone expression: exp;
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
        if (auto call_exp = dynamic_cast<CallExp*>(left_exp.get())) {
            std::unique_ptr<FunCall> fc = std::move(call_exp->fun_call);
            return std::make_unique<CallStmt>(std::move(fc));
        } else {
            error("standalone expressions must be function calls, starting at token " + std::to_string(start_token_index));
        }
    }
    // Unreachable: error(...) throws
        return nullptr;
}

// `if` exp block (`else` block)?
std::unique_ptr<Stmt> Parser::parse_if_stmt() {
    consume("If", "unexpected token at token " + std::to_string(peek().index));
    auto guard = parse_exp();
    std::vector<std::unique_ptr<Stmt>> tt = parse_block();
    std::vector<std::unique_ptr<Stmt>> ff;
    if (check("Else")) {
        advance(); // consume 'else'
    ff = parse_block();
    }
    return std::make_unique<If>(std::move(guard), std::move(tt), std::move(ff));
}

// block ::= `{` stmt⋆ `}`
std::vector<std::unique_ptr<Stmt>> Parser::parse_block() {
    consume("OpenBrace", "unexpected token at token " + std::to_string(peek().index));
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!check("CloseBrace") && !is_at_end()) {
        stmts.push_back(parse_stmt());
    }
    consume("CloseBrace", "unexpected token at token " + std::to_string(peek().index));
    return stmts;
}

// `while` exp block
std::unique_ptr<Stmt> Parser::parse_while_stmt() {
    consume("While", "unexpected token at token " + std::to_string(peek().index));
    auto guard = parse_exp();
    auto body = parse_block();
    return std::make_unique<While>(std::move(guard), std::move(body));
}

// `return` exp `;`
std::unique_ptr<Stmt> Parser::parse_return_stmt() {
    consume("Return", "unexpected token at token " + std::to_string(peek().index));
    auto exp = parse_exp();
    consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
    return std::make_unique<Return>(std::move(exp));
}

// --- Expression Parsing ---
// exp  ::= exp1 (`?` exp `:` exp1)⋆
// exp1 ::= exp2 ([`and`,`or`] exp2)⋆
// exp2 ::= exp3 ([`==`,`!=`,`<`,`<=`,`>`,`>=`] exp3)⋆
// exp3 ::= exp4 ([`+`,`-`] exp4)⋆
// exp4 ::= exp5 ([`*`,`/`] exp5)⋆
// exp5 ::= unop⋆ exp6
// exp6 ::= exp7 call_or_access⋆
// exp7 ::= id
//        | num
//        | `nil`
//        | `new` type
//        | `[` type `;` exp `]`
//        | `(` exp `)`

// exp  ::= exp1 (`?` exp `:` exp1)⋆
std::unique_ptr<Exp> Parser::parse_exp() {
    auto left = parse_exp1(); // Parse higher-precedence expression

    while (check("QuestionMark")) { // TODO no check parens??
        advance(); // consume '?'
        auto true_exp = parse_exp();
        consume("Colon", "unexpected token at token " + std::to_string(peek().index));
        auto false_exp = parse_exp1();
        left = std::make_unique<Select>(std::move(left), std::move(true_exp), std::move(false_exp));
    }
    return left;
}
// std::unique_ptr<Exp> Parser::parse_exp() {
//     auto condition = parse_exp1(); // Parse the condition
//     if (check("QuestionMark")) {
//         advance(); // consume '?'
//         auto true_exp = parse_exp(); // Recursively parse the true-branch expression
//         consume("Colon", "unexpected token at token " + std::to_string(peek().index));
//         auto false_exp = parse_exp(); // Recursively parse the false-branch expression
//         // Note: The false branch should also be parse_exp() to handle nesting
//         return std::make_unique<Select>(std::move(condition), std::move(true_exp), std::move(false_exp));
//     }
//     return condition;
// }

// exp1 ::= exp2 ([`and`,`or`] exp2)⋆
std::unique_ptr<Exp> Parser::parse_exp1() {
    // Right-associative for logical operators 'and'/'or'
    auto left = parse_exp2();
    if (check_any({"And", "Or"})) {
        Token op_token = advance();
        // For right-assoc, parse the rest at the same precedence level recursively
        auto right = parse_exp1();
        BinaryOp op = (op_token.type == "And") ? BinaryOp::And : BinaryOp::Or;
        return std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

// exp2 ::= exp3 ([`==`,`!=`,`<`,`<=`,`>`,`>=`] exp3)⋆
std::unique_ptr<Exp> Parser::parse_exp2() {
    auto left = parse_exp3(); // Parse higher-precedence expression

    // Handle ==, !=, <, <=, >, >= (left-associative)
    while (check_any({"Equal", "NotEq", "Lt", "Lte", "Gt", "Gte"})) {
        Token op_token = advance();
        auto right = parse_exp3();

        BinaryOp op;
        if (op_token.type == "Equal") {
            op = BinaryOp::Eq;
        } else if (op_token.type == "NotEq") {
            op = BinaryOp::NotEq;
        } else if (op_token.type == "Lt") {
            op = BinaryOp::Lt;
        } else if (op_token.type == "Lte") {
            op = BinaryOp::Lte;
        } else if (op_token.type == "Gt") {
            op = BinaryOp::Gt;
        } else if (op_token.type == "Gte") {
            op = BinaryOp::Gte;
        } else {
            error("unexpected token at token " + std::to_string(op_token.index));
        }
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

// exp3 ::= exp4 ((`+`|`-`) exp4)*
std::unique_ptr<Exp> Parser::parse_exp3() {
    auto left = parse_exp4(); // Parse higher-precedence expression

    while (check_any({"Plus", "Dash"})) {
        Token op_token = advance();
        auto right = parse_exp4();
        
        BinaryOp op = (op_token.type == "Plus") ? BinaryOp::Add : BinaryOp::Sub;
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

// exp4 ::= exp5 ((`*`|`/`) exp5)*
std::unique_ptr<Exp> Parser::parse_exp4() {
    auto left = parse_exp5(); // Parse higher-precedence expression
    while (check_any({"Star", "Slash"})) {
        Token op_token = advance();
        auto right = parse_exp5();
        BinaryOp op = (op_token.type == "Star") ? BinaryOp::Mul : BinaryOp::Div;
        left = std::make_unique<BinOp>(op, std::move(left), std::move(right));
    }
    return left;
}

// exp5 ::= unop⋆ exp6
std::unique_ptr<Exp> Parser::parse_exp5() {
    // Handle unary operators (right-associative)
    if (check_any({"Dash", "Not"})) {
        Token op_token = advance();
        auto exp = parse_exp5(); // Right-associative
        UnaryOp op = (op_token.type == "Dash") ? UnaryOp::Neg : UnaryOp::Not;
        return std::make_unique<UnOp>(op, std::move(exp));
    }
    return parse_exp6();
}

// exp6 ::= exp7 call_or_access⋆
// call_or_access ::= `[` exp `]`
                //  | `.` (id | `*`)
                //  | `(` LIST(exp) `)`
// std::unique_ptr<Exp> Parser::parse_exp6() {
//     auto left = parse_exp7();
//     while (check_any({"OpenParen", "OpenBracket", "Dot"})) {
//         if (check("OpenBracket")) {
//             consume("OpenBracket", "unexpected token at token " + std::to_string(peek().index));
//             auto index_exp = parse_exp();
//             consume("CloseBracket", "unexpected token at token " + std::to_string(peek().index));
//             auto place = std::make_unique<ArrayAccess>(std::move(left), std::move(index_exp));
//             left = std::make_unique<Val>(std::move(place));
//         } else if (check("Dot")) {
//             consume("Dot", "unexpected token at token " + std::to_string(peek().index));
//             if (check("Id")) {
//                 Token id_token = advance();
//                 auto place = std::make_unique<FieldAccess>(std::move(left), id_token.value);
//                 left = std::make_unique<Val>(std::move(place));
//             } else if (check("Star")) {
//                 advance();
//                 auto place = std::make_unique<Deref>(std::move(left));
//                 left = std::make_unique<Val>(std::move(place));
//             } else {
//                 error("unexpected token at token " + std::to_string(peek().index));
//             }
//         } else if (check("OpenParen")) {
//             consume("OpenParen", "unexpected token at token " + std::to_string(peek().index));
//             // Parse LIST(exp) for function call arguments
//             auto args = std::vector<std::unique_ptr<Exp>>();
//             if (!check("CloseParen")) { // skip list if no params
//                 do {
//                     args.push_back(parse_exp());
//                 } while (check("Comma") && (advance(), true)); // Consume comma and continue
//             }
//             consume("CloseParen", "unexpected token at token " + std::to_string(peek().index));
//             auto fc = std::make_unique<FunCall>(std::move(left), std::move(args));
//             left = std::make_unique<CallExp>(std::move(fc));
//         }
//         else {
//             error("unexpected token at token " + std::to_string(peek().index));
//         }
//     }
//     return left;
// }
std::unique_ptr<Exp> Parser::parse_exp6() {
    auto exp = parse_exp7(); // Start with a primary expression.

    while (true) {
        if (check("OpenBracket")) {
            advance();
            auto index = parse_exp();
            consume("CloseBracket", "unexpected token at token " + std::to_string(peek().index));
            // Create a Place from the current expression
            auto place = std::make_unique<ArrayAccess>(std::move(exp), std::move(index));
            // Wrap the new Place in a Val to continue the expression chain
            exp = std::make_unique<Val>(std::move(place));
        } else if (check("Dot")) {
            advance();
            if (check("Id")) {
                Token field_token = advance();
                auto place = std::make_unique<FieldAccess>(std::move(exp), field_token.value);
                exp = std::make_unique<Val>(std::move(place));
            } else if (check("Star")) {
                advance();
                auto place = std::make_unique<Deref>(std::move(exp));
                exp = std::make_unique<Val>(std::move(place));
            } else {
                error("unexpected token at token " + std::to_string(peek().index));
            }
        } else if (check("OpenParen")) {
            advance();
            auto args = std::vector<std::unique_ptr<Exp>>();
            if (!check("CloseParen")) {
                do {
                    args.push_back(parse_exp());
                } while (check("Comma") && (advance(), true));
            }
            consume("CloseParen", "unexpected token at token " + std::to_string(peek().index));
            auto fc = std::make_unique<FunCall>(std::move(exp), std::move(args));
            exp = std::make_unique<CallExp>(std::move(fc));
        } else {
            // No more call_or_access operators, break the loop.
            break;
        }
    }
    return exp;
}

// exp7 ::= id
    //    | num
    //    | `nil`
    //    | `new` type
    //    | `[` type `;` exp `]`
    //    | `(` exp `)`
std::unique_ptr<Exp> Parser::parse_exp7() {
    if (check("Id")) {
        Token id_token = advance();
        auto id_place = std::make_unique<Id>(id_token.value);
        return std::make_unique<Val>(std::move(id_place));
    }
    if (check("Num")) {
        Token num_token = advance();
        try {
            return std::make_unique<Num>(std::stoll(num_token.value));
        } catch (const std::out_of_range&) {
            error("invalid i64 number " + num_token.value + " at token " + std::to_string(num_token.index));
        }
    }
    if (check("Nil")) {
        advance();
        return std::make_unique<NilExp>();
    }
    if (check("New")) {
        advance();
        auto type = parse_type();
        return std::make_unique<NewSingle>(std::move(type));
    }
    if (check("OpenBracket")) {
        advance();
        auto type = parse_type();
        consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
        auto size_exp = parse_exp();
        consume("CloseBracket", "unexpected token at token " + std::to_string(peek().index));
        return std::make_unique<NewArray>(std::move(type), std::move(size_exp));
    }
    if (check("OpenParen")) {
        advance();
        auto exp = parse_exp();
        consume("CloseParen", "unexpected token at token " + std::to_string(peek().index));
        return exp;
    }
    error("unexpected token at token " + std::to_string(peek().index));
    return nullptr; // Unreachable
}

// type ::= `int`         # integer type
    //    | id            # struct type
    //    | `&` type      # pointer type
    //    | `[` type `]`  # array type
    //    | funtype       # function type
std::unique_ptr<Type> Parser::parse_type() {
    if (check("Int")) {
        advance();
        return std::make_unique<IntType>();
    }
    else if (check("Id")) {
        Token id_token = advance();
        return std::make_unique<StructType>(id_token.value);
    }
    else if (check("Ampersand")) {
        advance();
        auto inner_type = parse_type();
        return std::make_unique<PtrType>(std::move(inner_type));
    }
    else if (check("OpenBracket")) {
        advance();
        auto inner_type = parse_type();
        consume("CloseBracket", "unexpected token at token " + std::to_string(peek().index));
        return std::make_unique<ArrayType>(std::move(inner_type));
    }
    // else if (check("OpenParen")) { // new
    //     return parse_funtype();
    // }
    // else {
    //     error("Expected a type.");
    // }
    // return nullptr; // Unreachable
    return parse_funtype(); // Fallback to function type
}

// funtype ::= `(` LIST(type) `)` `->` type
std::unique_ptr<Type> Parser::parse_funtype() {
    consume("OpenParen", "unexpected token at token " + std::to_string(peek().index));
    std::vector<std::unique_ptr<Type>> param_types;
    if (!check("CloseParen")) { // skip list if no params
        do {
            param_types.push_back(parse_type());
        } while (check("Comma") && (advance(), true)); // Consume comma and continue
    }
    consume("CloseParen", "unexpected token at token " + std::to_string(peek().index));
    consume("Arrow", "unexpected token at token " + std::to_string(peek().index));
    auto return_type = parse_type();
    return std::make_unique<FnType>(std::move(param_types), std::move(return_type));
}

// `struct` id `{` LIST(decl) `}`
std::unique_ptr<StructDef> Parser::parse_struct_def() {
    consume("Struct", "unexpected token at token " + std::to_string(peek().index));
    auto struct_def = std::make_unique<StructDef>();
    Token name = consume("Id", "unexpected token at token " + std::to_string(peek().index));
    struct_def->name = name.value;
    consume("OpenBrace", "unexpected token at token " + std::to_string(peek().index));
    // Parse LIST(decl) for vector of decls
    if (!check("CloseBrace")) { // skip list if no params
        do {
            struct_def->fields.push_back(parse_decl());
        } while (check("Comma") && (advance(), true)); // Consume comma and continue
    }
    consume("CloseBrace", "unexpected token at token " + std::to_string(peek().index));
    return struct_def;
}

// extern ::= `extern` id `:` funtype `;`
std::unique_ptr<Decl> Parser::parse_extern_def() {
    consume("Extern", "unexpected token at token " + std::to_string(peek().index));
    Token id_token = consume("Id", "unexpected token at token " + std::to_string(peek().index));
    consume("Colon", "unexpected token at token " + std::to_string(peek().index));
    auto funtype = parse_funtype();
    consume("Semicolon", "unexpected token at token " + std::to_string(peek().index));
    return std::make_unique<Decl>(id_token.value, std::move(funtype));
}
// ... and so on for every other parse_... method in the header ...


// --- Helper Method Implementations ---

bool Parser::is_at_end() const {
    return m_current_pos >= m_tokens.size();
}

const Token& Parser::peek() const {
    if (is_at_end()) {
        error("unexpected end of token stream");
    }
    return m_tokens.at(m_current_pos);
}

const Token& Parser::previous() const {
    return m_tokens.at(m_current_pos - 1);
}

Token Parser::advance() {
    if (!is_at_end()) m_current_pos++;
    return previous();
}

Token Parser::consume(const std::string& expected_type, const std::string& error_message) {
    if (is_at_end()) {
        error("unexpected end of token stream");
    }
    if (peek().type != expected_type) {
        error(error_message);
    }
    return advance();
}

bool Parser::check(const std::string& type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

bool Parser::check_any(const std::vector<std::string>& types) const {
    if (is_at_end()) return false;
    for (const auto& type : types) {
        if (peek().type == type) return true;
    }
    return false;
}

// The error messages:
// - "parse error: unexpected token at token <index>"
// - "parse error: unexpected end of token stream"
// - "parse error: left-hand side of assignment must be a place, starting at token <index>"
// - "parse error: standalone expressions must be function calls, starting at token <index>"
// - "parse error: invalid i64 number <string> at token <index>"
void Parser::error(const std::string& message) const {
    std::string full_message = "parse error: " + message;
    throw std::runtime_error(full_message);
}