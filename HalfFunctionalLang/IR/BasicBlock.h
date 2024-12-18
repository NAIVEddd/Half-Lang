#pragma once

#include"IR.h"
#include"Temp.h"
#include<string>
#include<vector>

struct Half_Ir_Exp;

struct Half_Ir_BasicBlock
{
    struct Iterator
    {
        std::vector<Half_Ir_Exp>::iterator it;
        Iterator(std::vector<Half_Ir_Exp>::iterator i) : it(i) {}
        Half_Ir_Exp& operator*()
        {
            return *it;
        }
        Iterator& operator++()
        {
            ++it;
            return *this;
        }
        bool operator==(const Iterator& i)
        {
            return it == i.it;
        }
        bool operator!=(const Iterator& i)
        {
            return it != i.it;
        }
    };
    Temp::Label label;
    std::vector<Half_Ir_Exp> exps;
    Half_Ir_BasicBlock(Temp::Label l = Temp::NewBlockLabel()) : label(l) {}
    void Rename(Temp::Label l)
    {
        label = l;
    }

    //std::optional<Half_Ir_Exp> GetTerminator()
    //{
    //    if (exps.empty())   // or exps.back().exp is Half_Ir_Jump(terminator)
    //    {
    //        return std::nullopt;
    //    }
    //    return exps.back();
    //}
};