#include "Assem.h"


void MunchExp(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs)
{
    std::stack<Temp::Label> temps;
    MunchExp(exp, instrs, temps);
}

void MunchExp(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs, std::stack<Temp::Label>& temps)
{
    if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(&exp.exp))
    {
        auto t = Temp::Label(std::to_string((*pconst)->n));
        temps.push(t);
        return;
    }
    else if (auto e = std::get_if<std::shared_ptr<Half_Ir_Seq>>(&exp.exp))
    {
        printf("Half_Ir_Seq\n");
        auto& vec = (*e)->seq;
        for (size_t i = 0; i < vec.size(); i++)
        {
            printf("    ");
            MunchExp(vec[i], instrs, temps);
        }
        return;
    }
    else if (auto plabel = std::get_if<std::shared_ptr<Half_Ir_Label>>(&exp.exp))
    {
        printf("Half_Ir_Label: %s\n", (*plabel)->lab.l.c_str());
        instrs.push_back(AS_Label((*plabel)->lab));
        return;
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Func>>(&exp.exp))
    {
        printf("Half_Ir_Func\n");
        printf("    ");
        MunchExp((*pfunc)->name, instrs, temps);
        printf("    ");
        MunchExp((*pfunc)->body, instrs, temps);
        auto t = temps.top();
        temps.pop();
        instrs.push_back(AS_Move(Temp::Label("eax"), t));
        instrs.push_back(AS_Return());
        return;
    }
    else if (auto e = std::get_if<std::shared_ptr<Half_Ir_Move>>(&exp.exp))
    {
        printf("Half_Ir_Move\n");
        Temp::Label r;
        if (auto plab = std::get_if<std::shared_ptr<Half_Ir_Label>>(&(*e)->right.exp))
        {
            r = (*plab)->lab;
        }
        else
        {
            printf("    ");
            MunchExp((*e)->right, instrs, temps);
            r = temps.top();
            temps.pop();
        }

        if (auto plab = std::get_if<std::shared_ptr<Half_Ir_Label>>(&(*e)->left.exp))
        {
            instrs.push_back(AS_Move((*plab)->lab, r));
            return;
        }
        MunchExp((*e)->left, instrs, temps);
        auto l = temps.top();
        if(l.l != r.l)
            instrs.push_back(AS_Move(l, r));
        temps.pop();
        return;
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Ir_BinOp>>(&exp.exp))
    {
        printf("Half_Ir_Op\n");
        printf("    ");
        MunchExp((*pop)->left, instrs, temps);
        auto l = temps.top();
        temps.pop();
        auto movl = AS_Move(Temp::NewLabel(), l);
        printf("    ");
        MunchExp((*pop)->right, instrs, temps);
        auto r = temps.top();
        temps.pop();
        auto movr = AS_Move(Temp::NewLabel(), r);
        auto tostring = [](Half_Ir_BinOp::Oper op)
            {
                switch (op)
                {
                case Half_Ir_BinOp::Oper::Unknow:
                    break;
                case Half_Ir_BinOp::Oper::Plus:
                    return std::string("add");
                    break;
                case Half_Ir_BinOp::Oper::Minus:
                    return std::string("sub");
                    break;
                case Half_Ir_BinOp::Oper::Multy:
                    return std::string("imul");
                    break;
                case Half_Ir_BinOp::Oper::Divide:
                    return std::string("idiv");
                    break;
                default:
                    break;
                }
                return std::string();
            };
        instrs.push_back(movl);
        instrs.push_back(movr);
        instrs.push_back(AS_Oper(tostring((*pop)->op), movl.dst, movr.dst));
        temps.push(movl.dst);
        return;
    }
    else if (auto pmem = std::get_if<std::shared_ptr<Half_Ir_Memory>>(&exp.exp))
    {
        printf("Half_Ir_Memory\n");
        auto l = Temp::Label(std::to_string((*pmem)->offset + 4) + std::string("(%esp)"));
        temps.push(l);
        return;
    }
    else if (auto pcall = std::get_if<std::shared_ptr<Half_Ir_Call>>(&exp.exp))
    {
        printf("Half_Ir_Call\n");
        auto& vec = (*pcall)->args;
        for (size_t i = 0; i < vec.size(); i++)
        {
            printf("        ");
            MunchExp(vec[i], instrs, temps);
        }
        
        return;
    }
    else if (auto pret = std::get_if<std::shared_ptr<Half_Ir_Return>>(&exp.exp))
    {
        printf("Half_Ir_Return\n");
        auto v = (*pret)->value;
        if (auto pexp = std::get_if<std::shared_ptr<Half_Ir_Label>>(&v.exp))
        {
            instrs.push_back(AS_Move(Temp::Label("%eax"), (*pexp)->lab));
        }
        else
        {
            MunchExp(v, instrs, temps);
            auto r = temps.top();
            temps.pop();
            instrs.push_back(AS_Move(Temp::Label("%eax"), r));
        }
        instrs.push_back(AS_Return());
        return;
    }
    printf("None\n");
}

