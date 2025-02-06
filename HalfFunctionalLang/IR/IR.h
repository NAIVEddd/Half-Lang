#pragma once

#include"../Syntax/Base.h"
#include"Temp.h"
#include"BasicBlock.h"
#include"Type.h"
#include<map>

//struct Half_Ir;
struct Half_Ir_Exp;
struct Half_Ir_Stm;
struct Half_Ir_Alloca;
struct Half_Ir_Load;
struct Half_Ir_Store;
struct Half_Ir_Ext;
struct Half_Ir_Const;
struct Half_Ir_Float;
struct Half_Ir_String;
struct Half_Ir_Name;
struct Half_Ir_Call;
struct Half_Ir_BinOp;
struct Half_Ir_Compare;
struct Half_Ir_Branch;  // branch llvm like
struct Half_Ir_Move;
struct Half_Ir_Label;
struct Half_Ir_Jump;
struct Half_Ir_GetElementPtr;
struct Half_Ir_FetchPtr;
struct Half_Ir_Return;
struct Half_Ir_Value;
struct Half_Ir_Function;
struct Half_Ir_Phi;


struct Half_Ir_Exp
{
    using Type = std::variant<
        std::monostate,
        std::shared_ptr<Half_Ir_Alloca>,
        std::shared_ptr<Half_Ir_Load>,
        std::shared_ptr<Half_Ir_Store>,
        std::shared_ptr<Half_Ir_Ext>,
        std::shared_ptr<Half_Ir_Const>,
        std::shared_ptr<Half_Ir_GetElementPtr>,
        std::shared_ptr<Half_Ir_FetchPtr>,
        std::shared_ptr<Half_Ir_Return>,
        std::shared_ptr<Half_Ir_Function>,
        std::shared_ptr<Half_Ir_Name>,
        std::shared_ptr<Half_Ir_Value>,
        std::shared_ptr<Half_Ir_Call>,
        std::shared_ptr<Half_Ir_BinOp>,
        std::shared_ptr<Half_Ir_Branch>,
        std::shared_ptr<Half_Ir_Move>,
        std::shared_ptr<Half_Ir_Label>,
        std::shared_ptr<Half_Ir_Phi>,
        std::shared_ptr<Half_Ir_Jump>
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

struct RealAddress
{
    // this is a element type,
    //  if it is a int, load 4 bytes from memory.
    //  if it is a pointer, load 8 bytes from memory
    Half_Type_Info type;
    Temp::Label base;
    ptrdiff_t offset;
};

struct Address
{   // format as offset(base)
    Half_Type_Info target_type;    // TODO : is this a element type or a pointer type
    std::shared_ptr<RealAddress> real_address;
    Temp::Label reg;

    Address()
    {
        target_type = Half_Type_Info::BasicType::BasicT::Invalid;
        real_address = std::make_shared<RealAddress>();
        real_address->base = Temp::Label("bottom");
        real_address->offset = 0;
    }
    Address(Half_Type_Info t, std::shared_ptr<RealAddress> r, Temp::Label l = Temp::NewLabel())
        : target_type(t), real_address(r), reg(l) {
    }
    Address(const Address& a)
    {
        target_type = a.target_type;
        real_address = a.real_address;
        reg = a.reg;
    }
    bool operator<(const Address& a) const
    {
        if (real_address && a.real_address)
        {
            return real_address->base.l < a.real_address->base.l ||
                ( real_address->base.l == a.real_address->base.l && real_address->offset < a.real_address->offset);
        }
        _ASSERT(false);
        return false;
    }
    bool operator==(const Address& a) const
    {
        // type == a.type &&
        return  real_address->base.l == a.real_address->base.l && real_address->offset == a.real_address->offset;
    }
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
    Half_Type_Info GetType() const
    {
        if (std::holds_alternative<Address>(value))
        {
            return std::get<Address>(value).target_type;
        }
        else if (std::holds_alternative<Register>(value))
        {
            return std::get<Register>(value).type;
        }
        return Half_Type_Info::BasicType::BasicT::Invalid;
    }
    Temp::Label GetLabel() const
    {
        if (std::holds_alternative<Address>(value))
        {
            return std::get<Address>(value).real_address->base;
        }
        else if (std::holds_alternative<Register>(value))
        {
            return std::get<Register>(value).reg;
        }
        return Temp::Label("Value Invalid Label");
    }
    void SetLabel(Temp::Label l)
    {
        if (std::holds_alternative<Address>(value))
        {
            std::get<Address>(value).real_address->base = l;
        }
        else if (std::holds_alternative<Register>(value))
        {
            std::get<Register>(value).reg = l;
        }
    }

