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