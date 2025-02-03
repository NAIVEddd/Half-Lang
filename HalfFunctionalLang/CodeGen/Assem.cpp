#include "Assem.h"


void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Block>& blocks)
{
    for (size_t i = 0; i < builder.blocks.size(); i++)
    {
        auto& block = builder.blocks[i];
        for (auto& exp : block.exps)
        {
            MunchExp_llvmlike(exp, blocks);
        }
    }
}

void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Block>& blocks)
{
    if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(&exp.exp))
    {
        // 0.0 alloc block for function
        (*pfunc)->alloc.exps.push_back(Half_Ir_Exp(Half_Ir_Jump((*pfunc)->blocks[0].label)));
        size_t func_index = blocks.size();
        (*pfunc)->alloc.label.l = (*pfunc)->name;
        blocks.push_back(AS_Block((*pfunc)->alloc.label));
        // 0.1 map block label to index
        std::map<Temp::Label, size_t> block_map;
        block_map.insert({ (*pfunc)->alloc.label, func_index});
        for (auto& block : (*pfunc)->blocks)
        {
            auto b = AS_Block(block.label);
            block_map.insert({ block.label, blocks.size() });
            blocks.push_back(b);
        }
        // 0.2 update block's pred and succ
        size_t diff = func_index + 1;
        for (size_t i = 0; i < (*pfunc)->blocks.size(); i++)
        {
            auto& block = (*pfunc)->blocks[i];
            for (auto p : block.preds)
            {
                blocks[i + diff].preds.push_back(p + diff);
            }
            for (auto s : block.succs)
            {
                blocks[i + diff].succs.push_back(s + diff);
            }
        }
        blocks[func_index].succs.push_back(func_index + 1);
        blocks[func_index+1].preds.push_back(func_index);

        auto pinstrs = &blocks[func_index].instrs;
        pinstrs->push_back(AS_Label((*pfunc)->name));
        // 1. allocs stack space (according to the number of parameters)
        auto stack_size = (*pfunc)->stack_size;
        pinstrs->push_back(AS_StackAlloc(stack_size));

        // 1.5 store parameters from registers to stack
        auto regs = std::vector<std::string>{ "ecx","edx", "r8d", "r9d" };
        size_t offset = 0;
        for (size_t i = 0; i < (*pfunc)->args.size() && i < regs.size(); ++i)
        {
            AS_Move move(Temp::Label(std::to_string(offset) + "(%rsp)"), Temp::Label(regs[i]));
            move.sz = (*pfunc)->args_size[i];
            pinstrs->push_back(move);
            offset += (*pfunc)->args_size[i];
        }

        // 2. map func blocks with label and index
        std::map<Temp::Label, size_t> fun_block_map;
        for (size_t i = 0; i < (*pfunc)->blocks.size(); i++)
        {
            auto& b = (*pfunc)->blocks[i];
            fun_block_map.insert({ b.label, i });
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
                        if (auto p = fun_block_map.find(l.lab); p != fun_block_map.end())
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
            pinstrs = &blocks[block_map[block.label]].instrs;
            pinstrs->push_back(AS_Label(block.label));

            for (auto& e : block.exps)
            {
                if (auto pphi = std::get_if<std::shared_ptr<Half_Ir_Phi>>(&e.exp))
                {
                    // shouldn't be here
                    _ASSERT(false);
                }
                else
                {
                    MunchExp_llvmlike(e, *pinstrs);
                }
            }
        }

        // 5. return
        _ASSERT(std::get_if<AS_Return>(&pinstrs->back()));
        pinstrs->back() = AS_Return(stack_size);
        return;
    }
}

void MunchExps_llvmlike(const Builder& builder, std::vector<AS_Instr>& instrs)
{
    for (size_t i = 0; i < builder.blocks.size(); i++)
    {
        auto& block = builder.blocks[i];
        for (auto& exp : block.exps)
        {
            MunchExp_llvmlike(exp, instrs);
        }
    }
    for (auto& [f, s] : builder.strings)
    {
        instrs.push_back(AS_String(f, s));
    }
}