    std::variant<Address, Register> value;
    Value(Address a) : value(a) {}
    Value(Register r) : value(r) {}
};

// TODO: alloca from bottom of the stack(offset is positive)
//               or top of the stack(offset is negative)
struct Half_Ir_Alloca
{
    Address out_address;
    Half_Ir_Alloca(Address a) : out_address(a) {}
    Value GetResult() const
    {
        return Value(out_address);
    }
};

// alloca return a address
// format address to offset(rsp)
// so load become Mov offset(rsp), out_label
struct Half_Ir_Load
{
    Address address;
    Register out_register;
    Half_Ir_Load(Address a) : address(a), out_register(Register{ a.target_type, Temp::NewLabel() }) {}
    Half_Ir_Load(Address a, Register r) : address(a), out_register(r) {}
    Value GetResult() const
    {
        return Value(out_register);
    }
};

// store value to memory
struct Half_Ir_Store
{
    Value value;
    Address address;
    Half_Ir_Store(Value v, Address a) : value(v), address(a) {}
};

// extend value to a larger size
struct Half_Ir_Ext
{
    Register value;
    Register out_register;
    Half_Ir_Ext(Register v, Half_Type_Info t, Temp::Label l = Temp::NewLabel())
        : value(v)
    {
        out_register = Register{ t, l };
    }
    Half_Ir_Ext(const Half_Ir_Ext& e) : value(e.value)
    {
        out_register = e.out_register;
    }
    Value GetResult() const
    {
        return Value(out_register);
    }
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

struct Half_Ir_GetElementPtr
{
    using Indexer = std::variant<Half_Ir_Const, Value>;
    Address out_address;
    ptrdiff_t offset = 0;
    Temp::Label base;
    Half_Type_Info source_element_type;
    Half_Type_Info result_element_type;
    std::vector<Indexer> in_indexs;
    Temp::Label out_label;
    Half_Ir_GetElementPtr(Address a, Temp::Label l = Temp::NewLabel())
        : out_address(a), out_label(l)
        , source_element_type(a.target_type)
    {
        result_element_type = source_element_type;
    }

    void AddIndex(Indexer idx)
    {
        in_indexs.push_back(idx);
    }

    Half_Type_Info GetResultType() const
    {

        return result_element_type;
    }
};

struct Half_Ir_FetchPtr
{
    Address ptr;
    Temp::Label out_label;
    Half_Ir_FetchPtr(Address p, Temp::Label l = Temp::NewLabel())
        : ptr(p), out_label(l) {
    }
    Half_Ir_FetchPtr(const Half_Ir_FetchPtr& f)
        : ptr(f.ptr), out_label(f.out_label) {
    }
    Value GetResult() const
    {
        return Value(Register{ Half_Type_Info::PointerType(ptr.target_type), out_label });
    }
};

struct Half_Ir_Return
{
    Value value;
    Half_Ir_Return(Value e) : value(e) {}
    Half_Ir_Return(const Half_Ir_Return& e) : value(e.value) {}
};

struct Half_Ir_Function
{
    std::string name;
    Half_TypeDecl::FuncType type;
    std::vector<Half_FuncDecl::TypeField> args;
    std::vector<size_t> args_size;
    size_t stack_size;

