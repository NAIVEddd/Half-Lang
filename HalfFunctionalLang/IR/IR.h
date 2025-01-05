#pragma once

#include"../Syntax/Base.h"
#include"Temp.h"
#include"BasicBlock.h"
#include"Type.h"
#include<map>

//struct Half_Ir;
struct Half_Ir_Exp;
struct Half_Ir_Stm;
struct Half_Ir_Const;
struct Half_Ir_Float;
struct Half_Ir_String;
struct Half_Ir_Name;
struct Half_Ir_Call;
struct Half_Ir_LlvmBinOp;
struct Half_Ir_Compare;
struct Half_Ir_Memory;
struct Half_Ir_LlvmBranch;  // branch llvm like
struct Half_Ir_Func;
struct Half_Ir_Move;
struct Half_Ir_Label;
struct Half_Ir_Jump;
struct Half_Ir_Seq;
struct Half_Ir_Load;
struct Half_Ir_Store;
struct Half_Ir_ArrayElemLoad;
struct Half_Ir_ArrayElemStore;
struct Half_Ir_ElemLoad;
struct Half_Ir_ElemStore;
struct Half_Ir_GetElementPtr;
struct Half_Ir_Alloc;
struct Half_Ir_Return;
struct Half_Ir_Value;
struct Half_Ir_Function;
struct Half_Ir_Phi;

struct Alloca;
struct Load;
struct Store;

struct Half_Ir_Instruction
{
    using Type = std::variant<
        std::monostate,
        std::shared_ptr<Half_Ir_Const>,
        std::shared_ptr<Half_Ir_Load>,
        std::shared_ptr<Half_Ir_Store>,
        std::shared_ptr<Half_Ir_GetElementPtr>,
        std::shared_ptr<Half_Ir_Alloc>,
        std::shared_ptr<Half_Ir_Return>,
        std::shared_ptr<Half_Ir_Function>,
        std::shared_ptr<Half_Ir_Value>,
        std::shared_ptr<Half_Ir_Call>,
        std::shared_ptr<Half_Ir_LlvmBinOp>,
        std::shared_ptr<Half_Ir_LlvmBranch>,
        std::shared_ptr<Half_Ir_Move>,
        std::shared_ptr<Half_Ir_Label>,
        std::shared_ptr<Half_Ir_Phi>,
        std::shared_ptr<Half_Ir_Jump>
    >;
    Type inst;

    Half_Ir_Instruction() = default;
    template<typename T>
    Half_Ir_Instruction(T t) : inst(std::make_shared<T>(t)) {}
    Half_Ir_Instruction(const Half_Ir_Instruction& o) : inst(o.inst) {}
};

struct Half_Ir_Exp
{
    using Type = std::variant<
        std::monostate,
        std::shared_ptr<Alloca>,
        std::shared_ptr<Load>,
        std::shared_ptr<Store>,
        std::shared_ptr<Half_Ir_Const>,
        std::shared_ptr<Half_Ir_Load>,
        std::shared_ptr<Half_Ir_Store>,
        std::shared_ptr<Half_Ir_ArrayElemLoad>,
        std::shared_ptr<Half_Ir_ArrayElemStore>,
        std::shared_ptr<Half_Ir_ElemLoad>,
        std::shared_ptr<Half_Ir_ElemStore>,
        std::shared_ptr<Half_Ir_GetElementPtr>,
        std::shared_ptr<Half_Ir_Alloc>,
        std::shared_ptr<Half_Ir_Return>,
        std::shared_ptr<Half_Ir_Function>,
        std::shared_ptr<Half_Ir_Name>,
        std::shared_ptr<Half_Ir_Value>,
        std::shared_ptr<Half_Ir_Call>,
        std::shared_ptr<Half_Ir_LlvmBinOp>,
        std::shared_ptr<Half_Ir_Memory>,
        std::shared_ptr<Half_Ir_LlvmBranch>,
        std::shared_ptr<Half_Ir_Func>,
        std::shared_ptr<Half_Ir_Move>,
        std::shared_ptr<Half_Ir_Label>,
        std::shared_ptr<Half_Ir_Phi>,
        std::shared_ptr<Half_Ir_Jump>,
        std::shared_ptr<Half_Ir_Seq>
    >;
    Type exp;

