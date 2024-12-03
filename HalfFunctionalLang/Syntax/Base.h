#pragma once
#include<string>
#include<array>
#include<variant>
#include<vector>
#include<memory>
#include<set>
#include <optional>

const std::set<std::string> Keyword =
{
    "type", "function",
    "let", "in", "end",
    "if", "then", "else",
    "while", "for",
    "true", "false", "and", "or", "xor"
};


struct Half_Expr;
struct Half_Var;
struct Half_Value;
struct Half_Function;
struct Half_Funcall;
struct Half_Op;
struct Half_Assign;
struct Half_Let;
struct Half_If;
struct Half_For;
struct Half_While;
struct Half_Define;
struct Def_Type;
struct Def_Func;

struct Half_Expr
{
    using Expr = std::variant<
        std::shared_ptr<Half_Var>,
        std::shared_ptr<Half_Value>,
        std::shared_ptr<Half_Function>,
        std::shared_ptr<Half_Funcall>,
        std::shared_ptr<Half_Op>,
        std::shared_ptr<Half_Assign>,
        std::shared_ptr<Def_Func>,
        std::shared_ptr<Half_Let>,
        std::shared_ptr<Half_If>,
        std::shared_ptr<Half_For>,
        std::shared_ptr<Half_While>,
        std::shared_ptr<std::vector<Half_Expr>>>;

    Half_Expr() = default;
    template<typename T>
    Half_Expr(const T& t) : expr(std::make_shared<T>(t)) {}
    template<>
    Half_Expr(const std::vector<Half_Expr>& t)
        : expr(std::make_shared<std::vector<Half_Expr>>(std::move(t))) {}
    Half_Expr(std::vector<Half_Expr>&& t)
        : expr(std::make_shared<std::vector<Half_Expr>>(t)) {}
    //template<typename T>
    //Half_Expr(T&& t) : expr(std::make_shared(std::move(t))) {}
    template<typename T>
    Half_Expr(std::shared_ptr<T>& p): expr(p){}
    

    Expr expr;
};

struct Half_Var
{
    struct SimpleVar
    {
        std::string id;
        SimpleVar(std::string s) :id(s) {}
        SimpleVar(const SimpleVar& v) = default;
        SimpleVar& operator=(const SimpleVar& v) = default;
    };
    struct FieldVar
    {
        std::shared_ptr<Half_Var> var;
        std::string id;
        FieldVar(std::shared_ptr<Half_Var>& v, std::string s);
        FieldVar(const FieldVar& v) noexcept;
        FieldVar& operator=(const FieldVar& v) noexcept;
        ~FieldVar() = default;
    };
    struct SubscriptVar
    {
        std::shared_ptr<Half_Var> var;
        std::shared_ptr<Half_Expr> index;
        SubscriptVar(std::shared_ptr<Half_Var>& v, std::shared_ptr<Half_Expr>& e);
        SubscriptVar(const SubscriptVar& v);
        SubscriptVar& operator=(const SubscriptVar& v) noexcept;
        ~SubscriptVar() = default;
    };

    Half_Var() = default;
    Half_Var(SimpleVar&& v);
    Half_Var(FieldVar&& v);
    Half_Var(SubscriptVar&& v);
    Half_Var(const Half_Var& v);
    Half_Var& operator=(const Half_Var& v);
    std::string name() const;

    std::variant<std::monostate, SimpleVar, FieldVar, SubscriptVar> var;
};

// char, int, float, string
using BasicValue = std::variant<char, int, float, std::string>;
struct Half_Value
{
    using ValueT = BasicValue;
    Half_Value() = default;
    Half_Value(ValueT v) :value(v) {}
    Half_Value(const Half_Value& v) : value(v.value) {}
    Half_Value& operator=(const Half_Value& v);

    ValueT value;
};

struct Half_Funcall
{
    std::string name;
    std::vector<Half_Expr> args;

    Half_Funcall() = default;
    Half_Funcall(std::string n, std::vector<Half_Expr>&& args_);
    Half_Funcall(const Half_Funcall& other);
    Half_Funcall& operator=(const Half_Funcall& other);
};

struct Half_Op
{
    using Half_OpExpr = std::variant<Half_Var, Half_Value, Half_Funcall, Half_Op>;
    std::string op;
    std::unique_ptr<Half_OpExpr> left;
    std::unique_ptr<Half_OpExpr> right;

    Half_Op() = default;
    Half_Op(std::string o, Half_OpExpr&& l, Half_OpExpr&& r);
    Half_Op(const Half_Op& other);
    Half_Op& operator=(const Half_Op& other);
    //Half_Op(Half_Op&& other);
};

struct Half_Assign
{
    Half_Var left;
    Half_Expr right;
    Half_Assign() = default;
    Half_Assign(const Half_Var& l, const Half_Expr& r);
    Half_Assign(const Half_Assign& other);
    Half_Assign& operator=(const Half_Assign& other);
};

using Def_Var = Half_Assign;
struct Def_Func
{
    struct TypeField
    {
        std::string name;
        std::string type;
    };
    std::string name;
    std::vector<TypeField> parameters;
    std::string return_type;
    Half_Expr func_body;

    Def_Func() = default;
    Def_Func(std::string n, std::vector<TypeField>& p, std::string r, Half_Expr b);
    Def_Func(const Def_Func& o);
    Def_Func& operator=(const Def_Func& o);
};

struct Def_Type
{
    struct RenameType
    {
        std::string original_name;
    };
    struct ArrayType
    {
        std::string type_name;
    };
    struct StructType
    {
        struct TypePair
        {
            std::string name;
            std::string type;
            TypePair(std::string n, std::string t) : name(n), type(t) {}
            TypePair(const TypePair& o) : name(o.name), type(o.type) {}
        };
        std::string name;
        std::vector<TypePair> field_list;

        StructType(std::string n, std::vector<TypePair>& f) : name(n), field_list(f) {}
    };
    using Type = std::variant<RenameType, ArrayType, StructType>;
    Type type;

    Def_Type() = default;
    Def_Type(const RenameType& t);
    Def_Type(const ArrayType& t);
    Def_Type(const StructType& t);
    Def_Type(const Def_Type& o);
    Def_Type& operator=(const Def_Type& o);
};
struct Half_Let
{
    Half_Let() = default;
    Half_Let(const Def_Var& v);
    Half_Let(const Half_Let& o);
    Half_Let& operator=(const Half_Let& o);

    Def_Var def;
};

struct Half_If
{
    Half_Expr condition;
    Half_Expr trueExpr;
    Half_Expr falseExpr;
    Half_If(const Half_Expr& cond, const Half_Expr& t, const Half_Expr& f);
    Half_If(const Half_If& other);
    Half_If& operator=(const Half_If& other);
};

struct Half_For
{
    Half_Var var;
    Half_Expr start;
    Half_Expr end;
    //std::optional<Half_Expr> condition;
    bool isup;
    Half_Expr body;

    Half_For() = default;
    Half_For(const Half_Var& v, const Half_Expr& s, const Half_Expr& e, const Half_Expr& b, bool up);
    Half_For(const Half_For& o);
    Half_For& operator=(const Half_For& o);
};

struct Half_While
{
    Half_Expr condition;
    Half_Expr body;

    Half_While() = default;
    Half_While(const Half_Expr& c, const Half_Expr& b);
    Half_While(const Half_While& o);
    Half_While& operator=(const Half_While& o);
};
