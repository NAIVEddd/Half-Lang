#include "Liveness.h"
#include <iostream>

void Liveness_InBlock::rinitialize(const Graph& g)
{
    // insert all nodes use and def into temps
    std::map<Temp::Label, size_t> labelmap;
    std::set<Temp::Label> tempset;
    for (auto& node : g.Nodes)
    {
        for (auto& l : node.Use())
        {
            if (labelmap.find(l) == labelmap.end())
            {
                labelmap.insert({ l, tempset.size() });
                tempset.insert(l);
                temps.push_back(l);
            }
        }
        for (auto& l : node.Def())
        {
            if (labelmap.find(l) == labelmap.end())
            {
                labelmap.insert({ l, tempset.size() });
                tempset.insert(l);
                temps.push_back(l);
            }
        }
    }

    // initialize in and out
    in.clear();
    out.clear();
    in.resize(g.Nodes.size());
    out.resize(g.Nodes.size());
    bool change = true;
    while (change)
    {
        change = false;
        for (int iter = (int)g.Nodes.size() - 1; iter >= 0; iter--)
        {
            auto& n = g.Nodes[iter];
            auto& in = this->in[iter];
            auto& out = this->out[iter];
            auto succ = n.Succ();
            auto use = n.Use();
            auto def = n.Def();
            std::set<size_t> defset;
            for (auto& d : def)
            {
                defset.insert(labelmap[d]);
            }

            // change out
            for (auto i : succ)
            {
                for (auto l : this->in[i])
                {
                    if (out.find(l) == out.end())
                    {
                        out.insert(l);
                        change = true;
                    }
                }
            }

            // change in variable
            for (auto& u : use)
            {
                if (in.find(labelmap[u]) == in.end())
                {
                    in.insert(labelmap[u]);
                    change = true;
                }
            }
            for (auto& o : out)
            {
                if (defset.find(o) == defset.end() && in.find(o) == in.end())
                {
                    in.insert(o);
                    change = true;
                }
            }
        }
    }

    // Construct Interference Graph
    interferences.resize(temps.size());
    for (size_t i = 0; i < in.size(); ++i)
    {
        auto in_i = in[i];
        in_i.insert(out[i].begin(), out[i].end());
        for (auto& l : in_i)
        {
            for (auto& l2 : in_i)
            {
                if (l != l2)
                {
                    interferences[l].insert(l2);
                }
            }
        }
    }
    
    // print interferences
    std::map<Temp::Label, std::set<Temp::Label>> labelmap2;
    GetInterferenceGraph(labelmap2);
    for (auto& [l, interferences_l] : labelmap2)
    {
        std::cout << "interferences for " << l.l << ": ";
        for (auto& l2 : interferences_l)
        {
            std::cout << l2.l << " ";
        }
        std::cout << "\n";
    }

    /*for (size_t i = 0; i < in.size(); ++i)
    {
        std::cout << "live in: ";
        for (auto idx : in[i])
        {
            std::cout << temps[idx].l << " ";
        }
        std::cout << "\n";
    }
    for (size_t i = 0; i < out.size(); ++i)
    {
        std::cout << "live out: ";
        for (auto idx : out[i])
        {
            std::cout << temps[idx].l << " ";
        }
        std::cout << "\n";
    }
    // print interferences
    std::set<std::pair<size_t, size_t>> edges;
    for (size_t i = 0; i < interferences.size(); ++i)
    {
        std::cout << "interferences for " << temps[i].l << ": ";
        for (auto& j : interferences[i])
        {
            std::pair<size_t, size_t> edge1 = { i, j };
            std::pair<size_t, size_t> edge2 = { j, i };
            if (edges.find(edge1) == edges.end() && edges.find(edge2) == edges.end())
            {
                std::cout << temps[j].l << " ";
                interferences[i].insert(j);
                interferences[j].insert(i);
                edges.insert(edge1);
            }
        }
        std::cout << "\n";
    }*/
}

void Liveness_InBlock::GetInterferenceGraph(std::map<Temp::Label, std::set<Temp::Label>>& labelmap)
{
    for (size_t i = 0; i < interferences.size(); ++i)
    {
        auto& l = temps[i];
        std::set<Temp::Label> interferences_l;
        for (auto& j : interferences[i])
        {
            interferences_l.insert(temps[j]);
        }
        labelmap.insert({ l, interferences_l });
    }
}

