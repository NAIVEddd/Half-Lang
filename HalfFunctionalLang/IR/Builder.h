#pragma once

#include"../Syntax/Base.h"
#include"BasicBlock.h"

//LLVM IR���м��ʾ����һ�ֵͼ��ġ���Ӳ���޹صı�����ԣ����ڱ�ʾ�������м�׶εĴ��롣LLVM IR ���ڲ���ʾ��Ҫ�����¼����ؼ������ɣ�
//1. ģ�飨Module��
//ģ���� LLVM IR �Ķ�����������������ȫ�ֱ����������ͷ��ű�һ��ģ��ͨ����Ӧһ��Դ�ļ���
//2. ������Function��
//������ LLVM IR �еĻ�����Ԫ������������ǩ�����������ͺͲ������ͣ��������壨�ɻ�������ɣ��Լ�����Ԫ���ݡ�
//3. �����飨BasicBlock��
//�������� LLVM IR �е�һ����Ҫ�����ʾһ��˳��ִ�е�ָ�ÿ�������鶼��һ����ڵ��һ�����ڵ㣬������ֻ���ڻ�����֮����ת��
//4. ָ�Instruction��
//ָ���� LLVM IR �е���Сִ�е�Ԫ����ʾ����Ĳ��������������㡢�ڴ���ʡ��������ȡ�
//5. ֵ��Value��
//ֵ�� LLVM IR �е�һ����������ʾָ��Ĳ������ͽ����ֵ�����ǳ��������������������ȡ�

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