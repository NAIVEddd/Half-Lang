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
    void allocate(std::vector<Graph>& gs, Liveness_Graph& liveness)
    {
        color.initialize(liveness);
        std::map<Temp::Label, int> colored = GetDefaultColor(gs, liveness);
        color.precolored(colored);
        auto color_map = color.AllocateRegisters();
        for (size_t i = 0; i < gs.size(); i++)
        {
            update(color_map, gs[i]);
        }
    }
    std::map<Temp::Label, int> GetDefaultColor(std::vector<Graph>& gs, Liveness_Graph& liveness)
    {
        // set default color for some registers
        auto regs = std::vector<std::string>{ "ecx", "edx", "r8d", "r9d" };
        auto ret_reg = "eax";
        auto reg_idx = std::vector<size_t>(regs.size(), -1);
        auto ret_idx = std::find(regNames.begin(), regNames.end(), ret_reg) - regNames.begin();
        for (size_t i = 0; i < regs.size(); i++)
        {
            // findout the index of the register
            reg_idx[i] = std::find(regNames.begin(), regNames.end(), regs[i]) - regNames.begin();
        }

        std::map<Temp::Label, int> color_map;
        for (auto& g : gs)
        {
            for (auto& n : g.Nodes)
            {
                if (auto pcall = std::get_if<AS_Call>(&n.info))
                {
                    _ASSERT(pcall->args.size() <= 4);
                    for (size_t i = 0; i < pcall->args.size(); ++i)
                    {
                        if (i < regs.size())
                        {
                            color_map[pcall->args[i]] = (int)reg_idx[i];
                        }
                    }
                }
            }
        }
        return color_map;
    }

    void update(const std::map<Temp::Label, int>& color_map, Graph& g)
    {
        for (size_t i = 0; i < g.Nodes.size(); i++)
        {
            auto& instr = g.Nodes[i].info;
            if (auto pop = std::get_if<AS_Oper>(&instr))
            {
                color_map.contains(pop->src) ? pop->src = Temp::Label(regNames[color_map.at(pop->src)]) : pop->src = pop->src;
                color_map.contains(pop->dst) ? pop->dst = Temp::Label(regNames[color_map.at(pop->dst)]) : pop->dst = pop->dst;
            }
            else if (auto pext = std::get_if<AS_Ext>(&instr))
            {
                color_map.contains(pext->src) ? pext->src = Temp::Label(regNames[color_map.at(pext->src)]) : pext->src = pext->src;
                color_map.contains(pext->dst) ? pext->dst = Temp::Label(regNames[color_map.at(pext->dst)]) : pext->dst = pext->dst;
            }
            else if (auto pmv = std::get_if<AS_Move>(&instr))
            {
                color_map.contains(pmv->src) ? pmv->src = Temp::Label(regNames[color_map.at(pmv->src)]) : pmv->src = pmv->src;
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(regNames[color_map.at(pmv->dst)]) : pmv->dst = pmv->dst;
            }
            else if (auto pmv = std::get_if<AS_Move_String>(&instr))
            {
                //color_map.contains(pmv->src) ? pmv->src = Temp::Label(regNames[color_map.at(pmv->src)]) : pmv->src = pmv->src;
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(regNames[color_map.at(pmv->dst)]) : pmv->dst = pmv->dst;
            }
            else if (auto pmv = std::get_if<AS_Move_Type>(&instr))
            {
                auto src_l = pmv->src.GetLabel();
                auto dst_l = pmv->dst.GetLabel();
                auto src = color_map.contains(src_l) ? Temp::Label(regNames[color_map.at(src_l)]) : src_l;
                auto dst = color_map.contains(dst_l) ? Temp::Label(regNames[color_map.at(dst_l)]) : dst_l;
                pmv->src.SetLabel(src);
                pmv->dst.SetLabel(dst);
            }
            else if (auto plea = std::get_if<AS_Lea>(&instr))
            {
                color_map.contains(plea->src) ? plea->src = Temp::Label(regNames[color_map.at(plea->src)]) : plea->src = plea->src;
                color_map.contains(plea->dst) ? plea->dst = Temp::Label(regNames[color_map.at(plea->dst)]) : plea->dst = plea->dst;
            }
            else if (auto pptr = std::get_if<AS_ElemPtr>(&instr))
            {
                color_map.contains(pptr->elem_ptr) ? pptr->elem_ptr = Temp::Label(regNames[color_map.at(pptr->elem_ptr)]) : pptr->elem_ptr = pptr->elem_ptr;
                color_map.contains(pptr->out_label) ? pptr->out_label = Temp::Label(regNames[color_map.at(pptr->out_label)]) : pptr->out_label = pptr->out_label;
            }
            else if (auto pload = std::get_if<AS_ElemLoad>(&instr))
            {
                color_map.contains(pload->elem_ptr) ? pload->elem_ptr = Temp::Label(regNames[color_map.at(pload->elem_ptr)]) : pload->elem_ptr = pload->elem_ptr;
                color_map.contains(pload->dst) ? pload->dst = Temp::Label(regNames[color_map.at(pload->dst)]) : pload->dst = pload->dst;
            }
            else if (auto pstore = std::get_if<AS_ElemStore>(&instr))
            {
                color_map.contains(pstore->src) ? pstore->src = Temp::Label(regNames[color_map.at(pstore->src)]) : pstore->src = pstore->src;
                color_map.contains(pstore->elem_ptr) ? pstore->elem_ptr = Temp::Label(regNames[color_map.at(pstore->elem_ptr)]) : pstore->elem_ptr = pstore->elem_ptr;
            }
            else if (auto pmv = std::get_if<AS_ArrayLoad>(&instr))
            {
                color_map.contains(pmv->src) ? pmv->src = Temp::Label(regNames[color_map.at(pmv->src)]) : pmv->src = pmv->src;
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(regNames[color_map.at(pmv->dst)]) : pmv->dst = pmv->dst;
            }
            else if (auto pmv = std::get_if<AS_ArrayStore>(&instr))
            {
                color_map.contains(pmv->src) ? pmv->src = Temp::Label(regNames[color_map.at(pmv->src)]) : pmv->src = pmv->src;
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(regNames[color_map.at(pmv->dst)]) : pmv->dst = pmv->dst;
            }
            else if (auto pcall = std::get_if<AS_Call>(&instr))
            {
                for (size_t i = 0; i < pcall->args.size(); i++)
                {
                    color_map.contains(pcall->args[i]) ?
                        pcall->args[i] = Temp::Label(regNames[color_map.at(pcall->args[i])]) :
                        pcall->args[i] = pcall->args[i];
                }
            }
        }
    }
};