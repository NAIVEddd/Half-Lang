#include"Graph.h"
#include<variant>
#include<ranges>

bool TestLabel(Temp::Label l)
{
    return l.l.starts_with('t');
}

void Node::AddEdge(Graph& g, size_t from, size_t to)
{
    auto& fn = g.Nodes[from];
    auto& tn = g.Nodes[to];
    fn.SuccNodes.push_back(to);
    tn.PredNodes.push_back(from);
}

std::vector<size_t> Node::Succ() const
{
    return SuccNodes;
}

std::vector<size_t> Node::Pred() const
{
    return PredNodes;
}

std::vector<size_t> Node::Adj() const
{
    std::set<size_t> s;
    s.insert_range(PredNodes);
    s.insert_range(SuccNodes);
    return { s.begin(), s.end() };
}

std::vector<Temp::Label> Node::Def() const
{
    std::vector<Temp::Label> labels;
    if (auto pop = std::get_if<AS_Oper>(&info))
    {
        labels.push_back(pop->dst);
    }
    else if (auto pmv = std::get_if<AS_Move>(&info))
    {
        labels.push_back(pmv->dst);
    }
    else if (auto plea = std::get_if<AS_Lea>(&info))
    {
        labels.push_back(plea->dst);
    }
    else if (auto pptr = std::get_if<AS_ElemPtr>(&info))
    {
        labels.push_back(pptr->out_label);
    }
    else if (auto pload = std::get_if<AS_ElemLoad>(&info))
    {
        labels.push_back(pmv->dst);
    }
    else if (auto pstore = std::get_if<AS_ElemStore>(&info))
    {
    }
    else if (auto pmv = std::get_if<AS_ArrayLoad>(&info))
    {
        labels.push_back(pmv->dst);
    }
    else if (auto pmv = std::get_if<AS_ArrayStore>(&info))
    {
    }
    else if (auto pjmp = std::get_if<AS_Jump>(&info))
    {

    }
    else if (auto plab = std::get_if<AS_Label>(&info))
    {

    }
    else if (auto pcall = std::get_if<AS_Call>(&info))
    {
    }
    else if (auto pret = std::get_if<AS_Return>(&info))
    {

    }
    auto ls = labels | std::views::filter(TestLabel);
    return { ls.begin(), ls.end() };
}

std::vector<Temp::Label> Node::Use() const
{
    std::vector<Temp::Label> labels;
    if (auto pop = std::get_if<AS_Oper>(&info))
    {
        labels.push_back(pop->src);
        labels.push_back(pop->dst);
    }
    else if (auto pmv = std::get_if<AS_Move>(&info))
    {
        labels.push_back(pmv->src);
    }
    else if (auto plea = std::get_if<AS_Lea>(&info))
    {
        labels.push_back(plea->src);
    }
    else if (auto pptr = std::get_if<AS_ElemPtr>(&info))
    {
        labels.push_back(pptr->elem_ptr);
    }
    else if (auto pload = std::get_if<AS_ElemLoad>(&info))
    {
        labels.push_back(pload->elem_ptr);
    }
    else if (auto pstore = std::get_if<AS_ElemStore>(&info))
    {
        labels.push_back(pstore->elem_ptr);
        labels.push_back(pstore->src);
    }
    else if (auto pmv = std::get_if<AS_ArrayLoad>(&info))
    {
        labels.push_back(pmv->src);
    }
    else if (auto pmv = std::get_if<AS_ArrayStore>(&info))
    {
        labels.push_back(pmv->src);
        labels.push_back(pmv->dst);
    }
    else if (auto pjmp = std::get_if<AS_Jump>(&info))
    {
        
    }
    else if (auto plab = std::get_if<AS_Label>(&info))
    {
        
    }
    else if (auto pcall = std::get_if<AS_Call>(&info))
    {
        for (auto& arg : pcall->args)
        {
            labels.push_back(arg);
        }
    }
    else if (auto pret = std::get_if<AS_Return>(&info))
    {

    }
    auto ls = labels | std::views::filter(TestLabel);
    return { ls.begin(), ls.end() };
}

void Graph::initialize(std::vector<AS_Instr>& ins)
{
    instrs = ins;
    Nodes.clear();
    for (size_t i = 0; i < instrs.size(); i++)
    {
        printf("%d ", (int) instrs[i].index());
        Nodes.push_back(Node(instrs[i]));
        if (auto plab = std::get_if<AS_Label>(&instrs[i]))
        {
            LabelMap.insert({ plab->label, i });
        }
    }
    for (size_t i = 0; i < instrs.size(); i++)
    {
        if (auto pop = std::get_if<AS_Oper>(&instrs[i]))
        {
            
        }
        else if (auto pmv = std::get_if<AS_Move>(&instrs[i]))
        {

        }
        else if (auto pjmp = std::get_if<AS_Jump>(&instrs[i]))
        {
            Nodes[i].AddEdge(*this, i, LabelMap[pjmp->target]);
            return;
        }
        else if (auto plab = std::get_if<AS_Label>(&instrs[i]))
        {
            continue;
        }
        else if (auto pret = std::get_if<AS_Return>(&instrs[i]))
        {
            
        }
        Nodes[i - 1].AddEdge(*this, i - 1, i);
    }
}