void Munch_GetElementPtr(const Half_Ir_GetElementPtr& exp, std::vector<AS_Instr>& instrs)
{
    Half_Type_Info type = exp.source_element_type;// exp.out_address.real_address->type;
    auto& real_addr = exp.out_address.real_address;
    for (size_t i = 0; i < exp.in_indexs.size(); i++)
    {
        auto& in = exp.in_indexs[i];
        if (auto p = std::get_if<Half_Type_Info::ArrayType>(&type.type))
        {
            type = *p->type;
            if (auto pconst = std::get_if<Half_Ir_Const>(&in))
            {
                if (pconst->n != 0)
                {
                    real_addr->offset += type.GetSize() * pconst->n;
                }
            }
            else if (auto pvar = std::get_if<Value>(&in))
            {
                _ASSERT(std::holds_alternative<Register>(pvar->value));
                auto& reg = std::get<Register>(pvar->value);
                Half_Ir_Const ty_sz((int)type.GetSize());
                auto ty_sz_res = ty_sz.GetResult();
                Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Multy, std::get<Register>(ty_sz_res.value), reg, Temp::NewLabel());
                Half_Ir_Ext ext(std::get<Register>(binop.GetResult().value), binop.GetResult().GetType(), Temp::NewLabel());
                Half_Ir_FetchPtr fetch(Address{ Half_Type_Info::PointerType(std::make_shared<Half_Type_Info>(type)), real_addr, Temp::NewLabel() });
                auto ext_res = ext.GetResult();
                auto fetch_res = fetch.GetResult();
                Half_Ir_BinOp binop2(Half_Ir_BinOp::Oper::Plus, std::get<Register>(ext_res.value), std::get<Register>(fetch_res.value), Temp::NewLabel());
                MunchExp_llvmlike(Half_Ir_Exp(ty_sz), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(binop), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(ext), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(fetch), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(binop2), instrs);
                real_addr->base = binop2.GetResult().GetLabel();
                real_addr->offset = 0;
            }
        }
        else if (auto p = std::get_if<Half_Type_Info::PointerType>(&type.type))
        {
            type = *p->type;
            if (auto pconst = std::get_if<Half_Ir_Const>(&in))
            {
                if (pconst->n != 0)
                {
                    real_addr->offset += type.GetSize() * pconst->n;
                }
            }
            else if (auto pvar = std::get_if<Value>(&in))
            {
                _ASSERT(std::holds_alternative<Register>(pvar->value));
                auto& reg = std::get<Register>(pvar->value);
                Half_Ir_Const ty_sz((int)type.GetSize());
                auto ty_sz_res = ty_sz.GetResult();
                Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Multy, std::get<Register>(ty_sz_res.value), reg, Temp::NewLabel());
                Half_Ir_Ext ext(std::get<Register>(binop.GetResult().value), binop.GetResult().GetType(), Temp::NewLabel());
                Half_Ir_FetchPtr fetch(Address{ type, real_addr, Temp::NewLabel() });
                RealAddress addr = { type, real_addr->base, real_addr->offset };
                Half_Ir_Load load(Address{ real_addr->type, std::make_shared<RealAddress>(addr), Temp::NewLabel()});

                auto ext_res = ext.GetResult();
                auto load_res = load.GetResult();
                Half_Ir_BinOp binop2(Half_Ir_BinOp::Oper::Plus, std::get<Register>(ext_res.value), std::get<Register>(load_res.value), Temp::NewLabel());
                MunchExp_llvmlike(Half_Ir_Exp(ty_sz), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(binop), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(ext), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(load), instrs);
                MunchExp_llvmlike(Half_Ir_Exp(binop2), instrs);
                real_addr->base = binop2.GetResult().GetLabel();
                real_addr->offset = 0;
            }
        }
        else if (auto p = std::get_if<Half_Type_Info::StructType>(&type.type))
        {
            auto idx = std::get<Half_Ir_Const>(in);
            real_addr->offset += p->GetField(idx.n).offset;
            type = p->GetField(idx.n).type;
        }
    }
}

