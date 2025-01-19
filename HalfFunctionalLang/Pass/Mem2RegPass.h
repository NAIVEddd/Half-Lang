#pragma once
#include"Pass.h"

struct CFG
{
    using Graph = std::map<size_t, std::set<size_t>>;
    CFG(Half_Ir_Function& func);
    CFG(Graph& suc, Graph& pre, std::vector<Temp::Label>& lab)
        : successors(suc), predecessors(pre), labels(lab) {};
    std::vector<std::string> dump();
    std::map<size_t, std::set<size_t>> successors;
    std::map<size_t, std::set<size_t>> predecessors;
    std::vector<Temp::Label> labels;
};

struct DominatorTree
{
    DominatorTree(CFG &cfg);
    void CalculateDominator(CFG &cfg);
    void CalculateImmediateDominators();
    void CalculateDominatorsFrontiers(CFG& cfg);
    std::vector<std::string> dump();
    std::map<size_t, std::set<size_t>> dominators;
    std::map<size_t, size_t> idom;
    std::map<size_t, std::set<size_t>> children;
    std::map<size_t, std::set<size_t>> dominance_frontiers;
};


struct Mem2RegPass : Function_Pass
{
    void Run(Half_Ir_Function& program) override;
    ~Mem2RegPass() override {}
};

