#include "Liveness.h"

void Liveness::inittemps(const Graph& g)
{
    // insert all nodes use and def into temps
    std::set<Temp::Label> tempset;
    for (auto& node : g.Nodes)
    {
        for (auto& l : node.Use())
        {
            tempset.insert(l);
        }
        for (auto& l : node.Def())
        {
            tempset.insert(l);
        }
    }

    temps.clear();
    /*for (auto& i : in)
    {
        for (auto& l : i)
        {
            tempset.insert(l);
        }
    }
    for (auto& o : out)
    {
        for (auto& l : o)
        {
            tempset.insert(l);
        }
    }*/
    temps.assign_range(tempset);
}

void Liveness::initialize(const Graph& g)
{
    int count = 0;
    auto& nodes = g.Nodes;
    std::vector<std::set<Temp::Label>> inlabels(nodes.size());
    std::vector<std::set<Temp::Label>> outlabels(nodes.size());
    bool have_value_change = false;
    do
    {
        have_value_change = false;
        count++;

        for (size_t i = 0; i < nodes.size(); i++)
        {
            auto pred = nodes[i].Pred();
            auto succ = nodes[i].Succ();
            auto use = nodes[i].Use();
            auto def = nodes[i].Def();
            auto& in = inlabels[i];
            auto& out = outlabels[i];
            std::set<Temp::Label> defset;
            defset.insert_range(def);

            // change in variable
            for (auto& u : use)
            {
                if (in.find(u) == in.end())
                {
                    in.insert(u);
                    have_value_change = true;
                }
            }
            for (auto& o : out)
            {
                if (defset.find(o) == defset.end() && in.find(o) == in.end())
                {
                    in.insert(o);
                    have_value_change = true;
                }
            }

            // change out
            for (auto i : succ)
            {
                for (auto l : inlabels[i])
                {
                    if (out.find(l) == out.end())
                    {
                        out.insert(l);
                        have_value_change = true;
                    }
                }
            }
        }
    } while (have_value_change);
    in = inlabels;
    out = outlabels;
    inittemps(g);
    printf("\n->Liveness::initialize cost %d loops\n", count);
}

void Liveness::rinitialize(const Graph& g)
{
    int count = 0;
    auto& nodes = g.Nodes;
    std::vector<std::set<Temp::Label>> inlabels(nodes.size());
    std::vector<std::set<Temp::Label>> outlabels(nodes.size());
    bool have_value_change = false;
    do
    {
        have_value_change = false;
        count++;

        for (size_t i = 0; i < nodes.size(); i++)
        {
            auto pred = nodes[i].Pred();
            auto succ = nodes[i].Succ();
            auto use = nodes[i].Use();
            auto def = nodes[i].Def();
            auto& in = inlabels[i];
            auto& out = outlabels[i];
            std::set<Temp::Label> defset;
            defset.insert_range(def);

            // change out
            for (auto i : succ)
            {
                for (auto l : inlabels[i])
                {
                    if (out.find(l) == out.end())
                    {
                        out.insert(l);
                        have_value_change = true;
                    }
                }
            }

            // change in variable
            for (auto& u : use)
            {
                if (in.find(u) == in.end())
                {
                    in.insert(u);
                    have_value_change = true;
                }
            }
            for (auto& o : out)
            {
                if (defset.find(o) == defset.end() && in.find(o) == in.end())
                {
                    in.insert(o);
                    have_value_change = true;
                }
            }
        }
    } while (have_value_change);
    in = inlabels;
    out = outlabels;
    inittemps(g);
    printf("\n->Liveness::rinitialize cost %d loops\n", count);
}

bool Liveness::operator==(const Liveness& o) const
{
    bool sz = in.size() == o.in.size() && out.size() == o.out.size();
    for (size_t i = 0; i < in.size(); i++)
    {
        sz = sz && (in[i].size() == o.in[i].size());
        if (sz)
        {
            for (auto& l : in[i])
            {
                sz = sz && (o.in[i].count(l) == 1);
            }
            continue;
        }
        return false;
    }
    for (size_t i = 0; i < out.size(); i++)
    {
        sz = sz && (out[i].size() == o.out[i].size());
        if (sz)
        {
            for (auto& l : out[i])
            {
                sz = sz && (o.out[i].count(l) == 1);
            }
            continue;
        }
        return false;
    }
    return true;
}
