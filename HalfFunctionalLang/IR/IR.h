#pragma once

#include"../Syntax/Base.h"
#include"Temp.h"
#include"BasicBlock.h"
#include<map>

//struct Half_Ir;
struct Half_Ir_Exp;
struct Half_Ir_Stm;
struct Half_Ir_Const;
struct Half_Ir_Name;
struct Half_Ir_Call;
struct Half_Ir_BinOp;
struct Half_Ir_LlvmBinOp;
struct Half_Ir_Memory;
struct Half_Ir_Branch;
struct Half_Ir_Func;
struct Half_Ir_Move;
struct Half_Ir_Label;
struct Half_Ir_Jump;
struct Half_Ir_Seq;
struct Half_Ir_Load;
struct Half_Ir_Store;
struct Half_Ir_Alloc;
struct Half_Ir_Return;
struct Half_Ir_Value;
struct Half_Ir_Function;

struct Half_Ir_Exp
{
    using Type = std::variant<
        std::monostate,
        std::shared_ptr<Half_Ir_Const>,
        std::shared_ptr<Half_Ir_Load>,
        std::shared_ptr<Half_Ir_Store>,
        std::shared_ptr<Half_Ir_Alloc>,
        std::shared_ptr<Half_Ir_Return>,
        std::shared_ptr<Half_Ir_Function>,
        std::shared_ptr<Half_Ir_Name>,
        std::shared_ptr<Half_Ir_Value>,
        std::shared_ptr<Half_Ir_Call>,
        std::shared_ptr<Half_Ir_BinOp>,
        std::shared_ptr<Half_Ir_LlvmBinOp>,
        std::shared_ptr<Half_Ir_Memory>,
        std::shared_ptr<Half_Ir_Branch>,
        std::shared_ptr<Half_Ir_Func>,
        std::shared_ptr<Half_Ir_Move>,
        std::shared_ptr<Half_Ir_Label>,
        std::shared_ptr<Half_Ir_Jump>,
        std::shared_ptr<Half_Ir_Seq>
    >;
    Type exp;

    Half_Ir_Exp() = default;
    template<typename T>
    Half_Ir_Exp(T t) : exp(std::make_shared<T>(t)) {}
    Half_Ir_Exp(const Half_Ir_Exp& o) : exp(o.exp) {}
};

struct Half_Ir_Const
{
    int n;
    Half_Ir_Const() = default;
    Half_Ir_Const(int x) : n(x) {}
};

struct Half_Ir_Alloc
{
    size_t offset;
    Temp::Label out_label;
    Half_Ir_Alloc(size_t off, Temp::Label l) : offset(off), out_label(l) {}
    Half_Ir_Alloc(const Half_Ir_Alloc& a) : offset(a.offset), out_label(a.out_label) {}
};

struct Half_Ir_Load
{
    //size_t size;  // size of the data(4 bytes, 8 bytes, ...) maybe it should be a type
    size_t offset;
    Temp::Label out_label;
    Half_Ir_Load(size_t off, Temp::Label l) : offset(off), out_label(l) {}
    Half_Ir_Load(const Half_Ir_Load& load) : offset(load.offset), out_label(load.out_label) {}
};

struct Half_Ir_Store
{
    // store data to memory
    //   data maybe a register or a constant
    std::variant<size_t, Half_Ir_Const> data;
    Temp::Label in_label;
    Half_Ir_Store(size_t off, Temp::Label l) : data(off), in_label(l) {}
    Half_Ir_Store(Half_Ir_Const c, Temp::Label l) : data(c), in_label(l) {}
    Half_Ir_Store(const Half_Ir_Store& store) : data(store.data), in_label(store.in_label) {}
};

struct Half_Ir_Return
{
    Half_Ir_Exp value;
    Half_Ir_Return(Half_Ir_Exp e) : value(e) {}
    Half_Ir_Return(const Half_Ir_Return& e) : value(e.value) {}
};

struct Half_Ir_Function
{
    std::string name;
    Half_TypeDecl::FuncType type;
    std::vector<Half_FuncDecl::TypeField> args;

    std::vector<Half_Ir_BasicBlock> blocks;
    Half_Ir_Function() = default;
    Half_Ir_Function(std::string n, Half_TypeDecl::FuncType t, std::vector<Half_FuncDecl::TypeField>& a)
        : name(n), type(t), args(a) {}
};

struct Half_Ir_Name
{
    Temp::Label name;
    Half_Ir_Name() : name(Temp::Label("Invalid Value")) {}
    Half_Ir_Name(std::string n) : name(Temp::Label(n)) {}
    Half_Ir_Name(Temp::Label n) : name(n) {}
    Half_Ir_Name(const Half_Ir_Name& n) : name(n.name) {}
};

