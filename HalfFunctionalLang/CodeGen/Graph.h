#pragma once

#include"Assem.h"
#include<vector>
#include<set>
#include<map>
#include<memory>

// Flow Graph
struct Graph;

struct Node
{
    AS_Instr info;
    std::vector<size_t> SuccNodes;
    std::vector<size_t> PredNodes;

    Node(AS_Instr t) : info(t) {}
    void AddEdge(Graph& g, size_t from, size_t to);
    std::vector<size_t> Succ() const;
    std::vector<size_t> Pred() const;
    std::vector<size_t> Adj() const;
    std::vector<Temp::Label> Def() const;
    std::vector<Temp::Label> Use() const;
};

struct Graph
{
    std::vector<AS_Instr> instrs;
    std::vector<Node> Nodes;
    // Label to index of line
    std::map<Temp::Label, size_t> LabelMap;

    Graph() = default;
    void initialize(std::vector<AS_Instr>& ins);
};

struct GTable
{

};