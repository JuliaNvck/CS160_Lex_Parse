#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <set>

// Forward declarations
struct Type;
struct Stmt;
struct Exp;
struct Place;
struct FunCall;
struct Decl;

enum class UnaryOp { Neg, Not };
enum class BinaryOp { Add, Sub, Mul, Div, And, Or, Eq, NotEq, Lt, Lte, Gt, Gte };

// Base class for all AST nodes
struct Node {
    virtual ~Node() = default;
    virtual void print(std::ostream& os) const = 0; // print function
};

// Overload << operator to make printing easy
inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    node.print(os);
    return os;
}

// Type nodes
// Type
// | Int
// | Struct(String)
// | Fn(vector<Type>, Type)
// | Ptr(Type)
// | Array(Type)
// | Nil

struct Type : public Node {
    // Base class for all types
};

struct IntType : public Type {
    void print(std::ostream& os) const override {
        os << "Int";
    }
};

struct StructType : public Type {
    std::string name;
    explicit StructType(std::string n) : name(std::move(n)) {}

    void print(std::ostream& os) const override {
        os << "Struct(" << name << ")";
    }
};

struct FnType : public Type {
    std::vector<std::unique_ptr<Type>> param_types;
    std::unique_ptr<Type> return_type;

    FnType(std::vector<std::unique_ptr<Type>> ptrs, std::unique_ptr<Type> rt)
    : param_types(std::move(ptrs)), return_type(std::move(rt)) {}

    void print(std::ostream& os) const override {
        os << "Fn([";
        for (size_t i = 0; i < param_types.size(); ++i) {
            param_types[i]->print(os);
            if (i < param_types.size() - 1) os << ", ";
        }
        os << "], ";
        return_type->print(os);
        os << ")";
    }
};

struct PtrType : public Type {
    std::unique_ptr<Type> base_type;
    explicit PtrType(std::unique_ptr<Type> bt) : base_type(std::move(bt)) {}

    void print(std::ostream& os) const override {
        os << "Ptr(";
        base_type->print(os);
        os << ")";
    }
};

struct ArrayType : public Type {
    std::unique_ptr<Type> element_type;
    explicit ArrayType(std::unique_ptr<Type> et) : element_type(std::move(et)) {}

    void print(std::ostream& os) const override {
        os << "Array(";
        element_type->print(os);
        os << ")";
    }
};

struct NilType : public Type {
    void print(std::ostream& os) const override {
        os << "Nil";
    }
};

// Decl
struct Decl : public Node {
    std::string name;
    std::unique_ptr<Type> type;

    Decl(std::string n, std::unique_ptr<Type> t) : name(std::move(n)), type(std::move(t)) {}

    void print(std::ostream& os) const override {
        os << "Decl { name: \"" << name << "\", typ: ";
        type->print(os);
        os << " }";
    }
};

// Place base struct and Id (Id doesn't use Exp)
struct Place : public Node {
    // Base class for memory locations
};

struct Id : public Place {
    std::string name;
    explicit Id(std::string n) : name(std::move(n)) {}

    void print(std::ostream& os) const override {
        os << "Id(\"" << name << "\")";
    }
};

// Expression nodes
// Exp
// | Val(Place)
// | Num(i64) // must be non-negative and fit inside a 64-bit signed integer
// | Nil
// | Select { guard:Exp, tt:Exp, ff:Exp }
// | UnOp(UnaryOp, Exp)
// | BinOp { op:BinaryOp, left:Exp, right:Exp }
// | NewSingle(Type)
// | NewArray(Type, Exp)
// | Call(FunCall)

struct Exp : public Node {
    // Base class for all expressions
};

struct Val : public Exp {
    std::unique_ptr<Place> place;
    explicit Val(std::unique_ptr<Place> p) : place(std::move(p)) {}

    void print(std::ostream& os) const override {
        os << "Val(";
        place->print(os);
        os << ")";
    }
};

struct Num : public Exp {
    long long value;
    explicit Num(long long val) : value(val) {}

    void print(std::ostream& os) const override {
        os << "Num(" << value << ")";
    }
};

struct NilExp : public Exp {
    void print(std::ostream& os) const override {
        os << "Nil";
    }
};

struct Select : public Exp {
    std::unique_ptr<Exp> guard;
    std::unique_ptr<Exp> tt;
    std::unique_ptr<Exp> ff;

    Select(std::unique_ptr<Exp> g, std::unique_ptr<Exp> t, std::unique_ptr<Exp> f) 
    : guard(std::move(g)), tt(std::move(t)), ff(std::move(f)) {}

    void print(std::ostream& os) const override {
        os << "Select { guard: ";
        guard->print(os);
        os << ", tt: ";
        tt->print(os);
        os << ", ff: ";
        ff->print(os);
        os << " }";
    }
};

struct UnOp : public Exp {
    UnaryOp op;
    std::unique_ptr<Exp> exp;

