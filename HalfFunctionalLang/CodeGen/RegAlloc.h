#pragma once
#include "Color.h"
#include "Liveness.h"
#include "Graph.h"

const std::vector<std::string> RegNames{ "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d"};

struct RegAlloc
{
    Color color;
    std::vector<std::string> regNames;
    RegAlloc(const std::vector<std::string>& names = RegNames) : regNames(names), color((int)names.size()) {}
    void allocate(Graph& g, Liveness& liveness)
    {
        color.initialize(liveness);
        setDefaultColor(g);
        color.allocate();
        update(g);
    }
    void setDefaultColor(Graph& g)
    {
        for (auto& instr : g.instrs)
        {
            if (auto pcall = std::get_if<AS_Call>(&instr))
            {
                _ASSERT(pcall->args.size() <= 4);
                auto regs = std::vector<std::string>{ "ecx", "edx", "r8d", "r9d" };
                auto reg_idx = std::vector<size_t>(regs.size(), -1);
                for (size_t i = 0; i < regs.size(); i++)
                {
                    // findout the index of the register
                    reg_idx[i] = std::find(regNames.begin(), regNames.end(), regs[i]) - regNames.begin();
                }
                for (size_t i = 0; i < pcall->args.size(); ++i)
                {
                    if (i < regs.size())
                    {
                        color.color[color.tempMap.get(pcall->args[i])] = (int)reg_idx[i];
                        //pcall->args[i] = Temp::Label(regs[i]);
                    }
                }
            }
        }
    }
    void update(Graph& g)
    {
        auto& tempMap = color.tempMap;
        for (size_t i = 0; i < g.instrs.size(); i++)
        {
            auto& instr = g.instrs[i];
            if (auto pop = std::get_if<AS_Oper>(&instr))
            {
                auto src = tempMap.find(pop->src) == -1 ? pop->src : Temp::Label(regNames[color.color[tempMap.get(pop->src)]]);
                auto dst = tempMap.find(pop->dst) == -1 ? pop->dst : Temp::Label(regNames[color.color[tempMap.get(pop->dst)]]);
                pop->src = src;
                pop->dst = dst;
            }
            else if (auto pmv = std::get_if<AS_Move>(&instr))
            {
                auto src = tempMap.find(pmv->src) == -1 ? pmv->src : Temp::Label(regNames[color.color[tempMap.get(pmv->src)]]);
                auto dst = tempMap.find(pmv->dst) == -1 ? pmv->dst : Temp::Label(regNames[color.color[tempMap.get(pmv->dst)]]);
                pmv->src = src;
                pmv->dst = dst;
            }
            else if (auto plea = std::get_if<AS_Lea>(&instr))
            {
                auto src = tempMap.find(plea->src) == -1 ? plea->src : Temp::Label(regNames[color.color[tempMap.get(plea->src)]]);
                auto dst = tempMap.find(plea->dst) == -1 ? plea->dst : Temp::Label(regNames[color.color[tempMap.get(plea->dst)]]);
                plea->src = src;
                plea->dst = dst;
            }
            else if (auto pptr = std::get_if<AS_ElemPtr>(&instr))
            {
                auto src = tempMap.find(pptr->elem_ptr) == -1 ? pptr->elem_ptr : Temp::Label(regNames[color.color[tempMap.get(pptr->elem_ptr)]]);
                auto dst = tempMap.find(pptr->out_label) == -1 ? pptr->out_label : Temp::Label(regNames[color.color[tempMap.get(pptr->out_label)]]);
                pptr->elem_ptr = src;
                pptr->out_label = dst;
            }
            else if (auto pload = std::get_if<AS_ElemLoad>(&instr))
            {
                auto src = tempMap.find(pload->elem_ptr) == -1 ? pload->elem_ptr : Temp::Label(regNames[color.color[tempMap.get(pload->elem_ptr)]]);
                auto dst = tempMap.find(pload->dst) == -1 ? pload->dst : Temp::Label(regNames[color.color[tempMap.get(pload->dst)]]);
                pload->elem_ptr = src;
                pload->dst = dst;
            }
            else if (auto pstore = std::get_if<AS_ElemStore>(&instr))
            {
                auto src = tempMap.find(pstore->src) == -1 ? pstore->src : Temp::Label(regNames[color.color[tempMap.get(pstore->src)]]);
                auto ptr = tempMap.find(pstore->elem_ptr) == -1 ? pstore->elem_ptr : Temp::Label(regNames[color.color[tempMap.get(pstore->elem_ptr)]]);
                pstore->src = src;
                pstore->elem_ptr = ptr;
            }
            else if (auto pmv = std::get_if<AS_ArrayLoad>(&instr))
            {
                auto src = tempMap.find(pmv->src) == -1 ? pmv->src : Temp::Label(regNames[color.color[tempMap.get(pmv->src)]]);
                auto dst = tempMap.find(pmv->dst) == -1 ? pmv->dst : Temp::Label(regNames[color.color[tempMap.get(pmv->dst)]]);
                pmv->src = src;
                pmv->dst = dst;
            }
            else if (auto pmv = std::get_if<AS_ArrayStore>(&instr))
            {
                auto src = tempMap.find(pmv->src) == -1 ? pmv->src : Temp::Label(regNames[color.color[tempMap.get(pmv->src)]]);
                auto dst = tempMap.find(pmv->dst) == -1 ? pmv->dst : Temp::Label(regNames[color.color[tempMap.get(pmv->dst)]]);
                pmv->src = src;
                pmv->dst = dst;
            }
            else if (auto pcall = std::get_if<AS_Call>(&instr))
            {
                for (size_t i = 0; i < pcall->args.size(); i++)
                {
                    auto arg = tempMap.find(pcall->args[i]) == -1 ? pcall->args[i] : Temp::Label(regNames[color.color[tempMap.get(pcall->args[i])]]);
                    pcall->args[i] = arg;
                }
            }
        }
        g.initialize(g.instrs);
    }
};