    Half_Ir_Exp() = default;
    template<typename T>
    Half_Ir_Exp(T t) : exp(std::make_shared<T>(t)) {}
    Half_Ir_Exp(const Half_Ir_Exp& o) : exp(o.exp) {}
};


// default address of value is offset(rsp)  start from bottom of the stack
// load value from memory to register

enum class Half_AddressSpace
{
    StackTop,
    StackBottom,
};

struct Address
{   // format as offset(base)
    Half_Type_Info type;
    Temp::Label base;
    size_t offset;
};
struct Register
{
    Half_Type_Info type;
    Temp::Label reg;
};
// Value = Address | Register
struct Value
{
    // get label
    Temp::Label GetLabel() const
    {
        if (std::holds_alternative<Address>(value))
        {
            return std::get<Address>(value).base;
        }
        else if (std::holds_alternative<Register>(value))
        {
            return std::get<Register>(value).reg;
        }
        return Temp::Label("Invalid Value Label");
    }

    std::variant<Address, Register> value;
    Value(Address a) : value(a) {}
    Value(Register r) : value(r) {}
};

// TODO: alloca from bottom of the stack(offset is positive)
//               or top of the stack(offset is negative)
struct Alloca
{
    Address out_address;
    Alloca(Address a) : out_address(a) {}
    Value GetResult() const
    {
        return Value(out_address);
    }
};

// alloca return a address
// format address to offset(rsp)
// so load become Mov offset(rsp), out_label
struct Load
{
    Address address;
    Register out_register;
    Load(Address a) : address(a), out_register(Register{ a.type, Temp::NewLabel() }) {}
    Load(Address a, Register r) : address(a), out_register(r) {}
    Value GetResult() const
    {
        return Value(out_register);
    }
};

// store value to memory
struct Store
{
    Value value;
    Address address;
    Store(Value v, Address a) : value(v), address(a) {}
};

// struct Half_Ir_GetElementPtr
// {
//     Type PointeeType;

//     // Value is Address
//     Value Ptr;
//     // Value is const(index)|pointer(to index)
//     std::vector<Value> index;
//     // Value is Address
//     Value OutValue;
// };

struct Half_Ir_Const
{
    int n;
    Temp::Label out_label;
    Half_Ir_Const(int x, Temp::Label l = Temp::NewLabel()) : n(x), out_label(l) {}
    Half_Ir_Const(const Half_Ir_Const& c) : n(c.n), out_label(c.out_label) {}
    Value GetResult() const
    {
        return Value(Register{Half_Type_Info::BasicType::BasicT::Int, out_label});
    }
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

struct Half_Ir_ElemLoad
{
    // load data from memory
    //  elem_offset(elem_ptr) = elem_ptr + elem_offset (read 4 or 8 bytes from memory)
    size_t elem_offset;
    size_t size;    // int is 4, ...
    Temp::Label elem_ptr;
    Temp::Label out_label;
    Half_Ir_ElemLoad(size_t off, size_t sz, Temp::Label ptr, Temp::Label l)
        : elem_offset(off), size(sz), elem_ptr(ptr), out_label(l) {}
    Half_Ir_ElemLoad(const Half_Ir_ElemLoad& a)
        : elem_offset(a.elem_offset), size(a.size), elem_ptr(a.elem_ptr), out_label(a.out_label) {}
};

struct Half_Ir_ElemStore
{
    // store data to memory
    //   elem_offset(elem_ptr) = elem_ptr + elem_offset (write 4 or 8 bytes to memory)
    size_t elem_offset;
    size_t size;    // int is 4, ...
    Temp::Label elem_ptr;
    Temp::Label in_label;
    Half_Ir_ElemStore(size_t off, size_t sz, Temp::Label ptr, Temp::Label l)
        : elem_offset(off), size(sz), elem_ptr(ptr), in_label(l) {}
    Half_Ir_ElemStore(const Half_Ir_ElemStore& a)
        : elem_offset(a.elem_offset), size(a.size), elem_ptr(a.elem_ptr), in_label(a.in_label) {}
};

struct Half_Ir_ArrayElemLoad
{
    size_t array_offset;
    size_t size;    // int is 4, ...
    Temp::Label index;
    Temp::Label out_label;
    Half_Ir_ArrayElemLoad(size_t off, Temp::Label elem_index, size_t sz, Temp::Label l) : array_offset(off), size(sz), index(elem_index), out_label(l) {}
    Half_Ir_ArrayElemLoad(const Half_Ir_ArrayElemLoad& a) : array_offset(a.array_offset), size(a.size), index(a.index), out_label(a.out_label) {}
};

struct Half_Ir_ArrayElemStore
{
    size_t array_offset;
    size_t size;    // int is 4, ...
    Temp::Label index;
    Temp::Label in_label;
    Half_Ir_ArrayElemStore(size_t off, Temp::Label elem_index, size_t sz, Temp::Label l) : array_offset(off), size(sz), index(elem_index), in_label(l) {}
    Half_Ir_ArrayElemStore(const Half_Ir_ArrayElemStore& a) : array_offset(a.array_offset), size(a.size), index(a.index), in_label(a.in_label) {}
};

struct Half_Ir_GetElementPtr
{
    size_t offset = 0;
    std::vector<size_t> elem_sizes;
    std::vector<Half_Ir_Exp> in_index;
    std::vector<Temp::Label> exp_out_labels;
    Temp::Label out_label;
    Half_Ir_GetElementPtr(Temp::Label l = Temp::NewLabel())
        : out_label(l) {}
    Half_Ir_GetElementPtr(size_t off, Temp::Label l = Temp::NewLabel())
        : offset(off), out_label(l) {}
    Half_Ir_GetElementPtr(size_t off, std::vector<size_t> szs, std::vector<Half_Ir_Exp> ls, Temp::Label out = Temp::NewLabel())
        : offset(off), elem_sizes(szs), in_index(ls), out_label(out) {}
    Half_Ir_GetElementPtr(const Half_Ir_GetElementPtr& g)
        : offset(g.offset), elem_sizes(g.elem_sizes)
        , in_index(g.in_index), exp_out_labels(g.exp_out_labels), out_label(g.out_label) {};
    
