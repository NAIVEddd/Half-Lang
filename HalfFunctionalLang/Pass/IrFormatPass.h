#pragma once
#include"Pass.h"

struct IR_Print_Pass : IR_Pass
{
    void dump(std::vector<std::string>& out);  // print target : file or stdout

    virtual void Run(Half_Ir_Exp& program);
    virtual ~IR_Print_Pass() noexcept {}

private:
    void RunOn(Half_Ir_Alloca& alloc);
    void RunOn(Half_Ir_Load& load);
    void RunOn(Half_Ir_Store& store);
    void RunOn(Half_Ir_Ext& ext);
    void RunOn(Half_Ir_Const& c);
    void RunOn(Half_Ir_GetElementPtr& gep);
    void RunOn(Half_Ir_FetchPtr& fetch);
    void RunOn(Half_Ir_Return& ret);
    void RunOn(Half_Ir_Function& func);
    void RunOn(Half_Ir_Name& name);
    void RunOn(Half_Ir_Value& value);
    void RunOn(Half_Ir_Call& call);
    void RunOn(Half_Ir_BinOp& binop);
    void RunOn(Half_Ir_Compare& compare);
    void RunOn(Half_Ir_Branch& branch);
    void RunOn(Half_Ir_Move& move);
    void RunOn(Half_Ir_Label& label);
    void RunOn(Half_Ir_Phi& phi);
    void RunOn(Half_Ir_Jump& jump);
    void RunOn(Temp::Label& label);
    int indent = 0;
    std::vector<std::string> lines;
};