    Half_Ir_BasicBlock alloc;
    std::vector<Half_Ir_BasicBlock> blocks;
    Half_Ir_Function() = default;
    Half_Ir_Function(std::string n, Half_TypeDecl::FuncType t, std::vector<Half_FuncDecl::TypeField>& a, std::vector<size_t>& szs, size_t sz)
        : name(n), type(t), args(a), args_size(szs), stack_size(sz) {}
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
    Temp::Label label;
    Half_Ir_String(std::string s, Temp::Label l = Temp::NewLabel()) : str(s), label(l) {}
    Half_Ir_String(const Half_Ir_String& s) : str(s.str), label(s.label) {}
    Value GetResult() const
    {
        return Value(Register{ Half_Type_Info::BasicType::BasicT::String, label });
    }
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

struct Half_Ir_BinOp
{
    enum class Oper
    {
        Unknow, Plus, Minus, Multy, Divide,
        Mod, And, Or, Xor, LShift, RShift,
        Less, LessEqual, Greater, GreaterEqual,
        Equal, NotEqual
    };
    Oper op;
    Register left;
    Register right;
    Temp::Label out_label;
    Half_Ir_BinOp(std::string o, Register l, Register r, Temp::Label out)
        : op(GetOper(o)), left(l), right(r), out_label(out) {}
    Half_Ir_BinOp(Oper o, Register l, Register r, Temp::Label out)
        : op(o), left(l), right(r), out_label(out) {}
    Half_Ir_BinOp(const Half_Ir_BinOp& o)
        : op(o.op), left(o.left), right(o.right), out_label(o.out_label) {}
    
    Value GetResult() const
    {
        if (left.type.is_pointer() && right.type.is_basic())
        {
            return Value(Register{ left.type, out_label });
        }
        else if (left.type.is_basic() && right.type.is_pointer())
        {
            return Value(Register{ right.type, out_label });
        }
        else if (left.type.is_basic() && right.type.is_basic())
        {
            return Value(Register{ Half_Type_Info::BasicType::BasicT::Int, out_label });
        }
        // wrong type, error message and interrupt
        _ASSERT(false);
        return Value(Register{ Half_Type_Info::BasicType::BasicT::Int, out_label });
    }
    
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
    using Oper = Half_Ir_BinOp::Oper;
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
        return Half_Ir_BinOp::GetOper(s);
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
    Register out_register;
    Temp::Label fun_name;
    std::vector<Half_Ir_Name> args;
    Half_Ir_Call(Register r, Temp::Label n, std::vector<Half_Ir_Name>& a)
        : out_register(r), fun_name(n), args(a) {}
};

struct Half_Ir_Label
{
    Temp::Label lab;
    Half_Ir_Label(Temp::Label l):lab(l){}
    Half_Ir_Label(const Half_Ir_Label& l) : lab(l.lab) {}
};

struct Half_Ir_Branch
{
    Half_Ir_Compare condition;
    Temp::Label true_label;
    Temp::Label false_label;
    Half_Ir_Branch(Half_Ir_Compare cond, Temp::Label tl, Temp::Label fl)
        : condition(cond), true_label(tl), false_label(fl)
    {
    }
};

struct Half_Ir_Phi
{
    Half_Ir_Name result;
    std::vector<std::pair<Half_Ir_Name, Half_Ir_Label>> values;
    Half_Ir_Phi() : result(Half_Ir_Name("Invalid Phi")) {}
    Half_Ir_Phi(Half_Ir_Name r) : result(r) {}
    Half_Ir_Phi(const Half_Ir_Phi& p) : result(p.result), values(p.values) {}
    Value GetResult() const
    {
        return Value(Register{ Half_Type_Info::BasicType::BasicT::Int, result.name });
    }
    // insert
    void Insert(Half_Ir_Name n, Half_Ir_Label l)
    {
        values.push_back({ n, l });
    }
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
    Half_Type_Info type;
    Half_Ir_Move(Half_Ir_Exp l, Half_Ir_Exp r)
        :left(l), right(r) {}
};