    Address GetResult() const
    {
        return Address{ Half_Type_Info::BasicType::BasicT::Int, out_label, offset };
    }

    size_t GetOffset() const
    {
        _ASSERT(elem_sizes.size() == in_index.size());
        if (!is_const_offset())
        {
            return -1;
        }
        size_t sz = offset;
        for (size_t i = 0; i < elem_sizes.size(); i++)
        {
            if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(&in_index[i].exp))
            {
                sz += elem_sizes[i] * (*pconst)->n;
            }
        }
        return sz;
    }
    bool is_const_offset() const
    {
        if (in_index.empty())
        {
            return true;
        }
        for (size_t i = 0; i < in_index.size(); i++)
        {
            if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(&in_index[i].exp))
            {
                continue;
            }
            return false;
        }
        return true;
    }
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

    Half_Ir_BasicBlock alloc;
    std::vector<Half_Ir_BasicBlock> blocks;
    Half_Ir_Function() = default;
    Half_Ir_Function(std::string n, Half_TypeDecl::FuncType t, std::vector<Half_FuncDecl::TypeField>& a)
        : name(n), type(t), args(a) {}
};

struct Half_Ir_Float
{
    float f;
    Half_Ir_Float(float x) : f(x) {}
    Half_Ir_Float(const Half_Ir_Float& f) : f(f.f) {}
};

struct Half_Ir_String
{
    std::string str;
    Half_Ir_String(std::string s) : str(s) {}
    Half_Ir_String(const Half_Ir_String& s) : str(s.str) {}
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
    // Half_Ir_Type type;
    std::variant<Half_Ir_Const, Half_Ir_Float, Half_Ir_String, Half_Ir_Name> val;
    Temp::Label out_label;
    Half_Ir_Value(Half_Ir_Const c, Temp::Label label = Temp::NewLabel()) : val(c), out_label(label) {}
    Half_Ir_Value(Half_Ir_Float f, Temp::Label label = Temp::NewLabel()) : val(f), out_label(label) {}
    Half_Ir_Value(Half_Ir_String s, Temp::Label label = Temp::NewLabel()) : val(s), out_label(label) {}
    Half_Ir_Value(Half_Ir_Name n, Temp::Label label = Temp::NewLabel()) : val(n), out_label(label) {}
    Half_Ir_Value(const Half_Ir_Value& o) : val(o.val), out_label(o.out_label) {}
};

