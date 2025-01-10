#pragma once

#include"../IR/IR.h"
#include"../IR/Temp.h"
#include"../IR/Builder.h"
#include<stack>

struct AS_String
{
    Temp::Label label;
    std::string str;
    AS_String(Temp::Label l, std::string s) : label(l), str(s) {}
    AS_String(const AS_String& o) : str(o.str), label(o.label) {}
};

struct AS_StackAlloc
{
    size_t bytes;
    AS_StackAlloc(size_t sz) : bytes(sz) {}
    AS_StackAlloc(const AS_StackAlloc& a) : bytes(a.bytes) {}
};

struct AS_Oper
{
    std::string assem;
    Temp::Label dst;
    Temp::Label src;
    AS_Oper(std::string as, Temp::Label d, Temp::Label s)
        : assem(as), dst(d), src(s) {}
    AS_Oper(const AS_Oper& o)
        : assem(o.assem), dst(o.dst), src(o.src) {}
};

struct AS_Lea
{
    Temp::Label dst;
    Temp::Label src;
    ptrdiff_t offset;
    AS_Lea(Temp::Label d, Temp::Label s, size_t off)
        : dst(d), src(s), offset(off) {}
    AS_Lea(const AS_Lea& o)
        : dst(o.dst), src(o.src), offset(o.offset) {}
};

struct AS_Move
{
    Temp::Label dst;
    Temp::Label src;
    AS_Move(Temp::Label d, Temp::Label s)
        : dst(d), src(s) {}
    AS_Move(const AS_Move& o)
        : dst(o.dst), src(o.src) {}
    AS_Move(const Half_Ir_Name& d, const Half_Ir_Name& s)
        : dst(d.name), src(s.name) {}
};

struct AS_Move_String
{
    Temp::Label dst;
    Temp::Label src;
    AS_Move_String(Temp::Label d, Temp::Label s)
        : dst(d), src(s) {}
    AS_Move_String(const AS_Move_String& o)
        : dst(o.dst), src(o.src) {}
};

struct AS_Move_Type
{
    Value dst;
    Value src;
    AS_Move_Type(Value d, Value s)
        : dst(d), src(s) {}
    AS_Move_Type(const AS_Move_Type& o)
        : dst(o.dst), src(o.src) {}
};

struct AS_ElemPtr
{
    // elem_ptr = elem_ptr + elem_offset
    size_t elem_offset;
    Temp::Label elem_ptr;
    Temp::Label out_label;
    AS_ElemPtr(size_t off, Temp::Label ptr, Temp::Label l)
        : elem_offset(off), elem_ptr(ptr), out_label(l) {}
    AS_ElemPtr(const AS_ElemPtr& a)
        : elem_offset(a.elem_offset), elem_ptr(a.elem_ptr), out_label(a.out_label) {}
};

struct AS_ElemLoad
{
    // load data from memory
    //  movl elem_offset(elem_ptr), dst
    size_t elem_offset;
    size_t size;
    Temp::Label elem_ptr;
    Temp::Label dst;
    AS_ElemLoad(size_t off, size_t sz, Temp::Label ptr, Temp::Label l)
        : elem_offset(off), size(sz), elem_ptr(ptr), dst(l) {}
    AS_ElemLoad(const AS_ElemLoad& a)
        : elem_offset(a.elem_offset), size(a.size), elem_ptr(a.elem_ptr), dst(a.dst) {}
};

struct AS_ElemStore
{
    // movl src, offset(elem_ptr)
    size_t elem_offset;
    size_t size;
    Temp::Label elem_ptr;
    Temp::Label src;
    AS_ElemStore(size_t off, size_t sz, Temp::Label ptr, Temp::Label s)
        : elem_offset(off), size(sz), elem_ptr(ptr), src(s) {}
    AS_ElemStore(const AS_ElemStore& o)
        : elem_offset(o.elem_offset), size(o.size), elem_ptr(o.elem_ptr), src(o.src) {}
};

// load data from memory
// movl offset(%rsp, src, 4), dst
struct AS_ArrayLoad
{
    Temp::Label dst;
    Temp::Label src;
    size_t offset;
    size_t data_size;
    AS_ArrayLoad(Temp::Label d, Temp::Label s, size_t off, size_t sz)
        : dst(d), src(s), offset(off), data_size(sz) {}
    AS_ArrayLoad(const AS_ArrayLoad& o)
        : dst(o.dst), src(o.src), offset(o.offset), data_size(o.data_size) {}
};

// store data to memory
// movl src, offset(%rsp, dst, 4)
struct AS_ArrayStore
{
    Temp::Label dst;
    Temp::Label src;
    size_t offset;
    size_t data_size;
    AS_ArrayStore(Temp::Label d, Temp::Label s, size_t off, size_t sz)
        : dst(d), src(s), offset(off), data_size(sz) {}
    AS_ArrayStore(const AS_ArrayStore& o)
        : dst(o.dst), src(o.src), offset(o.offset), data_size(o.data_size) {}
};

struct AS_Label
{
    Temp::Label label;
    AS_Label(Temp::Label l) : label(l) {}
    AS_Label(const AS_Label& o) : label(o.label) {}
};

struct AS_Jump
{
    using Oper = Half_Ir_Compare::Oper;
    std::string jump;
    Temp::Label target;
    AS_Jump(std::string j, Temp::Label t) : jump(std::move(j)), target(std::move(t)) {}
    AS_Jump(Oper op, Temp::Label t)
        : jump(GetOperString(op)), target(std::move(t)) {}
    AS_Jump(const AS_Jump& o) : jump(o.jump), target(o.target) {}
    static std::string GetOperString(Oper op)
    {
        switch (op)
        {
        case Half_Ir_BinOp::Oper::Less:
            return "jl";
        case Half_Ir_BinOp::Oper::LessEqual:
            return "jle";
        case Half_Ir_BinOp::Oper::Greater:
            return "jg";
        case Half_Ir_BinOp::Oper::GreaterEqual:
            return "jge";
        case Half_Ir_BinOp::Oper::Equal:
            return "je";
        case Half_Ir_BinOp::Oper::NotEqual:
            return "jne";
        default:
            break;
        }
        return "unknown";
    }
};

struct AS_Call
{
    Temp::Label fun_name;
    std::vector<Temp::Label> args;
    AS_Call(Temp::Label f, std::vector<Temp::Label>& a)
        : fun_name(f), args(a) {}
    AS_Call(const AS_Call& o)
        : fun_name(o.fun_name), args(o.args) {}
};

struct AS_Return
{
    size_t bytes;
    AS_Return() : bytes(0) {}
    AS_Return(size_t cc) : bytes(cc) {}
    AS_Return(const AS_Return& o) : bytes(o.bytes) {}
};

using AS_Instr = std::variant<std::monostate, AS_String, AS_StackAlloc, AS_Oper, AS_Move, AS_Move_String, AS_Move_Type, AS_Lea, AS_ElemPtr, AS_ElemLoad, AS_ElemStore, AS_ArrayLoad, AS_ArrayStore, AS_Jump, AS_Label, AS_Call, AS_Return>;

struct AS_Function
{
    std::string name;
    std::vector<AS_Instr> instrs;
    AS_Function(std::string n, std::vector<AS_Instr>& i)
        : name(n), instrs(i) {
    }
};

void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Instr>& instrs);
void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs);
std::string to_string(const AS_Instr& instr);
