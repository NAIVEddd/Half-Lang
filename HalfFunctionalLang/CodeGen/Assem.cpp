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
        MunchExp((*e)->right, instrs, temps);
        auto r = temps.top();
        temps.pop();
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
        auto l = Temp::Label(std::string("[ebp - ") + std::to_string((*pmem)->offset + 4) + std::string("]"));
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
    printf("None\n");
}

inline std::string to_string(const AS_Alloc& al)
{
    return std::to_string(al.bytes);
}
inline std::string to_string(const AS_Oper& op)
{
    return op.assem + " " + op.dst.l + " " + op.src.l + "\n";
}
inline std::string to_string(const AS_Move& mv)
{
    return std::string("mov ") + mv.dst.l + " " + mv.src.l + "\n";
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
    if (auto pop = std::get_if<AS_Oper>(&instr))
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