struct Half_Ir_LlvmBinOp
{
    enum class Oper
    {
        Unknow, Plus, Minus, Multy, Divide,
        Mod, And, Or, Xor, LShift, RShift,
        Less, LessEqual, Greater, GreaterEqual,
        Equal, NotEqual
    };
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
        static std::map<std::string, Oper> map =
        {
            {"+", Oper::Plus},
            {"-", Oper::Minus},
            {"*", Oper::Multy},
            {"/", Oper::Divide},
            {"%", Oper::Mod},
            {"&&", Oper::And},
            {"||", Oper::Or},
            {"^", Oper::Xor},
            {"<<", Oper::LShift},
            {">>", Oper::RShift},
            {"<", Oper::Less},
            {"<=", Oper::LessEqual},
            {">", Oper::Greater},
            {">=", Oper::GreaterEqual},
            {"==", Oper::Equal},
            {"!=", Oper::NotEqual}
        };
        auto i = map.find(s);
        if (i == map.end())
        {
            return Oper::Unknow;
        }
        return i->second;
    }
};

struct Half_Ir_Compare
{
    using Oper = Half_Ir_LlvmBinOp::Oper;
    Oper op;
    Half_Ir_Name left;
    Half_Ir_Name right;
    Temp::Label out_label;
    Half_Ir_Compare(Oper o, Half_Ir_Name l, Half_Ir_Name r, Temp::Label out)
        : op(o), left(l), right(r), out_label(out) {}
    Half_Ir_Compare(std::string o, Half_Ir_Name l, Half_Ir_Name r, Temp::Label out)
        : op(GetOper(o)), left(l), right(r), out_label(out) {}
    Half_Ir_Compare(const Half_Ir_Compare& c)
        : op(c.op), left(c.left), right(c.right), out_label(c.out_label) {}
    static Oper GetOper(std::string s)
    {
        return Half_Ir_LlvmBinOp::GetOper(s);
    }
    static Oper GetNot(Oper op)
    {
        switch (op)
        {
        case Oper::Less:
            return Oper::GreaterEqual;
        case Oper::LessEqual:
            return Oper::Greater;
        case Oper::Greater:
            return Oper::LessEqual;
        case Oper::GreaterEqual:
            return Oper::Less;
        case Oper::Equal:
            return Oper::NotEqual;
        case Oper::NotEqual:
            return Oper::Equal;
        default:
            break;
        }
        return Oper::Unknow;
    }
};

struct Half_Ir_Call
{
    Temp::Label fun_name;
    std::vector<Half_Ir_Name> args;
    Temp::Label out_label;
    Half_Ir_Call(Temp::Label call_label, Temp::Label n, std::vector<Half_Ir_Name>& a)
        : fun_name(n), args(a), out_label(call_label) {}
    Half_Ir_Call(const Half_Ir_Call& c)
        : fun_name(c.fun_name), args(c.args), out_label(c.out_label) {}
};

struct Half_Ir_Memory
{
    size_t offset;
    Half_Ir_Memory(size_t o) : offset(o) {}
};

struct Half_Ir_Label
{
    Temp::Label lab;
    Half_Ir_Label(Temp::Label l):lab(l){}
    Half_Ir_Label(const Half_Ir_Label& l) : lab(l.lab) {}
};

struct Half_Ir_LlvmBranch
{
    Half_Ir_Compare condition;
    Temp::Label true_label;
    Temp::Label false_label;
    Half_Ir_LlvmBranch(Half_Ir_Compare cond, Temp::Label tl, Temp::Label fl)
        : condition(cond), true_label(tl), false_label(fl)
    {
    }
};

struct Half_Ir_Phi
{
    Half_Ir_Name result;
    std::vector<std::pair<Half_Ir_Name, Half_Ir_Label>> values;
    Half_Ir_Phi(Half_Ir_Name r) : result(r) {}
    Half_Ir_Phi(const Half_Ir_Phi& p) : result(p.result), values(p.values) {}
    // insert
    void Insert(Half_Ir_Name n, Half_Ir_Label l)
    {
        values.push_back({ n, l });
    }
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