    UnOp(UnaryOp o, std::unique_ptr<Exp> e) : op(o), exp(std::move(e)) {}

    void print(std::ostream& os) const override {
        os << "UnOp(";
        switch (op) {
            case UnaryOp::Neg: os << "Neg"; break;
            case UnaryOp::Not: os << "Not"; break;
        }
        os << ", ";
        exp->print(os);
        os << ")";
    }
};

struct BinOp : public Exp {
    BinaryOp op;
    std::unique_ptr<Exp> left;
    std::unique_ptr<Exp> right;

    BinOp(BinaryOp o, std::unique_ptr<Exp> l, std::unique_ptr<Exp> r) 
    : op(o), left(std::move(l)), right(std::move(r)) {}

    void print(std::ostream& os) const override {
        os << "BinOp { op: ";
        switch (op) {
            case BinaryOp::Add: os << "Add"; break;
            case BinaryOp::Sub: os << "Sub"; break;
            case BinaryOp::Mul: os << "Mul"; break;
            case BinaryOp::Div: os << "Div"; break;
            case BinaryOp::And: os << "And"; break;
            case BinaryOp::Or: os << "Or"; break;
            case BinaryOp::Eq: os << "Eq"; break;
            case BinaryOp::NotEq: os << "NotEq"; break;
            case BinaryOp::Lt: os << "Lt"; break;
            case BinaryOp::Lte: os << "Lte"; break;
            case BinaryOp::Gt: os << "Gt"; break;
            case BinaryOp::Gte: os << "Gte"; break;
        }
        os << ", left: ";
        left->print(os);
        os << ", right: ";
        right->print(os);
        os << " }";
    }
};

struct NewSingle : public Exp {
    std::unique_ptr<Type> type;
    explicit NewSingle(std::unique_ptr<Type> t) : type(std::move(t)) {}

    void print(std::ostream& os) const override {
        os << "NewSingle(";
        type->print(os);
        os << ")";
    }
};

struct NewArray : public Exp {
    std::unique_ptr<Type> type;
    std::unique_ptr<Exp> size;

    NewArray(std::unique_ptr<Type> t, std::unique_ptr<Exp> s) 
    : type(std::move(t)), size(std::move(s)) {}

    void print(std::ostream& os) const override {
        os << "NewArray { typ: ";
        type->print(os);
        os << ", size: ";
        size->print(os);
        os << " }";
    }
};

// Place derivatives that use Exp
struct Deref : public Place {
    std::unique_ptr<Exp> exp;
    explicit Deref(std::unique_ptr<Exp> e) : exp(std::move(e)) {}

    void print(std::ostream& os) const override {
        os << "Deref(";
        exp->print(os);
        os << ")";
    }
};

struct ArrayAccess : public Place {
    std::unique_ptr<Exp> array;
    std::unique_ptr<Exp> index;

    ArrayAccess(std::unique_ptr<Exp> arr, std::unique_ptr<Exp> idx) 
    : array(std::move(arr)), index(std::move(idx)) {}

    void print(std::ostream& os) const override {
        os << "ArrayAccess { array: ";
        array->print(os);
        os << ", index: ";
        index->print(os);
        os << " }";
    }
};

struct FieldAccess : public Place {
    std::unique_ptr<Exp> ptr;
    std::string field;

    FieldAccess(std::unique_ptr<Exp> p, std::string f) 
    : ptr(std::move(p)), field(std::move(f)) {}

    void print(std::ostream& os) const override {
        os << "FieldAccess { ptr: ";
        ptr->print(os);
        os << ", field: \"" << field << "\" }";
    }
};

// FunCall
struct FunCall: Node {
    std::unique_ptr<Exp> callee;
    std::vector<std::unique_ptr<Exp>> args;

    FunCall(std::unique_ptr<Exp> c, std::vector<std::unique_ptr<Exp>> a) 
    : callee(std::move(c)), args(std::move(a)) {}

    void print(std::ostream& os) const override {
        os << "FunCall { callee: ";
        callee->print(os);
        os << ", args: [";
        for (size_t i = 0; i < args.size(); ++i) {
            args[i]->print(os);
            if (i < args.size() - 1) os << ", ";
        }
        os << "] }";
    }
};

struct CallExp : public Exp {
    std::unique_ptr<FunCall> fun_call;
    explicit CallExp(std::unique_ptr<FunCall> fc) : fun_call(std::move(fc)) {}

    void print(std::ostream& os) const override {
        os << "Call(";
        fun_call->print(os);
        os << ")";
    }
};

// Statement nodes
// Stmt
// | Assign(Place, Exp)
// | Call(FunCall)
// | If { guard:Exp, tt:vector<Stmt>, ff:vector<Stmt> }
// | While { guard:Exp, body:vector<Stmt> }
// | Break
// | Continue
// | Return(Exp)

struct Stmt : public Node {
    // Base class for all statements
};

