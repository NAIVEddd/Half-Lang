#include "Assem.h"


void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Instr>& instrs)
{
    for (size_t i = 0; i < builder.blocks.size(); i++)
    {
        auto& block = builder.blocks[i];
        for (auto& exp : block.exps)
        {
            printf("\n");
            MunchExp_llvmlike(exp, instrs);
        }
    }
    for (auto& [f, s] : builder.strings)
    {
        instrs.push_back(AS_String(f, s));
    }
}

void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs)
{
    if (auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(&exp.exp))
    {
        printf("Half_Ir_Load:\n");
        AS_Move_Type move((*pload)->out_register, (*pload)->address);
        instrs.push_back(move);
        return;
    }
    else if (auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(&exp.exp))
    {
        printf("Half_Ir_Store:\n");
        auto base_l = (*pstore)->address.base.l;
        auto reg = base_l == "bottom" ? "(%rsp)"
            : (base_l == "top" ? "(%rbp)" : "(" + base_l + ")");
        auto l = Temp::Label(std::to_string((*pstore)->address.offset) + reg);
        AS_Move_Type move((*pstore)->address, (*pstore)->value);
        instrs.push_back(move);
        return;
    }
    else if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(&exp.exp))
    {
        auto t = (*pconst)->out_label;
        instrs.push_back(AS_Move(t, Temp::Label("$" + std::to_string((*pconst)->n))));
        return;
    }
    else if (auto pvalue = std::get_if<std::shared_ptr<Half_Ir_Value>>(&exp.exp))
    {
        auto v = (*pvalue)->val;
        if (auto pstr = std::get_if<Half_Ir_String>(&v))
        {
            AS_Move_String move((*pvalue)->out_label, pstr->label);
            instrs.push_back(move);
        }
        else if (auto pfloat = std::get_if<Half_Ir_Float>(&v))
        {
            _ASSERT(false);
            //instrs.push_back(AS_Move((*pvalue)->out_label, Temp::Label("$" + std::to_string(pfloat->f))));
        }
        else
        {
            _ASSERT(false);
            //instrs.push_back(AS_Move((*pvalue)->out_label, v));
        }
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
        auto stack_size = ((*pfunc)->alloc.exps.size() + 1) * 4;
        instrs.push_back(AS_StackAlloc(stack_size));

        // 1.5 store parameters from registers to stack
        auto regs = std::vector<std::string>{  "ecx","edx", "r8d", "r9d" };
        for (size_t i = 0; i < (*pfunc)->args.size() && i < regs.size(); ++i)
        {
            instrs.push_back(AS_Move(Temp::Label(std::to_string(i * 4) + "(%rsp)"), Temp::Label(regs[i])));
        }

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
                    MunchExp_llvmlike(e, instrs);
                }
            }
        }

        // 5. return
        _ASSERT(std::get_if<AS_Return>(&instrs.back()));
        instrs.back() = AS_Return(stack_size);
        return;
    }
    else if (auto pgetptr = std::get_if<std::shared_ptr<Half_Ir_GetElementPtr>>(&exp.exp))
    {
        printf("Half_Ir_GetElementPtr\n");
        /*auto& getptr = **pgetptr;
        std::vector<Temp::Label> offsets;
        for (size_t i = 0; i < getptr.in_index.size(); ++i)
        {
            MunchExp_llvmlike(getptr.in_index[i], instrs);
            Half_Ir_Const c(getptr.elem_sizes[i]);
            MunchExp_llvmlike(Half_Ir_Exp(c), instrs);
            Temp::Label offset = Temp::NewLabel();
            Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Multy, getptr.exp_out_labels[i], c.out_label, offset);
            MunchExp_llvmlike(Half_Ir_Exp(binop), instrs);
            offsets.push_back(offset);
        }
        if (offsets.size() == 1)
        {
            Temp::Label base = Temp::NewLabel();
            instrs.push_back(AS_ElemPtr(getptr.offset, Temp::Label("%rsp"), base));
            Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Plus, base, offsets[0], Temp::NewLabel());
            MunchExp_llvmlike(Half_Ir_Exp(binop), instrs);
            instrs.push_back(AS_ElemPtr(0, binop.out_label, getptr.out_label));
        }*/
        return;
    }
    else if (auto pfetch = std::get_if<std::shared_ptr<Half_Ir_FetchPtr>>(&exp.exp))
    {
        printf("Half_Ir_FetchPtr\n");
        auto& fetch = **pfetch;
        auto base_l = fetch.ptr.base.l;
        auto reg = base_l == "bottom" ? "rsp"
            : (base_l == "top" ? "rbp" : base_l);
        auto l = Temp::Label(reg);
        auto offset = fetch.ptr.offset;
        AS_Lea lea(fetch.out_label, l, offset);
        instrs.push_back(lea);/**/
        return;
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Ir_BinOp>>(&exp.exp))
    {
        auto& binop = **pop;
        printf("Half_Ir_Op\n");
        auto movl = binop.left.name;
        auto movr = binop.right.name;
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
        instrs.push_back(AS_Oper(tostring((*pop)->op), movl, movr));
        instrs.push_back(AS_Move(binop.out_label, movl));
        return;
    }
    else if (auto pcall = std::get_if<std::shared_ptr<Half_Ir_Call>>(&exp.exp))
    {
        printf("Half_Ir_Call\n");
        auto& call = **pcall;
        std::vector<Temp::Label> args(call.args.size());
        for (size_t i = 0; i < call.args.size(); i++)
        {
            args[i] = call.args[i].name;
        }
        instrs.push_back(AS_Call(call.fun_name, args));
        return;
    }
    else if (auto pret = std::get_if<std::shared_ptr<Half_Ir_Return>>(&exp.exp))
    {
        printf("Half_Ir_Return\n");
        auto v = (*pret)->value;
        auto label = v.GetLabel();
        instrs.push_back(AS_Move(Temp::Label("%eax"), label));
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
    else if (auto pbr = std::get_if<std::shared_ptr<Half_Ir_Branch>>(&exp.exp))
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

inline std::string to_string(const AS_String& str)
{
    return str.label.l + ":\n\t.asciz \"" + str.str + "\"\n";
}
inline std::string to_string(const AS_StackAlloc& al)
{
    auto push = std::string("pushq %rbp\n");
    return push + std::string("subq $" + std::to_string(al.bytes) + ", %rsp\n");
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
inline std::string to_string(const AS_Move_String& mv)
{
    auto src = mv.src.l + "(%rip)";
    std::string dst = mv.dst.l.starts_with('e') ? "%" + mv.dst.l : mv.dst.l;
    if (mv.dst.l.starts_with('e'))
    {
        dst = mv.dst.l;
        dst[0] = 'r';
        dst = "%" + dst;
    }
    return std::string("leaq ") + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Move_Type& mv)
{
    std::string src, dst;
    std::string inst;
    // 1.move Register to Register
    if (std::holds_alternative<Register>(mv.src.value) && std::holds_alternative<Register>(mv.dst.value))
    {
        src = std::get<Register>(mv.src.value).reg.l;
        dst = std::get<Register>(mv.dst.value).reg.l;
        auto src_type = mv.src.GetType();
        auto dst_type = mv.dst.GetType();
        if (src_type.is_pointer() || dst_type.is_pointer())
        {
            inst = "movq ";
            if (src.starts_with('e') || dst.starts_with('e'))
            {
                src[0] = 'r';
                dst[0] = 'r';
            }
            src = "%" + src;
            dst = "%" + dst;
            return inst + src + ", " + dst + "\n";
        }
        else if (src_type.is_basic() && dst_type.is_basic())
        {
            inst = "movl ";
            src = "%" + src;
            dst = "%" + dst;
            return inst + src + ", " + dst + "\n";
        }
        else
        {
            _ASSERT(false);
        }
    }
    // 2.move Register to Address
    else if (std::holds_alternative<Register>(mv.src.value) && std::holds_alternative<Address>(mv.dst.value))
    {
        auto& addr = std::get<Address>(mv.dst.value);
        auto val = std::get<Register>(mv.src.value);
        auto val_type = mv.src.GetType();
        if (val_type.GetSize() == 8)
        {
            inst = "movq ";
            if (val.reg.l.starts_with('e'))
            {
                val.reg.l[0] = 'r';
            }
            src = "%" + val.reg.l;
        }
        else if (val_type.is_basic())
        {
            inst = "movl ";
            src = "%" + val.reg.l;
        }
        else
        {
            _ASSERT(false);
        }
        auto reg = addr.base.l == "bottom" ? "(%rsp)" :
            (addr.base.l == "top" ? "(%rbp)" : "(" + addr.base.l + ")");
        dst = std::to_string(addr.offset) + reg;
        return inst + src + ", " + dst + "\n";
    }
    // 3.move Address to Register
    else if (std::holds_alternative<Register>(mv.dst.value) && std::holds_alternative<Address>(mv.src.value))
    {
        auto& addr = std::get<Address>(mv.src.value);
        auto val = std::get<Register>(mv.dst.value);
        if (val.type.GetSize() == 8)
        {
            inst = "movq ";
            if (val.reg.l.starts_with('e'))
            {
                val.reg.l[0] = 'r';
            }
            dst = "%" + val.reg.l;
        }
        else if (val.type.is_basic())
        {
            inst = "movl ";
            dst = "%" + val.reg.l;
        }
        else
        {
            _ASSERT(false);
        }
        // check if addr is a normal address
        if (addr.base.l == "bottom" || addr.base.l == "top")
        {
            auto reg = addr.base.l == "bottom" ? "(%rsp)" : "(%rbp)";
            src = std::to_string(addr.offset) + reg;
            return inst + src + ", " + dst + "\n";
        }
        _ASSERT(false);
        // not normal address, it's string address, so use rip relative addressing
        auto reg = "(%rip)";
        src = addr.base.l + reg;
        return inst + src + ", " + dst + "\n";
    }
    // 4.move Address to Address (invalid instruction)
    else
    {
        _ASSERT(false);
    }
    //auto src = mv.src.GetLabel().l.starts_with('e') ? "%" + mv.src.GetLabel().l : mv.src.GetLabel().l;
    //auto dst = mv.dst.GetLabel().l.starts_with('e') ? "%" + mv.dst.GetLabel().l : mv.dst.GetLabel().l;
    return std::string("movl ") + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Lea& lea)
{
    auto src = lea.src.l.starts_with('e') ? "%" + lea.src.l : lea.src.l;
    auto dst = lea.dst.l.starts_with('e') ? "%" + lea.dst.l : lea.dst.l;
    return std::string("leaq ") + std::to_string(lea.offset) + "(" + src + "), " + dst + "\n";
}
inline std::string to_string(const AS_ElemPtr& mv)
{
    auto elem_ptr = mv.elem_ptr.l.starts_with('e') ? "%" + mv.elem_ptr.l : mv.elem_ptr.l;
    auto out_label = mv.out_label.l.starts_with('e') ? "%" + mv.out_label.l : mv.out_label.l;
    return std::string("leaq ") + std::to_string(mv.elem_offset) + "(" + elem_ptr + "), " + out_label + "\n";
}
inline std::string to_string(const AS_ElemLoad& mv)
{
    auto elem_ptr = mv.elem_ptr.l.starts_with('e') ? "%" + mv.elem_ptr.l : mv.elem_ptr.l;
    auto dst = mv.dst.l.starts_with('e') ? "%" + mv.dst.l : mv.dst.l;
    return std::string("movl ") + std::to_string(mv.elem_offset) + "(" + elem_ptr + "), " + dst + "\n";
}
inline std::string to_string(const AS_ElemStore& mv)
{
    auto elem_ptr = mv.elem_ptr.l.starts_with('e') ? "%" + mv.elem_ptr.l : mv.elem_ptr.l;
    auto src = mv.src.l.starts_with('e') ? "%" + mv.src.l : mv.src.l;
    return std::string("movl ") + src + ", " + std::to_string(mv.elem_offset) + "(" + elem_ptr + ")\n";
}
// movl offset(%rsp, src, 4), dst
inline std::string to_string(const AS_ArrayLoad& mv)
{
    auto src = mv.src.l.starts_with('e') ? "%" + mv.src.l : mv.src.l;
    auto dst = mv.dst.l.starts_with('e') ? "%" + mv.dst.l : mv.dst.l;
    return std::string("movl ") + std::to_string(mv.offset) + "(%rsp," + src + ", " + std::to_string(mv.data_size) + "), " + dst + "\n";
}
// movl src, offset(%rsp, dst, 4)
inline std::string to_string(const AS_ArrayStore& mv)
{
    auto src = mv.src.l.starts_with('e') ? "%" + mv.src.l : mv.src.l;
    auto dst = mv.dst.l.starts_with('e') ? "%" + mv.dst.l : mv.dst.l;
    return std::string("movl ") + src + ", " + std::to_string(mv.offset) + "(%rsp," + dst + ", " + std::to_string(mv.data_size) + ")\n";
}
inline std::string to_string(const AS_Jump& jmp)
{
    return jmp.jump + " " + jmp.target.l + "\n";
}
inline std::string to_string(const AS_Label& lab)
{
    return lab.label.l + ":\n";
}
inline std::string to_string(const AS_Call& call)
{
    std::string str;
    /*for (size_t i = 0; i < call.args.size(); i++)
    {
        str += "pushq %" + call.args[i].l + "\n";
    }*/
    return str + "call " + call.fun_name.l + "\n";
}
inline std::string to_string(const AS_Return& ret)
{
    if (ret.bytes > 0)
    {
        return std::string("addq $" + std::to_string(ret.bytes) + ", %rsp\n") + "popq %rbp\nretq\n";
    }
    return "popq %rbp\nretq\n";
}

std::string to_string(const AS_Instr& instr)
{
    if (auto pstr = std::get_if<AS_String>(&instr))
    {
        return to_string(*pstr);
    }
    else if (auto palloc = std::get_if<AS_StackAlloc>(&instr))
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
    else if (auto pmv = std::get_if<AS_Move_String>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pmv = std::get_if<AS_Move_Type>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto plea = std::get_if<AS_Lea>(&instr))
    {
        return to_string(*plea);
    }
    else if (auto pmv = std::get_if<AS_ElemPtr>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pmv = std::get_if<AS_ElemLoad>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pmv = std::get_if<AS_ElemStore>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pmv = std::get_if<AS_ArrayLoad>(&instr))
    {
        return to_string(*pmv);
    }
    else if (auto pmv = std::get_if<AS_ArrayStore>(&instr))
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
    else if (auto pcall = std::get_if<AS_Call>(&instr))
    {
        return to_string(*pcall);
    }
    else if (auto pret = std::get_if<AS_Return>(&instr))
    {
        return to_string(*pret);
    }
    return std::string();
}