void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Instr>& instrs)
{
    for (size_t i = 0; i < builder.blocks.size(); i++)
    {
        auto& block = builder.blocks[i];
        std::stack<Temp::Label> temps;
        //MunchExp_llvmlike(Half_Ir_Label(block.label), instrs, temps);
        for (auto& exp : block.exps)
        {
            printf("\n");
            MunchExp_llvmlike(exp, instrs, temps);
        }
    }
}

void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs)
{
    std::stack<Temp::Label> temps;
    MunchExp_llvmlike(exp, instrs, temps);
}

void SeperateExp(std::vector<Half_Ir_Exp>& exps, std::vector<Half_Ir_Alloc>& allocs, std::vector<Half_Ir_Exp>& others)
{
    for (auto& e : exps)
    {
        if (auto palloc = std::get_if<std::shared_ptr<Half_Ir_Alloc>>(&e.exp))
        {
            allocs.push_back(**palloc);
        }
        else
        {
            others.push_back(e);
        }
    }
}

void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs, std::stack<Temp::Label>& temps)
{
    if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(&exp.exp))
    {
        auto t = (*pconst)->out_label;
        instrs.push_back(AS_Move(t, Temp::Label("%" + std::to_string((*pconst)->n))));
        return;
    }
    else if (auto plabel = std::get_if<std::shared_ptr<Half_Ir_Label>>(&exp.exp))
    {
        printf("Half_Ir_Label: %s\n", (*plabel)->lab.l.c_str());
        instrs.push_back(AS_Label((*plabel)->lab));
        return;
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(&exp.exp))
    {
        printf("Half_Ir_Func\n");
        printf("    ");
        instrs.push_back(AS_Label((*pfunc)->name));

        // 1. allocs stack space (according to the number of parameters)
        instrs.push_back(AS_StackAlloc(((*pfunc)->alloc.exps.size() + 1) * 4));

        // 2. map func blocks with label and index
        std::map<Temp::Label, size_t> block_map;
        for (size_t i = 0; i < (*pfunc)->blocks.size(); i++)
        {
            auto& b = (*pfunc)->blocks[i];
            block_map.insert({ b.label, i });
        }

        // 3. pre process the phi node
        for (size_t i = 0; i < (*pfunc)->blocks.size(); i++)
        {
            auto& block = (*pfunc)->blocks[i];
            for (auto& exp : block.exps)
            {
                if (auto pphi = std::get_if<std::shared_ptr<Half_Ir_Phi>>(&exp.exp))
                {
                    auto& phi = **pphi;
                    for (auto& [n, l] : phi.values)
                    {
                        if (auto p = block_map.find(l.lab); p != block_map.end())
                        {
                            auto& target_block = (*pfunc)->blocks[p->second];
                            auto last = target_block.exps.back();
                            target_block.exps.back() = Half_Ir_Exp(Half_Ir_Move(phi.result, n));
                            target_block.exps.push_back(last);
                        }
                        else
                        {
                            printf("Phi node error, not found block label:%s\n", l.lab.l.c_str());
                            _ASSERT(false);
                        }
                    }
                }
            }
            // remove phi node
            block.exps.erase(std::remove_if(block.exps.begin(), block.exps.end(), [](Half_Ir_Exp& e)
                {
                    if (auto pphi = std::get_if<std::shared_ptr<Half_Ir_Phi>>(&e.exp))
                    {
                        return true;
                    }
                    return false;
                }), block.exps.end());
        }

        // 4. generate code for each block
        for (auto& block : (*pfunc)->blocks)
        {
            // insert block label
            instrs.push_back(AS_Label(block.label));

            for (auto& e : block.exps)
            {
                printf("    ");
                if (auto pphi = std::get_if<std::shared_ptr<Half_Ir_Phi>>(&e.exp))
                {
                    // shouldn't be here
                    _ASSERT(false);
                }
                else
                {
                    MunchExp_llvmlike(e, instrs, temps);
                }
            }
        }
        return;
    }
    else if (auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(&exp.exp))
    {
        printf("Half_Ir_Load\n");
        auto l = Temp::Label(std::to_string((*pload)->offset) + std::string("(%esp)"));
        instrs.push_back(AS_Move((*pload)->out_label, l));
        return;
    }
    else if (auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(&exp.exp))
    {
        printf("Half_Ir_Store\n");
        auto l = Temp::Label(std::to_string(std::get<size_t>((*pstore)->data)) + std::string("(%esp)"));
        instrs.push_back(AS_Move(l, (*pstore)->in_label));
        return;
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Ir_LlvmBinOp>>(&exp.exp))
    {
        auto& binop = **pop;
        printf("Half_Ir_Op\n");
        auto movl = binop.left.name;
        auto movr = binop.right.name;
        auto tostring = [](Half_Ir_LlvmBinOp::Oper op)
            {
                switch (op)
                {
                case Half_Ir_LlvmBinOp::Oper::Unknow:
                    break;
                case Half_Ir_LlvmBinOp::Oper::Plus:
                    return std::string("add");
                    break;
                case Half_Ir_LlvmBinOp::Oper::Minus:
                    return std::string("sub");
                    break;
                case Half_Ir_LlvmBinOp::Oper::Multy:
                    return std::string("imul");
                    break;
                case Half_Ir_LlvmBinOp::Oper::Divide:
                    return std::string("idiv");
                    break;
                default:
                    break;
                }
                return std::string();
            };
        instrs.push_back(AS_Oper(tostring((*pop)->op), movl, movr));
        instrs.push_back(AS_Move(binop.out_label, movl));
        return;
    }
    else if (auto pmem = std::get_if<std::shared_ptr<Half_Ir_Memory>>(&exp.exp))
    {
        printf("Half_Ir_Memory\n");
        auto l = Temp::Label(std::to_string((*pmem)->offset + 4) + std::string("(%esp)"));
        temps.push(l);
        return;
    }
    else if (auto pcall = std::get_if<std::shared_ptr<Half_Ir_Call>>(&exp.exp))
    {
        printf("Half_Ir_Call\n");
        auto& vec = (*pcall)->args;
        for (size_t i = 0; i < vec.size(); i++)
        {
            printf("        ");
            MunchExp_llvmlike(vec[i], instrs, temps);
        }
        return;
    }
    else if (auto pret = std::get_if<std::shared_ptr<Half_Ir_Return>>(&exp.exp))
    {
        printf("Half_Ir_Return\n");
        auto v = (*pret)->value;
        if (auto pn = std::get_if<std::shared_ptr<Half_Ir_Name>>(&v.exp))
        {
            instrs.push_back(AS_Move(Temp::Label("%eax"), (*pn)->name));
        }
        instrs.push_back(AS_Return());
        return;
    }
    else if (auto pmove = std::get_if<std::shared_ptr<Half_Ir_Move>>(&exp.exp))
    {
        printf("Half_Ir_Move\n");
        auto& move = **pmove;
        Temp::Label dst, src;
        if (auto pname = std::get_if<std::shared_ptr<Half_Ir_Name>>(&move.left.exp))
        {
            dst = (*pname)->name;
        }
        else
        {
            printf("Not support other exp type in Half_Ir_Move.left\n");
            _ASSERT(false);
        }
        if (auto pname = std::get_if<std::shared_ptr<Half_Ir_Name>>(&move.right.exp))
        {
            src = (*pname)->name;
        }
        else
        {
            printf("Not support other exp type in Half_Ir_Move.right\n");
            _ASSERT(false);
        }
        instrs.push_back(AS_Move(dst, src));
        return;
    }
    else if (auto pbr = std::get_if<std::shared_ptr<Half_Ir_LlvmBranch>>(&exp.exp))
    {
        printf("Half_Ir_Branch\n");
        auto& branch = **pbr;

        instrs.push_back(AS_Oper("cmp", branch.condition.left.name, branch.condition.right.name));
        instrs.push_back(AS_Jump(branch.condition.op, branch.true_label));
        instrs.push_back(AS_Jump(Half_Ir_Compare::GetNot(branch.condition.op), branch.false_label));
        return;
    }
    else if (auto pjmp = std::get_if<std::shared_ptr<Half_Ir_Jump>>(&exp.exp))
    {
        printf("Half_Ir_Jump\n");
        auto& jmp = **pjmp;
        instrs.push_back(AS_Jump("jmp", jmp.target));
        return;
    }
    else
    {
        printf("None\n");
        _ASSERT(false);
    }
}

