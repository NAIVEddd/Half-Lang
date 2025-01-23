#pragma once
#include<stack>
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

struct MemoryUsagePass : Function_Pass
{
    struct VarDef
    {
        size_t block_id;
        size_t exp_id;
        size_t var_id;
    };
    std::vector<Address> allocs;
    std::vector<VarDef> loads;
    std::vector<VarDef> stores;

    void Run(Half_Ir_Function& program) override;
    void CollectLoads(Half_Ir_Function &program, const std::map<Address, size_t>& allocs, std::vector<VarDef>& exps_loc);
    void CollectStores(Half_Ir_Function &program, const std::map<Address, size_t>& allocs, std::vector<VarDef>& exps_loc);
    ~MemoryUsagePass() override {}
};

struct RenameInstr
{
    RenameInstr(Half_Ir_Function& p, CFG& c, DominatorTree& d, MemoryUsagePass& m) : program(p), cfg(c), dt(d), mup(m) {};
    void Rename(size_t block_id, size_t var_id, std::stack<Temp::Label>& labels, std::map<size_t, Half_Ir_Phi>& phi_map, std::set<size_t> def_blocks, std::set<size_t> use_blocks);

    Half_Ir_Function& program;
    // map block_id to exp_index and new_exp
    std::map<size_t, std::map<size_t, Half_Ir_Exp>> exprs;
    CFG cfg;
    DominatorTree dt;
    MemoryUsagePass mup;
};

struct Mem2RegPass : Function_Pass
{
    void Run(Half_Ir_Function& program) override;
    ~Mem2RegPass() override {}
};

