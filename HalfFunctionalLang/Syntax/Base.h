#pragma once
#include<string>
#include<array>
#include<variant>
#include<vector>
#include<memory>
#include<map>
#include<set>
#include <optional>

const std::set<std::string> Keyword =
{
    "type", "function",
    "array", "of", "struct",
    //"nil", "ptr",
    "let", "in", "end",
    "if", "then", "else",
    "while", "for",
    "true", "false", "and", "or", "xor"
};


struct Half_Expr;
struct Half_Var;
struct Half_Value;
struct Half_ArrayInit;
struct Half_StructInit;
struct Half_Funcall;
struct Half_Op;
struct Half_Assign;
struct Half_Let;
struct Half_If;
struct Half_For;
struct Half_While;
struct Half_Define;
struct Def_Type;
struct Half_FuncDecl;
struct Half_TypeDecl;

// function declaration
// name, parameters, return type, body
//  function should support template

// binary operator (type, operator, left, right)
// type definition
//  basic type (char, int, float, string)
//  array type (type, count)
//  struct(record) type (name, fields)
//  rename type (name)
//  function type (parameters, return type)
// variable definition
// return statement (type, value) // maybe not needed


struct Half_Expr
{
    using Expr = std::variant<
        std::shared_ptr<Half_Var>,    // rename to VarDecl
        std::shared_ptr<Half_Value>,    // rename to ValueLiteral
        std::shared_ptr<Half_ArrayInit>,
        std::shared_ptr<Half_StructInit>,
        std::shared_ptr<Half_Funcall>,
        std::shared_ptr<Half_Op>,
        std::shared_ptr<Half_Assign>,// deprecated, use VarDecl instead
        std::shared_ptr<Half_Let>,  // deprecated, use VarDecl|FunctionDecl instead
        std::shared_ptr<Half_If>,
        std::shared_ptr<Half_For>,
        std::shared_ptr<Half_While>,
        std::shared_ptr<Half_FuncDecl>,
        std::shared_ptr<Half_TypeDecl>,
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
        bool operator<(const SimpleVar& other) const {
            return id < other.id;
        }

        bool operator==(const SimpleVar& other) const {
            return id == other.id;
        }
    };
    struct FieldVar
    {
        std::shared_ptr<Half_Var> var;
        std::string id;
        FieldVar(std::shared_ptr<Half_Var>& v, std::string s);
        FieldVar(const FieldVar& v) noexcept;
        FieldVar& operator=(const FieldVar& v) noexcept;
        ~FieldVar() = default;
        bool operator<(const FieldVar& other) const {
            return std::tie(var, id) < std::tie(other.var, other.id);
        }

        bool operator==(const FieldVar& other) const {
            return std::tie(var, id) == std::tie(other.var, other.id);
        }
    };
    struct SubscriptVar
    {
        std::shared_ptr<Half_Var> var;
        std::shared_ptr<Half_Expr> index;
        SubscriptVar(std::shared_ptr<Half_Var>& v, std::shared_ptr<Half_Expr>& e);
        SubscriptVar(const SubscriptVar& v);
        SubscriptVar& operator=(const SubscriptVar& v) noexcept;
        ~SubscriptVar() = default;
        bool operator<(const SubscriptVar& other) const {
            return std::tie(var, index) < std::tie(other.var, other.index);
        }

        bool operator==(const SubscriptVar& other) const {
            return std::tie(var, index) == std::tie(other.var, other.index);
        }
    };

    Half_Var() = default;
    Half_Var(SimpleVar&& v);
    Half_Var(FieldVar&& v);
    Half_Var(SubscriptVar&& v);
    Half_Var(const Half_Var& v);
    Half_Var& operator=(const Half_Var& v);
    std::string name() const;
    bool operator<(const Half_Var& other) const {
        return var < other.var;
    }

    bool operator==(const Half_Var& other) const {
        return var == other.var;
    }

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

struct Half_ArrayInit
{
    std::string type_name;
    std::vector<Half_Expr> values;
    Half_ArrayInit() = default;
    Half_ArrayInit(std::string t, std::vector<Half_Expr>& v) : type_name(std::move(t)), values(std::move(v)) {}
    Half_ArrayInit(const Half_ArrayInit& o) : type_name(o.type_name), values(o.values) {}
};

struct Half_StructInit
{
    struct FieldInit
    {
        std::string name;
        Half_Expr value;
        FieldInit() = default;
        FieldInit(std::string n, Half_Expr v) : name(std::move(n)), value(std::move(v)) {}
        FieldInit(const FieldInit& o) : name(o.name), value(o.value) {}
    };
    std::string type_name;
    std::vector<FieldInit> fields;
    Half_StructInit(std::string t, std::vector<FieldInit>& f) : type_name(std::move(t)), fields(std::move(f)) {}
    Half_StructInit(const Half_StructInit& o) : type_name(o.type_name), fields(o.fields) {}
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

Half_Op::Half_OpExpr ConvertToOpExpr(Half_Expr& v);

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

struct Half_FuncDecl
{
    struct TypeField
    {
        std::string var_name;
        std::string type_name;
    };
    std::string name;
    std::vector<TypeField> parameters;
    std::string return_type;
    Half_Expr body;
    Half_FuncDecl() = default;
    Half_FuncDecl(std::string n, std::vector<TypeField>& p, std::string r, Half_Expr b)
        : name(std::move(n)), parameters(std::move(p)), return_type(std::move(r)), body(std::move(b)) {}
    Half_FuncDecl(const Half_FuncDecl& o) = default;
    Half_FuncDecl& operator=(const Half_FuncDecl& o) = default;
};

struct Half_TypeDecl
{
    using name_ref_t = std::string;
    using type_ref_t = std::shared_ptr<Half_TypeDecl>;
    using type_record_t = std::map<name_ref_t, type_ref_t>;