void MunchExp_llvmlike(const Half_Ir_Exp& exp, std::vector<AS_Instr>& instrs)
{
    if (auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(&exp.exp))
    {
        AS_Move_Type move((*pload)->out_register, (*pload)->address);
        instrs.push_back(move);
        return;
    }
    else if (auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(&exp.exp))
    {
        AS_Move_Type move((*pstore)->address, (*pstore)->value);
        instrs.push_back(move);
        return;
    }
    else if (auto pext = std::get_if<std::shared_ptr<Half_Ir_Ext>>(&exp.exp))
    {
        AS_Ext ext((*pext)->out_register.reg, (*pext)->value.reg);
        instrs.push_back(ext);
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
        }
        else
        {
            _ASSERT(false);
        }
    }
    else if (auto plabel = std::get_if<std::shared_ptr<Half_Ir_Label>>(&exp.exp))
    {
        instrs.push_back(AS_Label((*plabel)->lab));
        return;
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(&exp.exp))
    {
        instrs.push_back(AS_Declear((*pfunc)->name));
        instrs.push_back(AS_Label((*pfunc)->name));

        // 1. allocs stack space (according to the number of parameters)
        auto stack_size = (*pfunc)->stack_size;
        instrs.push_back(AS_StackAlloc(stack_size));

        // 1.5 store parameters from registers to stack
        auto regs = std::vector<std::string>{  "ecx","edx", "r8d", "r9d" };
        size_t offset = 0;
        for (size_t i = 0; i < (*pfunc)->args.size() && i < regs.size(); ++i)
        {
            AS_Move move(Temp::Label(std::to_string(offset) + "(%rsp)"), Temp::Label(regs[i]));
            move.sz = (*pfunc)->args_size[i];
            instrs.push_back(move);
            offset += (*pfunc)->args_size[i];
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
        Munch_GetElementPtr(**pgetptr, instrs);
        return;
    }
    else if (auto pfetch = std::get_if<std::shared_ptr<Half_Ir_FetchPtr>>(&exp.exp))
    {
        auto& fetch = **pfetch;
        auto base_l = fetch.ptr.real_address->base.l;
        auto reg = base_l == "bottom" ? "rsp"
            : (base_l == "top" ? "rbp" : base_l);
        auto l = Temp::Label(reg);
        auto offset = fetch.ptr.real_address->offset;
        AS_Lea lea(fetch.out_label, l, offset);
        instrs.push_back(lea);
        return;
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Ir_BinOp>>(&exp.exp))
    {
        auto& binop = **pop;
        auto movl = binop.left.reg;
        auto movr = binop.right.reg;
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
        AS_Oper oper(tostring((*pop)->op), movl, movr);
        binop.left.type.GetSize() == 8 ? oper.sz = 8 : oper.sz = 4;
        binop.right.type.GetSize() == 8 ? oper.sz = 8 : oper.sz = 4;
        instrs.push_back(oper);
        AS_Move_Type move((*pop)->GetResult(), (*pop)->left);
        instrs.push_back(move);
        return;
    }
    else if (auto pcall = std::get_if<std::shared_ptr<Half_Ir_Call>>(&exp.exp))
    {
        auto& call = **pcall;
        std::vector<Temp::Label> args(call.args.size());
        for (size_t i = 0; i < call.args.size(); i++)
        {
            args[i] = call.args[i].name;
        }
        instrs.push_back(AS_Call(call.fun_name, args, call.out_register.reg));
        return;
    }
    else if (auto pret = std::get_if<std::shared_ptr<Half_Ir_Return>>(&exp.exp))
    {
        auto v = (*pret)->value;
        auto label = v.GetLabel();
        instrs.push_back(AS_Move(Temp::Label("%eax"), label));
        instrs.push_back(AS_Return());
        return;
    }
    else if (auto pmove = std::get_if<std::shared_ptr<Half_Ir_Move>>(&exp.exp))
    {
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
        if (move.type.GetSize() == 8)
        {
            printf("Assem.cpp: line 436\n");
            Register d;
            Register s;
            d.reg = dst.l;
            s.reg = src.l;
            d.type = move.type;
            s.type = move.type;
            AS_Move_Type mv(d, s);
            instrs.push_back(mv);
            return;
        }
        instrs.push_back(AS_Move(dst, src));
        return;
    }
    else if (auto pbr = std::get_if<std::shared_ptr<Half_Ir_Branch>>(&exp.exp))
    {
        auto& branch = **pbr;

        instrs.push_back(AS_Oper("cmp", branch.condition.left.name, branch.condition.right.name));
        instrs.push_back(AS_Jump(branch.condition.op, branch.true_label));
        instrs.push_back(AS_Jump(Half_Ir_Compare::GetNot(branch.condition.op), branch.false_label));
        return;
    }
    else if (auto pjmp = std::get_if<std::shared_ptr<Half_Ir_Jump>>(&exp.exp))
    {
        auto& jmp = **pjmp;
        instrs.push_back(AS_Jump("jmp", jmp.target));
        return;
    }
    else
    {
        printf("MunchExp None-- invalid expr--\n");
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
    std::string inst;
    if (op.sz == 8)
    {
        inst = op.assem + "q ";
        std::string src = op.src.l;
        if (op.src.l.starts_with('e'))
        {
            src[0] = 'r';
            src = "%" + src;
        }
        std::string dst = op.dst.l;
        if (op.dst.l.starts_with('e'))
        {
            dst[0] = 'r';
            dst = "%" + dst;
        }
        return inst + src + ", " + dst + "\n";
    }
    auto src = op.src.l.starts_with('e') ? "%" + op.src.l : op.src.l;
    auto dst = op.dst.l.starts_with('e') ? "%" + op.dst.l : op.dst.l;
    return op.assem + "l " + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Ext& ext)
{
    auto src = ext.src.l.starts_with('e') ? "%" + ext.src.l : ext.src.l;
    std::string dst = ext.dst.l;
    if (ext.dst.l.starts_with('e'))
    {
        dst[0] = 'r';
        dst = "%" + dst;
    }
    return "movslq " + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Move& mv)
{
    if (mv.src.l == mv.dst.l)
    {
        //TODO: remove this, it's a hack
        // munch exp-binop add one instr with same src and dst
        return "# --- #\n";
    }
    std::string inst;
    if (mv.sz == 8)
    {
        inst = "movq ";
        std::string src = mv.src.l;
        if (mv.src.l.starts_with('e') || mv.src.l.starts_with('r'))
        {
            src[0] = 'r';
            src = "%" + src;
        }
        std::string dst = mv.dst.l;
        if (mv.dst.l.starts_with('e') || mv.dst.l.starts_with('r'))
        {
            dst[0] = 'r';
            dst = "%" + dst;
        }
        return inst + src + ", " + dst + "\n";
    }
    auto src = mv.src.l.starts_with('e') || mv.src.l.starts_with('r') ? "%" + mv.src.l : mv.src.l;
    auto dst = mv.dst.l.starts_with('e') || mv.dst.l.starts_with('r') ? "%" + mv.dst.l : mv.dst.l;
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
        auto reg = addr.real_address->base.l == "bottom" ? "rsp" :
            (addr.real_address->base.l == "top" ? "rbp" : addr.real_address->base.l);
        if (reg.starts_with('e'))
        {
            reg[0] = 'r';
        }
        reg = "(%" + reg + ")";
        dst = std::to_string(addr.real_address->offset) + reg;
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

            inst = "movl ";
            dst = "%" + val.reg.l;
            //_ASSERT(false);
        }
        // check if addr is a normal address
        if (addr.real_address->base.l == "bottom" || addr.real_address->base.l == "top")
        {
            auto reg = addr.real_address->base.l == "bottom" ? "(%rsp)" : "(%rbp)";
            src = std::to_string(addr.real_address->offset) + reg;
            return inst + src + ", " + dst + "\n";
        }
        // not normal address, it's calculated address
        auto reg = addr.real_address->base.l;
        if (reg.starts_with('e'))
        {
            reg[0] = 'r';
        }
        src = "(%" + reg + ")";
        return inst + src + ", " + dst + "\n";
    }
    // 4.move Address to Address (invalid instruction)
    else
    {
        _ASSERT(false);
    }
    return std::string("movl ") + src + ", " + dst + "\n";
}
inline std::string to_string(const AS_Lea& lea)
{
    auto src = lea.src.l;
    if (src.starts_with('e') || src.starts_with('r'))
    {
        src[0] = 'r';
        src = "%" + src;
    }
    std::string dst = lea.dst.l;
    if (dst.starts_with('e') || dst.starts_with('r'))
    {
        dst[0] = 'r';
        dst = "%" + dst;
    }
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
    else if (auto pdecl = std::get_if<AS_Declear>(&instr))
    {
        return ".globl " + pdecl->func + "\n";
    }
    else if (auto pext = std::get_if<AS_Ext>(&instr))
    {
        return to_string(*pext);
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