#include "Mem2RegPass.h"
#include <algorithm>
#include <iterator>
#include <cassert>

CFG::CFG(Half_Ir_Function& func)
{
    labels.resize(func.blocks.size());
    for (size_t i = 0; i < func.blocks.size(); i++)
    {
        labels[i] = func.blocks[i].label;
    }
    for (size_t i = 0; i < func.blocks.size(); i++)
    {
        auto& block = func.blocks[i];
        std::set<size_t> succ = { block.succs.begin(), block.succs.end() };
        std::set<size_t> pred = { block.preds.begin(), block.preds.end() };
        successors[i] = succ;
        predecessors[i] = pred;
    }
}

std::vector<std::string> CFG::dump()
{
    for (size_t i = 0; i < labels.size(); i++)
    {
        printf("block %s\n", std::to_string(i).c_str());
        printf("    succs: ");
        for (auto& s : successors[i])
        {
            printf("%s, ", std::to_string(s).c_str());
        }
        printf("\n");
        printf("    preds: ");
        for (auto& p : predecessors[i])
        {
            printf("%s, ", std::to_string(p).c_str());
        }
        printf("\n");
        /*printf("block %s\n", labels[i].l.c_str());
        printf("    succs: ");
        for (auto& s : successors[i])
        {
            printf("%s, ", labels[s].l.c_str());
        }
        printf("\n");
        printf("    preds: ");
        for (auto& p : predecessors[i])
        {
            printf("%s, ", labels[p].l.c_str());
        }
        printf("\n");*/
    }
    return std::vector<std::string>();
}

DominatorTree::DominatorTree(CFG &cfg)
{
    CalculateDominator(cfg);
    CalculateImmediateDominators();
    CalculateDominatorsFrontiers(cfg);
}

void DominatorTree::CalculateDominator(CFG &cfg)
{
    // The entry block (block 0) is dominated only by itself
    dominators[0] = {0};
    std::set<size_t> all_dom;
    for (size_t i = 0; i < cfg.labels.size(); i++)
    {
        all_dom.insert(i);
    }
    for (size_t i = 1; i < cfg.labels.size(); i++)
    {
        dominators[i] = all_dom;
    }

    bool changed = true;
    while (changed)
    {
        changed = false;
        for (size_t i = 1; i < cfg.labels.size(); i++)
        {
            std::set<size_t> new_doms = all_dom;
            for (auto pred : cfg.predecessors[i])
            {
                std::set<size_t> intersection;
                std::set_intersection(dominators[pred].begin(), dominators[pred].end(),
                                      new_doms.begin(), new_doms.end(),
                                      std::inserter(intersection, intersection.begin()));
                new_doms = intersection;
            }
            new_doms.insert(i);

            if (new_doms != dominators[i])
            {
                dominators[i] = new_doms;
                changed = true;
            }
        }
    }
}

void DominatorTree::CalculateImmediateDominators()
{
    for (const auto& [block, doms] : dominators)
    {
        std::set<size_t> candidates = doms;
        candidates.erase(block);

        for (auto dom : doms)
        {
            if (dom == block)
            {
                continue;
            }
            for (auto pred : doms)
            {
                if (pred == dom || pred == block)
                {
                    continue;
                }
                if (dominators[pred].find(dom) != dominators[pred].end())
                {
                    candidates.erase(dom);
                    break;
                }
            }
        }

        if (candidates.size() == 1)
        {
            idom[block] = *candidates.begin();
        }
    }

    for (const auto& [block, dom] : idom)
    {
        children[dom].insert(block);
    }
}

void DominatorTree::CalculateDominatorsFrontiers(CFG& cfg)
{
    for (const auto& [block, preds] : cfg.predecessors)
    {
        if (preds.size() < 2)
        {
            continue;
        }
        for (auto pred : preds)
        {
            size_t runner = pred;
            while (runner != idom.at(block))
            {
                dominance_frontiers[runner].insert(block);
                runner = idom.at(runner);
            }
        }
    }
}

std::vector<std::string> DominatorTree::dump()
{
    for (size_t i = 0; i < dominators.size(); i++)
    {
        printf("block %s\n", std::to_string(i).c_str());
        /*printf("    dominators: ");
        for (auto& d : dominators[i])
        {
            printf("%s, ", std::to_string(d).c_str());
        }*/
        if (idom.find(i) == idom.end())
        {
            printf("\n    immediate dominator: none\n");
        }
        else
        {
            printf("\n    immediate dominator: %s\n", std::to_string(idom[i]).c_str());
        }
        if (children.find(i) == children.end())
        {
            printf("    children: none\n");
        }
        else
        {
            printf("    children: ");
            for (auto& c : children[i])
            {
                printf("%s, ", std::to_string(c).c_str());
            }
            printf("\n");
        }
        if (dominance_frontiers.find(i) == dominance_frontiers.end())
        {
            printf("    dominance frontiers: none\n");
        }
        else
        {
            printf("    dominance frontiers: ");
            for (auto& df : dominance_frontiers[i])
            {
                printf("%s, ", std::to_string(df).c_str());
            }
            printf("\n");
        }
        printf("\n");
    }
    return std::vector<std::string>();
}