struct Assign : public Stmt {
    std::unique_ptr<Place> place;
    std::unique_ptr<Exp> exp;

    Assign(std::unique_ptr<Place> p, std::unique_ptr<Exp> e) 
    : place(std::move(p)), exp(std::move(e)) {}

    void print(std::ostream& os) const override {
        os << "Assign(";
        place->print(os);
        os << ", ";
        exp->print(os);
        os << ")";
    }
};

struct CallStmt : public Stmt {
    std::unique_ptr<FunCall> fun_call;
    explicit CallStmt(std::unique_ptr<FunCall> fc) : fun_call(std::move(fc)) {}

    void print(std::ostream& os) const override {
        os << "Call(";
        fun_call->print(os);
        os << ")";
    }
};

struct If : public Stmt {
    std::unique_ptr<Exp> guard;
    std::vector<std::unique_ptr<Stmt>> tt;
    std::vector<std::unique_ptr<Stmt>> ff;

    If(std::unique_ptr<Exp> g, std::vector<std::unique_ptr<Stmt>> t, std::vector<std::unique_ptr<Stmt>> f) 
    : guard(std::move(g)), tt(std::move(t)), ff(std::move(f)) {}

    void print(std::ostream& os) const override {
        os << "If { guard: ";
        guard->print(os);
        os << ", tt: [";
        for (size_t i = 0; i < tt.size(); ++i) {
            tt[i]->print(os);
            if (i < tt.size() - 1) os << ", ";
        }
        os << "], ff: [";
        for (size_t i = 0; i < ff.size(); ++i) {
            ff[i]->print(os);
            if (i < ff.size() - 1) os << ", ";
        }
        os << "] }";
    }
};

struct While : public Stmt {
    std::unique_ptr<Exp> guard;
    std::vector<std::unique_ptr<Stmt>> body;

    While(std::unique_ptr<Exp> g, std::vector<std::unique_ptr<Stmt>> b) 
    : guard(std::move(g)), body(std::move(b)) {}

    void print(std::ostream& os) const override {
        os << "While(";
        guard->print(os);
        os << ", [";
        for (size_t i = 0; i < body.size(); ++i) {
            body[i]->print(os);
            if (i < body.size() - 1) os << ", ";
        }
        os << "])";
    }
};

struct Break : public Stmt {
    void print(std::ostream& os) const override {
        os << "Break";
    }
};

struct Continue : public Stmt {
    void print(std::ostream& os) const override {
        os << "Continue";
    }
};

struct Return : public Stmt {
    std::unique_ptr<Exp> exp;
    explicit Return(std::unique_ptr<Exp> e) : exp(std::move(e)) {}
    void print(std::ostream& os) const override {
        os << "Return(";
        exp->print(os);
        os << ")";
    }
};

// Top level nodes

struct FunctionDef : public Node {
    std::string name;
    std::vector<std::unique_ptr<Decl>> params;
    std::unique_ptr<Type> rettype;
    std::vector<std::unique_ptr<Decl>> locals;
    std::vector<std::unique_ptr<Stmt>> stmts;

    void print(std::ostream& os) const override {
        os << "Function { name: \"" << name << "\", ";
        os << "prms: [";
        for (size_t i = 0; i < params.size(); ++i) {
            params[i]->print(os);
            if (i < params.size() - 1) os << ", ";
        }
        os << "], rettyp: ";
        rettype->print(os);
        os << ", locals: {";
        for (const auto& l : locals) {
            l->print(os);
            if (l != locals.back()) os << ", ";
        }
        os << "}, ";
        os << "stmts: [";
        for (size_t i = 0; i < stmts.size(); ++i) {
            stmts[i]->print(os);
            if (i < stmts.size() - 1) os << ", ";
        }
        os << "] }";
    }
};

struct StructDef : public Node {
    std::string name;
    std::vector<std::unique_ptr<Decl>> fields;

    void print(std::ostream& os) const override {
        os << "Struct { name: \"" << name << "\", fields: [";
        for (size_t i = 0; i < fields.size(); ++i) {
            fields[i]->print(os);
            if (i < fields.size() - 1) os << ", ";
        }
        os << "] }";
    }
};

struct Program : public Node {
    std::vector<std::unique_ptr<StructDef>> structs;
    std::vector<std::unique_ptr<Decl>> externs;
    std::vector<std::unique_ptr<FunctionDef>> functions;

    void print(std::ostream& os) const override {
        os << "Program { structs: {";
        for (const auto& s : structs) {
            s->print(os);
            os << ", ";
        }
        os << "}, externs: {";
        for (const auto& e : externs) {
            e->print(os);
            os << ", ";
        }
        os << "}, functions: {";
        for (size_t i = 0; i < functions.size(); ++i) {
            functions[i]->print(os);
            if (i < functions.size() - 1) os << ", ";
        }
        os << "}}";
    }
};