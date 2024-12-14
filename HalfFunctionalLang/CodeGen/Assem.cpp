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
        auto t = Temp::Label(std::to_string((*pconst)->n));
        //temps.push(t);
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

        auto block = (*pfunc)->blocks[0];
        std::vector<Half_Ir_Alloc> allocs;
        std::vector<Half_Ir_Exp> others;
        SeperateExp(block.exps, allocs, others);
        instrs.push_back(AS_StackAlloc((allocs.size() + 1) * 4));

        for (auto& b : others)
        {
            printf("    ");
            MunchExp_llvmlike(b, instrs, temps);
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
}

inline std::string to_string(const AS_StackAlloc& al)
{
    return std::string("subq %" + std::to_string(al.bytes) + ", %rsp\n");
}
inline std::string to_string(const AS_Oper& op)
{
    return op.assem + "l " + op.src.l + ", " + op.dst.l + "\n";
}
inline std::string to_string(const AS_Move& mv)
{
    return std::string("movl ") + mv.src.l + ", " + mv.dst.l + "\n";
}
inline std::string to_string(const AS_Jump& jmp)
{
    return std::string("jmp ") + jmp.target.l + "\n";
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