#pragma once
#include<map>
#include<memory>
#include<string>
#include"../IR/IR.h"

template<typename T>
struct Pass
{
    virtual void Run(T& program) = 0;
    virtual ~Pass() noexcept {}
};

struct IR_Pass : Pass<Half_Ir_Exp>
{
    virtual void Run(Half_Ir_Exp& program) = 0;
    virtual ~IR_Pass() {}
};

struct Function_Pass : Pass<Half_Ir_Function>
{
    virtual void Run(Half_Ir_Function& program) = 0;
    virtual ~Function_Pass() {}
};

struct IR_Pass_Manager
{
    void AddPass(std::string nm, std::shared_ptr<IR_Pass> pass)
    {
        passes[nm] = pass;
    }

    std::map<std::string, std::shared_ptr<IR_Pass>> passes;
};