struct Half_Ir_Value
{
    std::variant<Half_Ir_Const, Half_Ir_Name> val;
    Half_Ir_Value(Half_Ir_Const c) : val(c) {}
    Half_Ir_Value(Half_Ir_Name n) : val(n) {}
    Half_Ir_Value(const Half_Ir_Value& o) : val(o.val) {}
    Half_Ir_Const GetConst()
    {
        if (auto p = std::get_if<Half_Ir_Const>(&val))
        {
            return *p;
        }
        _ASSERT(false);
        return Half_Ir_Const(-1);
    }
    Half_Ir_Name GetName()
    {
        if (auto p = std::get_if<Half_Ir_Name>(&val))
        {
            return *p;
        }
        _ASSERT(false);
        return Half_Ir_Name();
    }
};

struct Half_Ir_BinOp
{
    enum class Oper
    {
        Unknow, Plus, Minus, Multy, Divide,
    };
    Oper op;
    Half_Ir_Exp left;
    Half_Ir_Exp right;
    Half_Ir_BinOp(Oper o, Half_Ir_Exp l, Half_Ir_Exp r)
        : op(o), left(l), right(r) {}
    Half_Ir_BinOp(std::string o, Half_Ir_Exp l, Half_Ir_Exp r)
        : op(GetOper(o)), left(l), right(r) {}
    static Oper GetOper(std::string s)
    {
        static std::map<std::string, Oper> map =
        {
            {"+", Oper::Plus},
            {"-", Oper::Minus},
            {"*", Oper::Multy},
            {"/", Oper::Divide}
        };
        auto i = map.find(s);
        if (i == map.end())
        {
            return Oper::Unknow;
        }
        return i->second;
    }
};

struct Half_Ir_LlvmBinOp
{
    using Oper = Half_Ir_BinOp::Oper;
    Oper op;
    Half_Ir_Name left;
    Half_Ir_Name right;
    Temp::Label out_label;
    Half_Ir_LlvmBinOp(std::string o, Half_Ir_Name l, Half_Ir_Name r, Temp::Label out)
        : op(GetOper(o)), left(l), right(r), out_label(out) {}
    Half_Ir_LlvmBinOp(Oper o, Half_Ir_Name l, Half_Ir_Name r, Temp::Label out)
        : op(o), left(l), right(r), out_label(out) {}
    Half_Ir_LlvmBinOp(const Half_Ir_LlvmBinOp& o)
        : op(o.op), left(o.left), right(o.right), out_label(o.out_label) {}
    static Oper GetOper(std::string s)
    {
        return Half_Ir_BinOp::GetOper(s);
    }
};

struct Half_Ir_Call
{
    Half_Ir_Name fun_name;
    std::vector<Half_Ir_Exp> args;
    Half_Ir_Call(Half_Ir_Name n, std::vector<Half_Ir_Exp>& a)
        : fun_name(n), args(a) {}
};

struct Half_Ir_Memory
{
    size_t offset;
    Half_Ir_Memory(size_t o) : offset(o) {}
};

struct Half_Ir_Branch
{
    Half_Ir_Exp Cond;
    Half_Ir_Exp True;
    Half_Ir_Exp False;
    Half_Ir_Branch(Half_Ir_Exp condition, Half_Ir_Exp truee, Half_Ir_Exp falsee)
        : Cond(condition), True(truee), False(falsee) {}
};

struct Half_Ir_Label
{
    Temp::Label lab;
    Half_Ir_Label(Temp::Label l):lab(l){}
    Half_Ir_Label(const Half_Ir_Label& l) : lab(l.lab) {}
};

struct Half_Ir_Func
{
    Half_Ir_Label name;
    Half_Ir_Exp body;
    Half_Ir_Func(Half_Ir_Label n, Half_Ir_Exp b)
        : name(n), body(b) {}
};

struct Half_Ir_Jump
{
    Temp::Label target;
    Half_Ir_Jump(Temp::Label l) : target(l) {}
};

struct Half_Ir_Move
{
    Half_Ir_Exp left;
    Half_Ir_Exp right;
    Half_Ir_Move(Half_Ir_Exp l, Half_Ir_Exp r)
        :left(l), right(r) {}
};

struct Half_Ir_Seq
{
    std::vector<Half_Ir_Exp> seq;
    Half_Ir_Seq(std::vector<Half_Ir_Exp>& s) : seq(s) {}
    Half_Ir_Seq(std::vector<Half_Ir_Exp>&& s) : seq(std::move(s)) {}
};

struct Half_IR
{
    using IR = std::variant<
        std::monostate,
        Half_Ir_Exp
    >;
    IR ir;

    Half_IR() = default;
    template<typename T>
    Half_IR(T t) : ir(t) {}
};