void MemoryUsagePass::Run(Half_Ir_Function &program)
{
    std::map<Address, size_t> allocs_map;
    for (auto& exp : program.alloc.exps)
    {
        if(auto palloc = std::get_if<std::shared_ptr<Half_Ir_Alloca>>(&exp.exp))
        {
            auto alloc = **palloc;
            auto sz = alloc.out_address.type.GetSize();
            if (sz == 4 || sz == 8)
            {
                allocs_map.insert({alloc.out_address, allocs_map.size()});
            }
        }
    }
    // move allocs to vector
    allocs.resize(allocs_map.size());
    for (auto& [addr, id] : allocs_map)
    {
        allocs[id] = addr;
    }

    CollectLoads(program, allocs_map, loads);
    CollectStores(program, allocs_map, stores);
}

void MemoryUsagePass::CollectLoads(Half_Ir_Function &program, const std::map<Address, size_t> &allocs, std::vector<VarDef> &exps_loc)
{
    for(size_t i = 0; i < program.blocks.size(); ++i)
    {
        auto& block = program.blocks[i];
        for(size_t j = 0; j < block.exps.size(); ++j)
        {
            auto& exp = block.exps[j];
            if(auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(&exp.exp))
            {
                auto load = **pload;
                if(allocs.find(load.address) != allocs.end())
                {
                    auto var_id = allocs.at(load.address);
                    exps_loc.push_back({i, j, var_id});
                }
            }
        }
    }
}

void MemoryUsagePass::CollectStores(Half_Ir_Function &program, const std::map<Address, size_t> &allocs, std::vector<VarDef> &exps_loc)
{
    for(size_t i = 0; i < program.blocks.size(); ++i)
    {
        auto& block = program.blocks[i];
        for(size_t j = 0; j < block.exps.size(); ++j)
        {
            auto& exp = block.exps[j];
            if(auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(&exp.exp))
            {
                auto store = **pstore;
                if(allocs.find(store.address) != allocs.end())
                {
                    auto var_id = allocs.at(store.address);
                    exps_loc.push_back({i, j, var_id});
                }
            }
        }
    }
}

void RenameInstr::Rename(size_t block_id, size_t var_id, std::stack<Temp::Label>& labels, std::map<size_t, Half_Ir_Phi>& phi_map, std::set<size_t> def_blocks, std::set<size_t> use_blocks)
{
    auto& block = program.blocks[block_id];
    auto& block_label = cfg.labels[block_id];
    auto stack_count = labels.size();

    auto& new_exps = exprs[block_id];

    // push load/store/phi result to stack
    auto phi_iter = phi_map.find(block_id);
    if (phi_iter != phi_map.end())
    {
        auto& phi = phi_iter->second;
        auto& result = phi.result;
        labels.push(result.name);
    }

    if (def_blocks.find(block_id) != def_blocks.end() || use_blocks.find(block_id) != use_blocks.end())
    {
        std::set<size_t> def_worklist;
        std::set<size_t> use_worklist;
        for (auto& [b_id, e_id, v_id] : mup.loads)
        {
            if (var_id == v_id && block_id == b_id)
            {
                use_worklist.insert(e_id);
            }
        }
        for (auto& [b_id, e_id, v_id] : mup.stores)
        {
            if (var_id == v_id && block_id == b_id)
            {
                def_worklist.insert(e_id);
            }
        }

        for (size_t i = 0; i < block.exps.size(); ++i)
        {
            auto& exp = block.exps[i];
            if (use_worklist.find(i) != use_worklist.end())
            {
                if (labels.empty())
                {
                    continue;
                }
                if (auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(&exp.exp))
                {
                    auto& load = **pload;
                    Half_Ir_Name val = Half_Ir_Name(labels.top());
                    Half_Ir_Name out = Half_Ir_Name(load.out_register.reg);
                    Half_Ir_Move mv(out, val);
                    mv.type = load.out_register.type;
                    new_exps[i] = mv;
                    labels.push(load.out_register.reg);
                }
            }
            if (def_worklist.find(i) != def_worklist.end())
            {
                if (auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(&exp.exp))
                {
                    auto& store = **pstore;

                    _ASSERT(store.value.value.index() == 1);
                    if (store.value.value.index() == 1)
                    {
                        auto& reg = std::get<Register>(store.value.value);
                        labels.push(reg.reg);
                    }
                }
            }
        }
    }
    

    for (auto succ : cfg.successors.at(block_id))
    {
        if (phi_map.find(succ) != phi_map.end())
        {
            auto& phi = phi_map[succ];
            Half_Ir_Name n(labels.top());
            phi.Insert(n, block_label);
        }
    }

    if (dt.children.find(block_id) != dt.children.end())
    {
        for (auto child : dt.children.at(block_id))
        {
            Rename(child, var_id, labels, phi_map, def_blocks, use_blocks);
        }
    }

    // clear stack
    while (labels.size() > stack_count)
    {
        labels.pop();
    }
}

