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
