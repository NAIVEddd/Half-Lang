#include "Color.h"

void Color::initialize(Liveness_Graph& l)
{
    liveness = l;
}

void Color::precolored(std::map<Temp::Label, int>& colored)
{
    precolor = colored;
}

std::map<Temp::Label, int> Color::AllocateRegisters()
{
    std::map<Temp::Label, int> registerMap;
    registerMap = precolor;
    for (size_t i = 0; i < liveness.blocks.size(); ++i)
    {
        std::map<Temp::Label, std::set<Temp::Label>> interferenceGraph;
        liveness.GetBlockInterferenceGraph(i, interferenceGraph);
        for (const auto& graph : interferenceGraph)
        {
            const auto& var = graph.first;
            const auto& neighbors = graph.second;

            if (registerMap.find(var) != registerMap.end())
            {
                continue;
            }

            std::set<int> usedRegisters;
            for (const auto& neighbor : neighbors)
            {
                if (registerMap.find(neighbor) != registerMap.end())
                {
                    usedRegisters.insert(registerMap[neighbor]);
                }
            }
            bool found = false;
            for (int i = 0; i < numRegisters; ++i)
            {
                if (usedRegisters.find(i) == usedRegisters.end())
                {
                    registerMap[var] = i;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                printf("Error: No available register for variable %s\n", var.l.c_str());
            }
        }
    }
    
    return registerMap;
}

void Color::simplify()
{
    for (size_t i = 0; i < adjList.size(); i++)
    {
        // add degree of adjacent nodes
        degree[i] = (int)adjList[i].size();
    }
    for (int i = 0; i < adjList.size(); ++i)
    {
        if (degree[i] < numRegisters)
        {
            stack.push(i);
        }
    }
}

void Color::select()
{
    while (!stack.empty())
    {
        int node = stack.top();
        stack.pop();

        // already colored
        if (color[node] != -1)
        {
            continue;
        }

        std::set<int> usedColors;
        for (int neighbor : adjList[node])
        {
            if (color[neighbor] != -1)
            {
                usedColors.insert(color[neighbor]);
            }
        }
        for (int c = 0; c < numRegisters; ++c)
        {
            // select a color
            //    if current register not used by neighbors, assign it
            if (usedColors.find(c) == usedColors.end())
            {
                color[node] = c;
                break;
            }
        }
    }
}

void Color::coalesce()
{
    /*for (int i = 0; i < adjList.size(); ++i)
    {
        if (removed[i])
        {
        continue;
        }
        for (int j : adjList[i])
        {
            if (removed[j])
            {
            continue;
            }
            if (color[i] == color[j])
            {
                alias[i] = j;
                //removed[i] = true;
                break;
            }
        }
    }*/
}