void Liveness_Graph::rinitialize(std::vector<Graph>& gs)
{
    livein.clear();
    liveout.clear();
    livein.resize(gs.size());
    liveout.resize(gs.size());
    temps.clear();
    blocks.clear();
    blocks.resize(gs.size());
    for (size_t i = 0; i < gs.size(); i++)
    {
        blocks[i].rinitialize(gs[i]);
    }

    std::map<Temp::Label, size_t> labelmap;
    for (size_t i = 0; i < gs.size(); i++)
    {
        auto& g = gs[i];
        for (auto& n : g.Nodes)
        {
            for (auto& l : n.Use())
            {
                if(labelmap.find(l) == labelmap.end())
                {
                    labelmap.insert({ l, temps.size() });
                    temps.push_back(l);
                }
            }
            for (auto& l : n.Def())
            {
                if (labelmap.find(l) == labelmap.end())
                {
                    labelmap.insert({ l, temps.size() });
                    temps.push_back(l);
                }
            }
        }
    }

    bool change = true;
    while (change)
    {
        change = false;
        for (int iter = (int)gs.size() - 1; iter >= 0; iter--)
        {
            auto& g = gs[iter];
            auto& in = livein[iter];
            auto& out = liveout[iter];
            auto& def = g.Def();
            auto& use = g.Use();
            // liveOut[B] = union(liveIn[S] for each successor S of B)
            for (auto s : g.Succs)
            {
                auto& out_s = livein[s];
                for (auto l : out_s)
                {
                    if (out.find(l) == out.end())
                    {
                        out.insert(l);
                        change = true;
                    }
                }
            }

            // liveIn[B] = use[B] union (liveOut[B] - def[B])
            for (auto& u : use)
            {
                size_t uidx = labelmap[u];
                if (in.find(uidx) == in.end())
                {
                    in.insert(uidx);
                    change = true;
                }
            }
            std::set<size_t> def_set;
            for (auto& d : def)
            {
                size_t didx = labelmap[d];
                def_set.insert(didx);
            }
            for (auto oidx : out)
            {
                if (def_set.find(oidx) == def_set.end() && in.find(oidx) == in.end())
                {
                    in.insert(oidx);
                    change = true;
                }
            }
        }
    }

    // Construct Interference Graph
    interferences.clear();
    interferences.resize(temps.size());
    for (size_t i = 0; i < livein.size(); ++i)
    {
        auto in_i = livein[i];
        in_i.insert(liveout[i].begin(), liveout[i].end());
        for (auto& l : in_i)
        {
            for (auto& l2 : in_i)
            {
                if (l != l2)
                {
                    interferences[l].insert(l2);
                }
            }
        }
    }

    /*for (size_t i = 0; i < livein.size(); i++)
    {
        std::cout << "Basic Block: " << gs[i].name.l << "\n";

        auto& def = gs[i].Def();
        auto& use = gs[i].Use();
        std::cout << "  Def: ";
        for (auto& l : def)
        {
            std::cout << l.l << " ";
        }
        std::cout << "\n";
        std::cout << "  Use: ";
        for (auto& l : use)
        {
            std::cout << l.l << " ";
        }
        std::cout << "\ncommon use and def label\n";
        std::set<Temp::Label> use_set{ use.begin(), use.end() };
        for (auto& l : def)
        {
            if (use_set.find(l) != use_set.end())
            {
                std::cout << l.l << " ";
            }
        }
        std::cout << "\n";

        std::cout << "  Live In: ";
        for (auto V : livein[i]) {
            std::cout << temps[V].l << " ";
        }
        std::cout << "\n";
        std::cout << "  Live Out: ";
        for (auto V : liveout[i]) {
            std::cout << temps[V].l << " ";
        }
        std::cout << "\n";
    }*/
}

void Liveness_Graph::GetBlockInterferenceGraph(size_t index, std::map<Temp::Label, std::set<Temp::Label>>& labelmap)
{
    blocks[index].GetInterferenceGraph(labelmap);
    auto& l = temps[index];
    std::set<Temp::Label> interferences_l;
    // insert livein[index] and liveout[index] into interferences_l
    for (auto& j : livein[index])
    {
        interferences_l.insert(temps[j]);
    }
    for (auto& j : liveout[index])
    {
        interferences_l.insert(temps[j]);
    }
    // insert interferences_l into labelmap
    for (auto& j : interferences_l)
    {
        for (auto& j2 : interferences_l)
        {
            if (j.l != j2.l)
            {
                labelmap[j].insert(j2);
            }
        }
    }
}