inline std::string to_string(const AS_StackAlloc& al)
{
    return std::string("subq %" + std::to_string(al.bytes) + ", %rsp\n");
}
inline std::string to_string(const AS_Oper& op)
{
    auto src = op.src.l.starts_with('e') ? "%" + op.src.l : op.src.l;
    auto dst = op.dst.l.starts_with('e') ? "%" + op.dst.l : op.dst.l;
    return op.assem + "l " + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Move& mv)
{
    auto src = mv.src.l.starts_with('e') ? "%" + mv.src.l : mv.src.l;
    auto dst = mv.dst.l.starts_with('e') ? "%" + mv.dst.l : mv.dst.l;
    return std::string("movl ") + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Jump& jmp)
{
    return jmp.jump + " " + jmp.target.l + "\n";
}
inline std::string to_string(const AS_Label& lab)
{
    return lab.label.l + ":\n";
}
inline std::string to_string(const AS_Return& ret)
{
    return "ret\n";
}

std::string to_string(const AS_Instr& instr)
{
    if (auto palloc = std::get_if<AS_StackAlloc>(&instr))
    {
        return to_string(*palloc);
    }
    else if (auto pop = std::get_if<AS_Oper>(&instr))
    {
        return to_string(*pop);
    }
    else if (auto pmv = std::get_if<AS_Move>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pjmp = std::get_if<AS_Jump>(&instr))
    {
        return to_string(*pjmp);
    }
    else if (auto plab = std::get_if<AS_Label>(&instr))
    {
        return to_string(*plab);
    }
    else if (auto pret = std::get_if<AS_Return>(&instr))
    {
        return to_string(*pret);
    }
    return std::string();
}