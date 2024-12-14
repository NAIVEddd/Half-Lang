#pragma once

#include"../Syntax/Base.h"
#include"BasicBlock.h"

//LLVM IR（中间表示）是一种低级的、与硬件无关的编程语言，用于表示编译器中间阶段的代码。LLVM IR 的内部表示主要由以下几个关键组件组成：
//1. 模块（Module）
//模块是 LLVM IR 的顶级容器，包含所有全局变量、函数和符号表。一个模块通常对应一个源文件。
//2. 函数（Function）
//函数是 LLVM IR 中的基本单元，包含函数的签名（返回类型和参数类型）、函数体（由基本块组成）以及其他元数据。
//3. 基本块（BasicBlock）
//基本块是 LLVM IR 中的一个重要概念，表示一组顺序执行的指令。每个基本块都有一个入口点和一个出口点，控制流只能在基本块之间跳转。
//4. 指令（Instruction）
//指令是 LLVM IR 中的最小执行单元，表示具体的操作，如算术运算、内存访问、控制流等。
//5. 值（Value）
//值是 LLVM IR 中的一个抽象概念，表示指令的操作数和结果。值可以是常量、变量、函数参数等。

struct Builder
{
    std::vector<Half_Ir_BasicBlock> blocks;
    size_t insert_point;

    Builder() : insert_point(0)
    {
        NewBlock();
    }

    void SetInsertPoint(size_t i)
    {
        insert_point = i;
    }

    size_t NewBlock()
    {
        blocks.push_back(Half_Ir_BasicBlock());
        return blocks.size() - 1;
    }
    Half_Ir_BasicBlock& GetBlock(size_t i)
    {
        return blocks[i];
    }
    void AddExp(Half_Ir_Exp exp)
    {
        blocks[insert_point].exps.push_back(exp);
    }
    void AddExp(size_t i, Half_Ir_Exp exp)
    {
        blocks[i].exps.push_back(exp);
    }
};