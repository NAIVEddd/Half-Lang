#pragma once
#include "Color.h"
#include "Liveness.h"
#include "Graph.h"
#include "Register.h"

const std::vector<std::string> RegNames{ "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d"};

struct RegAlloc
{
    Color color;
    std::vector<std::string> regNames;
    std::set<Temp::Label> args_set;
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
                        args_set.insert(pcall->args[i]);
                    }
                    color_map[pcall->out_register.reg] = (int)ret_idx;
                }
            }
        }
        return color_map;
    }

    void update(const std::map<Temp::Label, int>& color_map, Graph& g)
    {
        AS_Register registers;
        for (size_t i = 0; i < g.Nodes.size(); i++)
        {
            auto& instr = g.Nodes[i].info;
            if (auto pop = std::get_if<AS_Oper>(&instr))
            {
                Half_Type_Info ty;
                // set ty = float if the operator is float
                if (pop->is_float)
                {
                    ty = Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Float);
                }
                else if (pop->sz == 8)
                {
                    ty = Half_Type_Info::PointerType(Half_Type_Info::BasicType::BasicT::Int);
                }
                else if (pop->sz == 4)
                {
                    ty = Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Int);
                }
                color_map.contains(pop->src) ? pop->src = Temp::Label(registers.get_register(color_map.at(pop->src), ty)) : pop->src = pop->src;
                color_map.contains(pop->dst) ? pop->dst = Temp::Label(registers.get_register(color_map.at(pop->dst), ty)) : pop->dst = pop->dst;
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
                if (args_set.contains(pmv->dst))
                {
                    Half_Type_Info ty = Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::String);
                    color_map.contains(pmv->src) ? pmv->src = Temp::Label(registers.get_arg_register(color_map.at(pmv->dst), ty)) : pmv->src = pmv->src;
                    return;
                }
                //color_map.contains(pmv->src) ? pmv->src = Temp::Label(regNames[color_map.at(pmv->src)]) : pmv->src = pmv->src;
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(regNames[color_map.at(pmv->dst)]) : pmv->dst = pmv->dst;
            }
            else if (auto pmv = std::get_if<AS_Move_Float>(&instr))
            {
                Half_Type_Info ty = Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Float);
                color_map.contains(pmv->dst) ? pmv->dst = Temp::Label(registers.get_register(color_map.at(pmv->dst), ty)) : pmv->dst = pmv->dst;
            }
            else if (auto pmv = std::get_if<AS_Move_Type>(&instr))
            {
                auto src_l = pmv->src.GetLabel();
                auto dst_l = pmv->dst.GetLabel();
                Half_Type_Info ty = pmv->src.GetType();
                auto src = color_map.contains(src_l) ?
                    (args_set.contains(src_l) ?
                        Temp::Label(registers.get_arg_register(color_map.at(src_l), ty)) :
                        registers.get_register(color_map.at(src_l), ty)) :
                    src_l;
                ty = pmv->dst.GetType();
                auto dst = color_map.contains(dst_l) ?
                    (args_set.contains(dst_l) ?
                        Temp::Label(registers.get_arg_register(color_map.at(dst_l), ty)) :
                        registers.get_register(color_map.at(dst_l), ty)) :
                    dst_l;
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
                    auto l = pcall->args_new[i].GetLabel();
                    auto ty = pcall->args_new[i].GetType();
                    color_map.contains(l) ?
                        pcall->args[i] = Temp::Label(registers.get_arg_register(color_map.at(l), ty)) :
                        pcall->args[i] = pcall->args[i];
                }

                Half_Type_Info ty = pcall->out_register.type;
                color_map.contains(pcall->out_register.reg) ?
                    pcall->out_register.reg = Temp::Label(registers.get_register(color_map.at(pcall->out_register.reg), ty)) :
                    pcall->out_register.reg = pcall->out_register.reg;
            }
        }
    }
};