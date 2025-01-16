#pragma once

#include"Liveness.h"

#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <stack>

struct TempMap
{
    std::map<Temp::Label, int> map;
    std::map<int, Temp::Label> rmap;
    int count = 0;

    int find(Temp::Label l)
    {
        if (map.find(l) == map.end())
        {
            return -1;
        }
        return map[l];
    }
    int get(Temp::Label l)
    {
        if (map.find(l) == map.end())
        {
            map[l] = count;
            rmap[count] = l;
            count++;
        }
        return map[l];
    }
    Temp::Label get(int l)
    {
        return rmap[l];
    }
};

struct Color
{
    int numRegisters;
    TempMap tempMap;
    std::vector<std::set<int>> adjList;
    std::vector<int> color;
    std::vector<int> degree;
    std::vector<int> alias;
    std::stack<int> stack;
    std::map<Temp::Label, int> precolor;
    Liveness_Graph liveness;
    Color(int numRegisters) : numRegisters(numRegisters), alias(numRegisters) {}
    void initialize(Liveness& liveness);
    void initialize(Liveness_Graph& l);
    void precolored(std::map<Temp::Label, int>& colored);
    std::map<Temp::Label, int> AllocateRegisters();
    void addEdge(int u, int v)
    {
        if (u != v) {
            adjList[u].insert(v);
            adjList[v].insert(u);
        }
    }
    void simplify();
    void select();
    void coalesce();
    void allocate()
    {
        simplify();
        select();
        coalesce();
    }
    void print()
    {
        printf("Coloring result:\n");
        for (size_t i = 0; i < color.size(); i++)
        {
            printf("  %s use color %d\n", tempMap.get((int)i).l.c_str(), color[i]);
        }
    }
};

