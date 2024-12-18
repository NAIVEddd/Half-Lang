#pragma once

#include"Graph.h"

struct Liveness
{
    std::vector<std::set<Temp::Label>> in;
    std::vector<std::set<Temp::Label>> out;
    std::vector<Temp::Label> temps;

    Liveness() = default;
    void inittemps(const Graph& g);
    void initialize(const Graph& g);
    void rinitialize(const Graph& g);
    bool operator==(const Liveness& o) const;
};