#pragma once

#include"../IR/IR.h"
#include"../IR/Temp.h"
#include"../IR/Builder.h"
#include<stack>

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

struct AS_Return
{
    char c;
};

using AS_Instr = std::variant<std::monostate, AS_StackAlloc, AS_Oper, AS_Move, AS_Jump, AS_Label, AS_Return>;

struct AS_Function
{
    std::string name;
    std::vector<AS_Instr> instrs;
    AS_Function(std::string n, std::vector<AS_Instr>& i)
        : name(n), instrs(i) {
    }
};

void MunchExp(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs);
void MunchExp(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs, std::stack<Temp::Label>& temps);
void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Instr>& instrs);
void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs);
std::string to_string(const AS_Instr& instr);
