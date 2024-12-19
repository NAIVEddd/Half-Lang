#include "Semant.h"
#include "Symbol.h"
#include "Builder.h"
#include <algorithm>


/*auto IR_Make_Char =
[](char c) -> Half_IR
    {
        // TODO
        return Half_Ir_Const(c);
    };

auto IR_Make_Value =
[](std::shared_ptr<Half_Value>& value) -> Half_IR
    {
        static auto overload = overloaded{ IR_Make_Char, IR_Make_Const, IR_Make_Float, IR_Make_String };
        _ASSERT(value);
        return std::visit(overload, value->value);
    };*/



Builder Default_builder;

Half_Ir_Exp Trans_Expr(Half_Expr& expr)
{
    auto table = std::make_shared<Table>();
    auto exp_ir = Trans_Expr(table, expr);
    return exp_ir;
}

Half_Ir_Exp Trans_Var(std::shared_ptr<Table>& table, Half_Var& var)
{
    auto symb = table->find(var.name());
    if (!symb)
    {
        _ASSERT(false);
        return Half_Ir_Const(-1);
    }
    return Half_Ir_Memory(symb.value().offset);
}

Half_Ir_Exp Trans_Value(std::shared_ptr<Table>& table, Half_Value& value)
{
    if (auto pc = std::get_if<char>(&value.value))
    {
        return Half_Ir_Const(-2);
    }
    else if (auto pi = std::get_if<int>(&value.value))
    {
        return Half_Ir_Const(*pi);
    }
    else if (auto pf = std::get_if<float>(&value.value))
    {
        return Half_Ir_Const(*pf);
    }
    else if (auto ps = std::get_if<std::string>(&value.value))
    {
        return Half_Ir_Const(-3);
    }
    _ASSERT(false);
    return Half_Ir_Const(-4);
}

Half_Ir_Exp Trans_Funcall(std::shared_ptr<Table>& table, Half_Funcall& call)
{
    auto name = Half_Ir_Name(Temp::Label(call.name));
    std::vector<Half_Ir_Exp> arg_exp(call.args.size());
    for (size_t i = 0; i < call.args.size(); i++)
    {
        arg_exp[i] = Trans_Expr(table, call.args[i]);
    }
    return Half_Ir_Call(name, arg_exp);
}

Half_Ir_Exp Trans_Op(std::shared_ptr<Table>& table, Half_Op& op);
Half_Ir_Exp Trans_OpExpr(std::shared_ptr<Table>& table, Half_Op::Half_OpExpr& op)
{
    if (auto pvar = std::get_if<Half_Var>(&op))
    {
        auto& var = *pvar;
        return Trans_Var(table, var);
    }
    else if (auto pvalue = std::get_if<Half_Value>(&op))
    {
        auto& value = *pvalue;
        return Trans_Value(table, value);
    }
    else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    {
        auto& funcall = *pfuncall;
        return Trans_Funcall(table, funcall);
    }
    else if (auto pop = std::get_if<Half_Op>(&op))
    {
        auto& binop = *pop;
        return Trans_Op(table, binop);
    }

    _ASSERT(false);
    return Half_Ir_Const(-3);
}

Half_Ir_Exp Trans_Op(std::shared_ptr<Table>& table, Half_Op& op)
{
    auto l = Trans_OpExpr(table, *op.left);
    auto r = Trans_OpExpr(table, *op.right);
    
    auto t = Half_Ir_Label(Temp::NewLabel());
    Default_builder.AddExp(Half_Ir_Move(t, Half_Ir_BinOp(op.op, l, r)));

    return t;
}

Half_Ir_Exp Trans_Assign(std::shared_ptr<Table>& table, Half_Assign& assign)
{
    auto l = Trans_Var(table, assign.left);
    auto r = Trans_Expr(table, assign.right);
    Default_builder.AddExp(Half_Ir_Move(l, r));
    return l;
}

Half_Ir_Exp Trans_Let(std::shared_ptr<Table>& table, Half_Let& let)
{
    auto s = Symbol();
    s.name = let.def.left.name();

    table->insert(s);
    //auto alloc = Half_Ir_Alloc(4);
    //auto assign = Trans_Assign(table, let.def);
    //table->labels.insert({ s.name, Temp::NewLabel(table->labels.size()) });
    //return Half_Ir_Seq(std::vector<Half_Ir_Exp>{alloc, assign});
    return Trans_Assign(table, let.def);
}

