#include "Color.h"

void Color::initialize(Liveness& liveness)
{
    adjList.resize(liveness.temps.size());
    color.resize(liveness.temps.size(), -1);
    degree.resize(liveness.temps.size());
    printf("Coloring initialize:\n");
    for (int i = 0; i < liveness.in.size(); ++i)
    {
        std::vector<Temp::Label> in = { liveness.in[i].begin(), liveness.in[i].end() };
        for (size_t i = 0; i < in.size(); ++i)
        {
            for (size_t j = i + 1; j < in.size(); ++j)
            {
                auto u = tempMap.get(in[i]);
                auto v = tempMap.get(in[j]);
                addEdge(u, v);
                printf("    add edge %d %d\n", u, v);
            }
        }
    }
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
        std::set<int> usedColors;
        for (int neighbor : adjList[node])
        {
            if (color[neighbor] != -1)
            {
                usedColors.insert(color[neighbor]);
            }
        }
        for (int c = 0; c < adjList.size(); ++c)
        {
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
