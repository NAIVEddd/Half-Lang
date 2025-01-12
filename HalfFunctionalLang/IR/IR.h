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

struct Address
{   // format as offset(base)
    Half_Type_Info type;    // TODO : is this a element type or a pointer type
    Temp::Label base;
    ptrdiff_t offset;
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
            return std::get<Address>(value).type;
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
            return std::get<Address>(value).base;
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
            std::get<Address>(value).base = l;
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
    Half_Ir_Load(Address a) : address(a), out_register(Register{ a.type, Temp::NewLabel() }) {}
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
    Value value;
    Half_Type_Info type;
    Temp::Label out_label;
    Half_Ir_Ext(Value v, Half_Type_Info t, Temp::Label l = Temp::NewLabel())
        : value(v), type(t), out_label(l) {}
    Half_Ir_Ext(const Half_Ir_Ext& e) : value(e.value), type(e.type), out_label(e.out_label) {}
    Value GetResult() const
    {
        return Value(Register{ type, out_label });
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
    ptrdiff_t offset = 0;
    Temp::Label base;
    Half_Type_Info source_element_type;
    Half_Type_Info result_element_type;
    std::vector<Indexer> in_indexs;
    std::vector<size_t> elem_sizes;
    std::vector<Half_Ir_Exp> in_index;
    std::vector<Temp::Label> exp_out_labels;
    Temp::Label out_label;
    Half_Ir_GetElementPtr() = default;
    Half_Ir_GetElementPtr(Temp::Label b, Temp::Label l = Temp::NewLabel())
        : base(b), out_label(l) {}
    Half_Ir_GetElementPtr(Address a, Temp::Label l = Temp::NewLabel())
        : offset(a.offset), base(a.base), out_label(l)
        , source_element_type(Half_Type_Info::PointerType(a.type))
    {
        result_element_type = source_element_type;
    }
    Half_Ir_GetElementPtr(Register reg, Temp::Label l = Temp::NewLabel())
        : offset(0), base(reg.reg), out_label(l)
        , source_element_type(reg.type)
    {
        result_element_type = source_element_type;
    }
    Half_Ir_GetElementPtr(const Half_Ir_GetElementPtr& g)
        : offset(g.offset), base(g.base)
        , source_element_type(g.source_element_type), result_element_type(g.result_element_type)
        , in_indexs(g.in_indexs), elem_sizes(g.elem_sizes), in_index(g.in_index), exp_out_labels(g.exp_out_labels), out_label(g.out_label) {
    }

    void AddIndex(Indexer idx)
    {
        in_indexs.push_back(idx);
    }

    Half_Type_Info GetResultType() const
    {

        return result_element_type;
    }

    /*Address GetResult() const
    {
        return Address{ Half_Type_Info::BasicType::BasicT::Int, out_label, offset };
    }*/

    size_t GetOffset() const
    {
        _ASSERT(elem_sizes.size() == in_index.size());
        if (!is_const_offset())
        {
            return -1;
        }
        ptrdiff_t sz = offset;
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
        return Value(Register{ Half_Type_Info::PointerType(ptr.type), out_label });
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
            //return Value(Address{ left.type, left.reg, 0 });
        }
        else if (left.type.is_basic() && right.type.is_pointer())
        {
            return Value(Register{ right.type, out_label });
            //return Value(Address{ right.type, right.reg, 0 });
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
    Temp::Label fun_name;
    std::vector<Half_Ir_Name> args;
    Temp::Label out_label;
    Half_Ir_Call(Temp::Label call_label, Temp::Label n, std::vector<Half_Ir_Name>& a)
        : fun_name(n), args(a), out_label(call_label) {}
    Half_Ir_Call(const Half_Ir_Call& c)
        : fun_name(c.fun_name), args(c.args), out_label(c.out_label) {}
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
    Half_Ir_Move(Half_Ir_Exp l, Half_Ir_Exp r)
        :left(l), right(r) {}
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