Half_Ir_Exp Trans_While(std::shared_ptr<Table>& table, Half_While& _while)
{
    return Half_Ir_Const(-10);
}

Half_Ir_Exp Trans_For(std::shared_ptr<Table>& table, Half_For& _for)
{
    return Half_Ir_Const(-11);
}

Half_Ir_Exp Trans_Expr(std::shared_ptr<Table>& table, Half_Expr& expr)
{
    if (auto pvar = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
    {
        auto& var = **pvar;
        return Trans_Var(table, var);
    }
    else if (auto pvalue = std::get_if<std::shared_ptr<Half_Value>>(&expr.expr))
    {
        auto& value = **pvalue;
        return Trans_Value(table, value);
    }
    else if (auto pcall = std::get_if<std::shared_ptr<Half_Funcall>>(&expr.expr))
    {
        auto& call = **pcall;
        return Trans_Funcall(table, call);
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Op>>(&expr.expr))
    {
        auto& op = **pop;
        return Trans_Op(table, op);
    }
    else if (auto passign = std::get_if<std::shared_ptr<Half_Assign>>(&expr.expr))
    {
        auto& assign = **passign;
        return Trans_Assign(table, assign);
    }
    else if (auto plet = std::get_if<std::shared_ptr<Half_Let>>(&expr.expr))
    {
        auto& let = **plet;
        return Trans_Let(table, let);
    }
    else if (auto pwhile = std::get_if<std::shared_ptr<Half_While>>(&expr.expr))
    {
        auto& _while = **pwhile;
        return Trans_While(table, _while);
    }
    else if (auto pfor = std::get_if<std::shared_ptr<Half_For>>(&expr.expr))
    {
        auto& _for = **pfor;
        return Trans_For(table, _for);
    }
    else if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&expr.expr))
    {
        auto& vec = **pvec;
        auto seq = std::vector<Half_Ir_Exp>(vec.size());
        for (size_t i = 0; i < vec.size(); i++)
        {
            seq[i] = Trans_Expr(table, vec[i]);
        }
        return Half_Ir_Seq(seq);
    }
    else if (auto ptype = std::get_if<std::shared_ptr<Half_TypeDecl>>(&expr.expr))
    {
        return Half_Ir_Const(-5);
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_FuncDecl>>(&expr.expr))
    {
        auto current_block_size = Default_builder.blocks.size();
        Half_Ir_Function func_ir;
        auto& func = **pfunc;

        func_ir.args = func.parameters;

        Stack stack;

        auto funcscope = stack.NewScope();
        //auto funcscope = Table::begin_scope(table);
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            Symbol smb;
            smb.name = func.parameters[i].var_name;
            funcscope->insert(smb);
            funcscope->labels.insert({ smb.name, Temp::NewLabel(funcscope->labels.size()) });
        }
        auto block_entry = Default_builder.NewBlock("entry");
        Default_builder.SetInsertPoint(block_entry);
        Half_Ir_Label func_label(Temp::Label(func.name));
        Half_Ir_Exp func_label_exp(func_label);
        Default_builder.AddExp(block_entry, func_label_exp);

        auto seq = Trans_Expr(funcscope, func.body);
        if (auto pseq = std::get_if<std::shared_ptr<Half_Ir_Seq>>(&seq.exp))
        {
            auto ret = (**pseq).seq.back();
            Default_builder.AddExp(Half_Ir_Return(ret));
            Half_Ir_Seq res = Default_builder.blocks[block_entry].exps;
            Default_builder.blocks.erase(Default_builder.blocks.begin() + block_entry);
            Default_builder.insert_point = Default_builder.blocks.size() - 1;
            return res;
        }
        return Half_Ir_Const(-6);
    }

    _ASSERT(false);
    return Half_Ir_Const(0);
}



Half_Ir_Name Trans_Var_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder)
{
    auto symbol = table->find(var.name());
    if (!symbol)
    {
        _ASSERT(false);
        return Half_Ir_Name("Varabile not defined:(" + var.name() + ")");
    }
    auto load = Half_Ir_Load(symbol.value().offset, Temp::NewLabel());
    builder.AddExp(load);
    return load.out_label;
}

