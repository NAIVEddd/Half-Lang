#pragma once
#include "Color.h"
#include "Liveness.h"
#include "Graph.h"

const std::vector<std::string> RegNames{ "ebx", "ecx", "edx", "esi", "edi" };

struct RegAlloc
{
    Color color;
    std::vector<std::string> regNames;
    RegAlloc(const std::vector<std::string>& names = RegNames) : regNames(names), color((int)names.size()) {}
    void allocate(Graph& g, Liveness& liveness)
    {
        color.initialize(liveness);
        color.allocate();
        update(g);
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
        }
        g.initialize(g.instrs);
    }
};