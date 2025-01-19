#pragma once

#include"Graph.h"

struct Liveness_InBlock
{
    std::vector<std::set<size_t>> in;
    std::vector<std::set<size_t>> out;
    std::vector<Temp::Label> temps;
    std::vector<std::set<size_t>> interferences;
    
    Liveness_InBlock() = default;
    void rinitialize(const Graph& g);
    void GetInterferenceGraph(std::map<Temp::Label, std::set<Temp::Label>>& labelmap);
    bool operator==(const Liveness_InBlock& o) const;
};

struct Liveness_Graph
{
    std::vector<Liveness_InBlock> blocks;
    std::vector<std::set<size_t>> livein;
    std::vector<std::set<size_t>> liveout;
    std::vector<std::set<size_t>> interferences;
    std::vector<Temp::Label> temps;
    Liveness_Graph() = default;
    void rinitialize(std::vector<Graph>& gs);
    void GetBlockInterferenceGraph(size_t index, std::map<Temp::Label, std::set<Temp::Label>>& labelmap);
    //bool operator==(const Liveness_Graph& o) const;
};