Half_Ir_Name Trans_Op_Builder(std::shared_ptr<Table>& table, Half_Op& op, Builder& builder);
Half_Ir_Name Trans_OpExpr_Builder(std::shared_ptr<Table>& table, Half_Op::Half_OpExpr& op, Builder& builder)
{
    if (auto pvar = std::get_if<Half_Var>(&op))
    {
        auto& var = *pvar;
        return Trans_Var_Builder(table, var, builder);
    }
    else if (auto pvalue = std::get_if<Half_Value>(&op))
    {
        auto& value = *pvalue;
        auto value_expr = Half_Expr(value);
        return Trans_Expr(table, builder, value_expr);
        //return Trans_Value_Builder(table, value, builder);
    }
    else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    {
        auto& funcall = *pfuncall;
        auto funcall_expr = Half_Expr(funcall);
        return Trans_Expr(table, builder, funcall_expr);
        //return Trans_Funcall_Builder(table, funcall, builder);
    }
    else if (auto pop = std::get_if<Half_Op>(&op))
    {
        auto& binop = *pop;
        return Trans_Op_Builder(table, binop, builder);
    }

    _ASSERT(false);
    return Half_Ir_Name();
}

Half_Ir_Name Trans_Op_Builder(std::shared_ptr<Table>& table, Half_Op& op, Builder& builder)
{
    auto l = Trans_OpExpr_Builder(table, *op.left, builder);
    auto r = Trans_OpExpr_Builder(table, *op.right, builder);

    auto op_ir = Half_Ir_LlvmBinOp(op.op, l, r, Temp::NewLabel());
    builder.AddExp(op_ir);

    return op_ir.out_label;
}

Half_Ir_Name Trans_Expr(Half_Expr& expr, Builder& builder)
{
    auto table = std::make_shared<Table>();
    auto exp_result = Trans_Expr(table, builder, expr);
    return exp_result;
}

Half_Ir_Name Trans_CondOp(std::shared_ptr<Table>& table, Builder& builder, Half_Op::Half_OpExpr& op, std::string prefix, Temp::Label true_label, Temp::Label false_label)
{
    if (auto pvar = std::get_if<Half_Var>(&op))
    {
        auto& var = *pvar;
        auto tmp = Half_Expr(var);
        return Trans_Expr(table, builder, tmp);
    }
    else if (auto pvalue = std::get_if<Half_Value>(&op))
    {
        auto& value = *pvalue;
        auto tmp = Half_Expr(value);
        return Trans_Expr(table, builder, tmp);
    }
    else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    {
        auto& funcall = *pfuncall;
        auto tmp = Half_Expr(funcall);
        return Trans_Expr(table, builder, tmp);
    }
    else if (auto pop = std::get_if<Half_Op>(&op))
    {
        auto& binop = *pop;

        Temp::Label new_block_label = builder.GenBlockLabel(prefix + "_right");
        Temp::Label left_true_label = true_label;
        Temp::Label left_false_label = false_label;
        if (binop.op == "&&")
        {
            left_true_label = new_block_label;
            left_false_label = false_label;
        }
        else if (binop.op == "||")
        {
            left_true_label = true_label;
            left_false_label = new_block_label;
        }
        auto l = Trans_CondOp(table, builder, *binop.left, prefix + "_left", left_true_label, left_false_label);

        // new block for right
        if (binop.op == "&&" || binop.op == "||")
        {
            auto right_entry = builder.NewBlock(new_block_label);
            builder.SetInsertPoint(right_entry);
        }
        auto r = Trans_CondOp(table, builder, *binop.right, prefix + "_right", true_label, false_label);

        if (binop.op != "&&" && binop.op != "||")
        {
            auto cmp = Half_Ir_Compare(binop.op, l, r, Temp::NewLabel());
            auto br = Half_Ir_LlvmBranch(cmp, true_label, false_label);
            builder.AddExp(br);
            return cmp.out_label;
        }
        
        return r;
    }

    // Todo: support other type
    _ASSERT(false);
    return Half_Ir_Name();
}

Half_Ir_Name Trans_If_Cond(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& cond, std::string prefix, Temp::Label true_label, Temp::Label false_label)
{
    //  now it's support multiple conditions like (a < b && c > d)
    if (auto pop = std::get_if<std::shared_ptr<Half_Op>>(&cond.expr))
    {
        auto& op = **pop;
        Half_Op::Half_OpExpr temp_op = op;
        return Trans_CondOp(table, builder, temp_op, prefix, true_label, false_label);
    }
    printf("only support binary op exp type in if.cond\n");
    _ASSERT(false);
    return Half_Ir_Name();
}