    // void type, used for function return type or null pointer
    struct Nil
    {
        int dummy;
        bool operator==(const Nil& o) const { return true; }
    };
    // type ... (this is the signature used in printf) int printf(const char* format, ...);
    struct Additional
    {
        int dummy;
        bool operator==(const Additional& o) const { return true; }
    };
    struct Ptr
    {
        name_ref_t target;
        Ptr(name_ref_t t) : target(std::move(t)) {}
        bool operator==(const Ptr& o) const { return target == o.target; }
    };
    struct BasicType
    {
        /*enum class BasicT
        {
            Char, Int, Float, String
        };*/
        std::string type_name;
        BasicType() = default;
        BasicType(std::string t) : type_name(std::move(t)) {}
        BasicType(const BasicType& o) : type_name(o.type_name) {}
        BasicType& operator=(const BasicType& o)
        {
            type_name = o.type_name;
            return *this;
        }
        bool operator==(const BasicType& o) const
        {
            return type_name == o.type_name;
        }
        static bool is_basic_t(const Half_Value& v);
    };
    struct TupleType
    {
        using T = std::shared_ptr<Half_TypeDecl>;
        std::vector<T> type_list;
        TupleType(std::vector<T> t) : type_list(std::move(t)) {}
        bool operator==(const TupleType& o) const
        {
            if (type_list.size() != o.type_list.size())
            {
                return false;
            }
            for (auto i = 0; i < type_list.size(); i++)
            {
                if (type_list[i] != o.type_list[i])
                {
                    return false;
                }
            }
            return true;
        }
    };
    struct RenameType
    {
        using T = std::shared_ptr<Half_TypeDecl>;
        std::string name;
        T type;
        RenameType(std::string n, Half_TypeDecl t) : name(std::move(n)), type(std::make_shared<Half_TypeDecl>(t)) {}
        bool operator==(const RenameType& o) const
        {
            return type == o.type;
        }
    };
    // no size, used in rename type
    struct IncompleteArrayType
    {
        std::string name;
        IncompleteArrayType(std::string n) : name(std::move(n)) {}
        bool operator==(const IncompleteArrayType& o) const
        {
            return name == o.name;
        }
    };
    struct CompleteArrayType
    {
        name_ref_t incomplete_array_name;
        int count;
        CompleteArrayType(std::string n, int c) : incomplete_array_name(std::move(n)), count(c) {}
        bool operator==(const CompleteArrayType& o) const
        {
            return incomplete_array_name == o.incomplete_array_name && count == o.count;
        }
    };
    struct ArrayType
    {
        name_ref_t type_name;
        int count;
        ArrayType(name_ref_t t, int c) : type_name(std::move(t)), count(c) {}
        ArrayType(const ArrayType& o) : type_name(o.type_name), count(o.count) {}
        ArrayType& operator=(const ArrayType& o)
        {
            type_name = o.type_name;
            count = o.count;
            return *this;
        }
        bool operator==(const ArrayType& o) const
        {
            return type_name == o.type_name && count == o.count;
        }
        bool operator!=(const ArrayType& o) const
        {
            return !(*this == o);
        }
    };
    struct StructType
    {
        struct TypePair
        {
            std::string name;
            std::string type;
            TypePair(std::string n, std::string t) : name(std::move(n)), type(std::move(t)) {}
            TypePair(const TypePair& o) : name(o.name), type(o.type) {}
        };
        std::string name;
        std::vector<TypePair> field_list;
        StructType(std::string n, std::vector<TypePair> f) : name(std::move(n)), field_list(std::move(f)) {}
        StructType(const StructType& o) : name(o.name), field_list(o.field_list) {}
        StructType& operator=(const StructType& o)
        {
            name = o.name;
            field_list = o.field_list;
            return *this;
        }
        bool operator==(const StructType& o) const
        {
            return name == o.name;//&& field_list == o.field_list;
        }
        bool operator!=(const StructType& o) const
        {
            return !(*this == o);
        }
    };
    struct FuncType
    {
        std::string return_type;
        std::vector<name_ref_t> parameter_types;
        FuncType() = default;
        FuncType(std::string r, std::vector<name_ref_t> p) : return_type(std::move(r)), parameter_types(std::move(p)) {}
        FuncType(const FuncType& o) : return_type(o.return_type), parameter_types(o.parameter_types) {}
        FuncType& operator=(const FuncType& o)
        {
            parameter_types = o.parameter_types;
            return_type = o.return_type;
            return *this;
        }
        bool operator==(const FuncType& o) const
        {
            if (parameter_types.size() != o.parameter_types.size())
            {
                return false;
            }
            for (auto i = 0; i < parameter_types.size(); i++)
            {
                if (parameter_types[i] != o.parameter_types[i])
                {
                    return false;
                }
            }
            return return_type == o.return_type;// parameter_types == o.parameter_types && return_type == o.return_type;
        }
        bool operator!=(const FuncType& o) const
        {
            return !(*this == o);
        }
    };
    using Type = std::variant<std::monostate, name_ref_t, Nil, Ptr, BasicType, RenameType, IncompleteArrayType, CompleteArrayType, TupleType, ArrayType, StructType, FuncType>;
    //using Type = std::variant<UnknowType, BasicType, RenameType, ArrayType, StructType, FuncType>;
    Type type;
    Half_TypeDecl() = default;
    template<typename T>
    Half_TypeDecl(const T& t) : type(t) {}
    Half_TypeDecl(const Half_TypeDecl& o);
    Half_TypeDecl& operator=(const Half_TypeDecl& o);
    bool operator==(const Half_TypeDecl& o) const;
    bool operator!=(const Half_TypeDecl& o) const;
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