void Mem2RegPass::Run(Half_Ir_Function &program)
{
    CFG cfg(program);
    DominatorTree dt(cfg);
    MemoryUsagePass mup;
    mup.Run(program);

    // dump program's allocation and store/load
    auto& blocks = program.blocks;
    for (size_t i = 0; i < mup.allocs.size(); i++)
    {
        auto& addr = mup.allocs[i];
        auto str = addr.real_address->base.l + "(" + std::to_string(addr.real_address->offset) + ")";
        printf("alloc at %s, var_id %zd\n", str.c_str(), i);
    }
    for(auto& [i, j, v] : mup.loads)
    {
        auto& block = blocks[i];
        printf("load at block(%zd) %s, exp %zd, var_id %zd\n", i, block.label.l.c_str(), j, v);
    }
    for(auto& [i, j, v] : mup.stores)
    {
        auto& block = blocks[i];
        printf("store at block(%zd) %s, exp %zd, var_id %zd\n", i, block.label.l.c_str(), j, v);
    }

    std::map<size_t, std::vector<Half_Ir_Exp>> phi_exp_map;
    std::map<size_t, std::vector<Half_Ir_Exp>> block_map;

    // find out which block to insert phi functions
    for (size_t i = 0; i < mup.allocs.size(); i++)
    {
        auto& addr = mup.allocs[i];
        std::set<size_t> def_blocks;
        std::set<size_t> use_blocks;
        for(auto& [block_id, exp_id, var_id] : mup.loads)
        {
            if(var_id == i)
            {
                use_blocks.insert(block_id);
            }
        }
        for(auto& [block_id, exp_id, var_id] : mup.stores)
        {
            if(var_id == i)
            {
                def_blocks.insert(block_id);
            }
        }

        // if a var used in one block and defined in this block, no need to insert phi functions
        /*if (def_blocks.size() == 1 && use_blocks.size() == 1 && *def_blocks.begin() == *use_blocks.begin())
        {
            def_blocks.clear();
        }*/

        std::set<size_t> phi_blocks;
        // if a var used in one block and defined in this block, no need to insert phi functions
        if (!(def_blocks.size() == 1 && use_blocks.size() == 1 && *def_blocks.begin() == *use_blocks.begin()))
        {
            std::set<size_t> worklist = def_blocks;
            while (!worklist.empty())
            {
                size_t block_id = *worklist.begin();
                worklist.erase(worklist.begin());
                if (dt.dominance_frontiers.find(block_id) != dt.dominance_frontiers.end())
                {
                    for (auto id : dt.dominance_frontiers.at(block_id))
                    {
                        if (phi_blocks.find(id) == phi_blocks.end())
                        {
                            phi_blocks.insert(id);
                            worklist.insert(id);
                        }
                    }
                }
            }
        }
        
        // insert phi functions
        std::map<size_t, Half_Ir_Phi> phi_map;
        for (auto id : phi_blocks)
        {
            Half_Ir_Phi phi(Temp::NewLabel());
            phi_map[id] = phi;
        }


        std::stack<Temp::Label> labels;
        RenameInstr ri(program, cfg, dt, mup);
        ri.Rename(0, i, labels, phi_map, def_blocks, use_blocks);
        // copy new exps to block
        for (auto& [block_id, phi] : phi_map)
        {
            phi_exp_map[block_id].push_back(Half_Ir_Exp(phi));
        }
        for (auto& [block_id, new_exps] : ri.exprs)
        {
            if (new_exps.size() == 0)
            {
                continue;
            }
            if (block_map.find(block_id) == block_map.end())
            {
                block_map[block_id] = blocks[block_id].exps;
            }
            auto& vec = block_map[block_id];
            for (auto& [exp_id, new_exp] : new_exps)
            {
                vec[exp_id] = new_exp;
            }
        }
        for (auto& exps : ri.exprs)
        {
            for (auto& exp : exps.second)
            {
                printf("block %zd, exp %zd: \n", exps.first, exp.first);
            }
        }
        /*for (auto id : worklist)
        {
            if (dt.dominance_frontiers.find(id) != dt.dominance_frontiers.end())
            {
                for (auto df : dt.dominance_frontiers.at(id))
                {
                    phi_blocks.insert(df);
                }
            }
        }*/
        printf("var %zd, phi blocks: ", i);
        for(auto& block_id : phi_blocks)
        {
            printf("%zd : %s, ", block_id, blocks[block_id].label.l.c_str());
        }
        printf("\n");
    }
    
    // insert exp to phi vectors
    for (auto& [block_id, exps] : phi_exp_map)
    {
        if (block_map.find(block_id) != block_map.end())
        {
            auto& vec = block_map[block_id];
            for (auto& exp : vec)
            {
                exps.push_back(exp);
            }
            block_map.erase(block_id);
        }
        else
        {
            auto& vec = blocks[block_id].exps;
            for (auto& exp : vec)
            {
                exps.push_back(exp);
            }
        }
    }
    
    // erase store exps

    // set new exps to blocks
    for (auto& [block_id, exps] : phi_exp_map)
    {
        auto& block = blocks[block_id];
        block.exps = exps;
    }
    for (auto& [block_id, exps] : block_map)
    {
        auto& block = blocks[block_id];
        block.exps = exps;
    }
}