Half_Ir_Name Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr)
{
    // return std::visit([&](auto&& arg) { return Trans_Expr_Impl(table, builder, arg); }, expr.expr);
    if (auto plet = std::get_if<std::shared_ptr<Half_Let>>(&expr.expr))
    {
        auto& let = **plet;
        // Todo: move implementation to function

        auto s = Symbol();
        s.name = let.def.left.name();

        table->insert(s);

        _ASSERT(builder.block_alloc_entry != -1);
        auto alloc = Half_Ir_Alloc(s.offset, Temp::NewLabel());
        builder.AddExp(builder.block_alloc_entry, alloc);

        auto assign = Half_Expr(let.def);
        return Trans_Expr(table, builder, assign);
    }
    else if (auto pvar = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
    {
        auto& var = **pvar;
        return Trans_Var_Builder(table, var, builder);
    }
    else if (auto pvalue = std::get_if<std::shared_ptr<Half_Value>>(&expr.expr))
    {
        auto& value = **pvalue;
        if (auto pint = std::get_if<int>(&value.value))
        {
            auto const_ir = Half_Ir_Const(*pint);
            builder.AddExp(const_ir);
            return Half_Ir_Name(const_ir.out_label);
        }

        // TODO: support other value type
        _ASSERT(false);
    }
    else if (auto pop = std::get_if<std::shared_ptr<Half_Op>>(&expr.expr))
    {
        auto& op = **pop;
        return Trans_Op_Builder(table, op, builder);
    }
    else if (auto passign = std::get_if<std::shared_ptr<Half_Assign>>(&expr.expr))
    {
        auto& assign = **passign;
        auto rval = Trans_Expr(table, builder, assign.right);
        auto symbol = table->find(assign.left.name());
        if (!symbol)
        {
            _ASSERT(false);
            return Half_Ir_Name("Varabile not defined:(" + assign.left.name() + ")");
        }
        auto store = Half_Ir_Store(symbol.value().offset, rval.name);
        builder.AddExp(store);
        return rval.name;
    }
    else if (auto pfuncall = std::get_if<std::shared_ptr<Half_Funcall>>(&expr.expr))
    {
        auto& funcall = **pfuncall;
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_FuncDecl>>(&expr.expr))
    {
        auto& func = **pfunc;

        Stack stack;

        auto current_block_size = builder.blocks.size();
        Half_Ir_Function func_ir;
        func_ir.name = func.name;
        func_ir.args = func.parameters;

        // 1. Create a new block for the function
        auto alloc_entry = builder.NewBlock("stack_alloc");
        auto block_entry = builder.NewBlock("entry");
        builder.SetAllocEntry(alloc_entry);
        builder.SetInsertPoint(block_entry);

        // 2. Create a new scope for the function
        auto funcscope = stack.NewScope();
        //auto funcscope = Table::begin_scope(table);
        // 3. Add the function parameters to the scope(allocation stack to store the parameters)
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            Symbol smb;
            smb.name = func.parameters[i].var_name;
            funcscope->insert(smb);
            funcscope->labels.insert({ smb.name, Temp::NewLabel(funcscope->labels.size()) });
            auto alloc = Half_Ir_Alloc(smb.offset, Temp::NewLabel());
            builder.AddExp(alloc_entry, alloc);
        }
        
        // 4. Translate the function body
        auto last_val = Trans_Expr(funcscope, builder, func.body);
        Half_Ir_Return ret(last_val);
        builder.AddExp(ret);

        // 5. Move all blocks in builder to function ir
        func_ir.alloc = builder.blocks[alloc_entry];
        for (size_t i = block_entry; i < builder.blocks.size(); i++)
        {
            func_ir.blocks.push_back(builder.blocks[i]);
        }
        // 6. Remove the blocks from the builder
        builder.blocks.erase(builder.blocks.begin() + block_entry, builder.blocks.end());
        builder.blocks.erase(builder.blocks.begin() + builder.block_alloc_entry);
        builder.insert_point = builder.blocks.size() - 1;
        builder.block_alloc_entry = -1;
        builder.AddExp(func_ir);
        return Half_Ir_Name("FuncDecl, should't use this label " + func.name);
    }
    else if (auto pif = std::get_if<std::shared_ptr<Half_If>>(&expr.expr))
    {
        auto& _if = **pif;

        // use temp builder to translate condition and body
        //   insert these blocks to the main builder in the end
        //   to make sure the blocks are in the right order

        auto cond_label = builder.GenBlockLabel("if_cond");
        auto block_if_true_label = builder.GenBlockLabel("if_true");
        auto block_if_true_phi_label = builder.GenBlockLabel("if_true_phi");
        auto block_if_false_label = builder.GenBlockLabel("if_false");
        auto block_if_false_phi_label = builder.GenBlockLabel("if_false_phi");
        auto block_if_end_label = builder.GenBlockLabel("if_merge");

        auto if_result_phi = Half_Ir_Phi(Temp::NewLabel());

        // jump to condition block, if current block is not empty
        if (builder.blocks[builder.insert_point].exps.size() > 0)
        {
            auto jmp_to_cond = Half_Ir_Jump(cond_label);
            builder.AddExp(jmp_to_cond);
        }

        auto true_used_var = std::map<Half_Var, Half_Ir_Name>();
        auto false_used_var = std::map<Half_Var, Half_Ir_Name>();
        auto common_used_var = std::vector<std::pair<Half_Ir_Name, Half_Ir_Label>>();

        /// translate condition
        auto cond_entry = builder.NewBlock(cond_label);
        builder.SetInsertPoint(cond_entry);
        Trans_If_Cond(table, builder, _if.condition, "if_cond",
            block_if_true_label, block_if_false_label);

        /// translate body
        auto insert_var = [](std::map<Half_Var, Half_Ir_Name>& used_var, const Table& tab, const Half_Expr& e, Half_Ir_Name& label)
            {
                if (auto passign = std::get_if<std::shared_ptr<Half_Assign>>(&e.expr))
                {
                    auto& assign = **passign;
                    if (tab.find(assign.left.name(), false))
                    {
                        used_var[assign.left] = label;
                    }
                }
            };
        auto if_true_entry = builder.NewBlock(block_if_true_label);
        builder.SetInsertPoint(if_true_entry);
        if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&_if.trueExpr.expr))
        {
            auto& vec = **pvec;
            auto true_table = Table::begin_scope(table);
            Half_Ir_Name result;
            for (size_t i = 0; i < vec.size(); i++)
            {
                result = Trans_Expr(true_table, builder, vec[i]);
                insert_var(true_used_var, true_table, vec[i], result);
            }
            auto jmp_to_phi = Half_Ir_Jump(block_if_true_phi_label);
            builder.AddExp(jmp_to_phi);

            // insert last result to phi node
            builder.SetInsertPoint(builder.NewBlock(block_if_true_phi_label));
            if_result_phi.Insert(result, block_if_true_phi_label);
            auto jmp_to_end = Half_Ir_Jump(block_if_end_label);
            builder.AddExp(jmp_to_end);
        }
        else
        {
            _ASSERT(false);
        }

        auto if_false_entry = builder.NewBlock(block_if_false_label);
        builder.SetInsertPoint(if_false_entry);
        if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&_if.falseExpr.expr))
        {
            auto& vec = **pvec;
            auto false_table = Table::begin_scope(table);
            Half_Ir_Name result;
            for (size_t i = 0; i < vec.size(); i++)
            {
                result = Trans_Expr(false_table, builder, vec[i]);
                insert_var(false_used_var, false_table, vec[i], result);
            }
            auto jmp_to_phi = Half_Ir_Jump(block_if_false_phi_label);
            builder.AddExp(jmp_to_phi);

            // insert last result to phi node
            builder.SetInsertPoint(builder.NewBlock(block_if_false_phi_label));
            if_result_phi.Insert(result, block_if_false_phi_label);
            auto jmp_to_end = Half_Ir_Jump(block_if_end_label);
            builder.AddExp(jmp_to_end);
        }
        else
        {
            _ASSERT(false);
        }

        // set insert point to the end block
        //  to make sure the phi node is in the end
        auto if_end_entry = builder.NewBlock(block_if_end_label);
        builder.SetInsertPoint(if_end_entry);

        // find out common used var in true and false block
        auto find_begin = true_used_var.begin();
        auto find_end = true_used_var.end();
        for (auto& v : false_used_var)
        {
            auto t_iter = true_used_var.find(v.first);
            if (t_iter != true_used_var.end())
            {
                // print debug info
                printf("Common used var: %s\n", v.first.name().c_str());
                // convert common used var to phi
                auto phi = Half_Ir_Phi(Temp::NewLabel());
                phi.Insert(t_iter->second, block_if_true_phi_label);
                phi.Insert(v.second, block_if_false_phi_label);
                builder.AddExp(phi);
            }
        }

        builder.AddExp(if_result_phi);
        return Half_Ir_Name(if_result_phi.result);
    }
    else if (auto pwhile = std::get_if<std::shared_ptr<Half_While>>(&expr.expr))
    {
        auto& _while = **pwhile;
        auto cond_label = builder.GenBlockLabel("while_cond");
        auto body_label = builder.GenBlockLabel("while_body");
        auto end_label = builder.GenBlockLabel("while_end");

        // jump to condition block, if current block is not empty
        if (builder.blocks[builder.insert_point].exps.size() > 0)
        {
            auto jmp_to_cond = Half_Ir_Jump(cond_label);
            builder.AddExp(jmp_to_cond);
        }

        auto cond_block = builder.NewBlock(cond_label);
        builder.SetInsertPoint(cond_block);
        Trans_If_Cond(table, builder, _while.condition, "while_cond",
            body_label, end_label);

        auto body_block = builder.NewBlock(body_label);
        builder.SetInsertPoint(body_block);
        if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&_while.body.expr))
        {
            auto& vec = **pvec;
            auto while_table = Table::begin_scope(table);
            for (size_t i = 0; i < vec.size(); i++)
            {
                Trans_Expr(while_table, builder, vec[i]);
            }
            auto jmp_to_begin = Half_Ir_Jump(cond_label);
            builder.AddExp(jmp_to_begin);
        }
        else
        {
            _ASSERT(false);
        }
        auto end_block = builder.NewBlock(end_label);
        builder.SetInsertPoint(end_block);
        return Half_Ir_Name("While-expr-end-nil");
    }
    else if (auto pfor = std::get_if<std::shared_ptr<Half_For>>(&expr.expr))
    {
        auto& _for = **pfor;
        auto cond_label = builder.GenBlockLabel("for_cond");
        auto body_label = builder.GenBlockLabel("for_body");
        auto end_label = builder.GenBlockLabel("for_end");

        // for table
        auto for_table = Table::begin_scope(table);
        auto for_init = Half_Let(Half_Assign(_for.var, _for.start));
        auto init_expr = Half_Expr(for_init);
        Trans_Expr(for_table, builder, init_expr);

        // jump to condition block, if current block is not empty
        if (builder.blocks[builder.insert_point].exps.size() > 0)
        {
            auto jmp_to_cond = Half_Ir_Jump(cond_label);
            builder.AddExp(jmp_to_cond);
        }

        // condition block
        auto cond_block = builder.NewBlock(cond_label);
        builder.SetInsertPoint(cond_block);
        auto cond = Half_Op(_for.isup? "<" : ">", Half_Op::Half_OpExpr(_for.var), ConvertToOpExpr(_for.end));
        auto cond_expr = Half_Expr(cond);
        Trans_If_Cond(for_table, builder, cond_expr, "for_cond",
            body_label, end_label);

        // body block
        auto body_block = builder.NewBlock(body_label);
        builder.SetInsertPoint(body_block);
        if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&_for.body.expr))
        {
            auto& vec = **pvec;
            for (size_t i = 0; i < vec.size(); i++)
            {
                Trans_Expr(for_table, builder, vec[i]);
            }
        }
        else
        {
            _ASSERT(false);
        }

        // update condition variable
        auto update = Half_Op(_for.isup ? "+" : "-", Half_Op::Half_OpExpr(_for.var), Half_Op::Half_OpExpr(Half_Value(1)));
        auto update_assign = Half_Assign(_for.var, update);
        auto update_expr = Half_Expr(update_assign);
        Trans_Expr(for_table, builder, update_expr);

        // jump to condition block, continue the loop
        auto jmp_to_cond = Half_Ir_Jump(cond_label);
        builder.AddExp(jmp_to_cond);

        // end block
        auto end_block = builder.NewBlock(end_label);
        builder.SetInsertPoint(end_block);
    }
    else if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&expr.expr))
    {
        auto& vec = **pvec;
        for (auto begin = vec.begin(); begin != vec.end() - 1; ++begin)
        {
            Trans_Expr(table, builder, *begin);
        }
        return Trans_Expr(table, builder, vec.back());
    }
    else if (auto ptype = std::get_if<std::shared_ptr<Half_TypeDecl>>(&expr.expr))
    {
        auto& type = **ptype;
    }
    else
    {
        _ASSERT(false);
    }

    return Half_Ir_Name();
}

