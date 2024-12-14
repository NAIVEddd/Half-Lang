#include "Semant.h"
#include "Symbol.h"
#include "Builder.h"


auto IR_Make_Char =
[](char c) -> Half_IR
    {
        // TODO
        return Half_Ir_Const(c);
    };

auto IR_Make_Const =
[](int i) -> Half_IR
    {
        return Half_Ir_Const(i);
    };

auto IR_Make_Float =
[](float f) -> Half_IR
    {
        // TODO
        return Half_Ir_Const(f);
    };

auto IR_Make_String =
[](std::string s) -> Half_IR
    {
        // TODO
        return Half_Ir_Const(s.size());
    };

auto IR_Make_Value =
[](std::shared_ptr<Half_Value>& value) -> Half_IR
    {
        static auto overload = overloaded{ IR_Make_Char, IR_Make_Const, IR_Make_Float, IR_Make_String };
        _ASSERT(value);
        return std::visit(overload, value->value);
    };



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
    //return Half_Ir_Move(l, r);
    return l;
}

Half_Ir_Exp Trans_Deffunc(std::shared_ptr<Table>& table, Def_Func& func)
{
    FunctionSymbol funcSymb;
    std::vector<Half_Ir_Exp> allocs;
    auto funcscope = Table::begin_scope(table);

    for (size_t i = 0; i < func.parameters.size(); i++)
    {
        Symbol smb;
        smb.name = func.parameters[i].name;
        funcscope->insert(smb);
        funcscope->labels.insert({ smb.name, Temp::NewLabel(funcscope->labels.size()) });
    }

    funcSymb.name = func.name;
    funcSymb.label = Temp::NewLabel(func.name);
    funcSymb.scope = funcscope;
    funcSymb.body = Trans_Expr(funcscope, func.func_body);

    table->insert(funcSymb);
    auto funLabel = Half_Ir_Label(Temp::Label(func.name));
    return Half_Ir_Func(funLabel, funcSymb.body);
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

Half_Ir_Exp Trans_If(std::shared_ptr<Table>& table, Half_If& _if)
{
    auto c = Trans_Expr(table, _if.condition);
    auto t = Trans_Expr(table, _if.trueExpr);
    auto f = Trans_Expr(table, _if.falseExpr);
    return Half_Ir_Branch(c, t, f);
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
    else if (auto pdeffunc = std::get_if<std::shared_ptr<Def_Func>>(&expr.expr))
    {
        auto& deffunc = **pdeffunc;
        return Trans_Deffunc(table, deffunc);
    }
    else if (auto plet = std::get_if<std::shared_ptr<Half_Let>>(&expr.expr))
    {
        auto& let = **plet;
        return Trans_Let(table, let);
    }
    else if (auto pif = std::get_if<std::shared_ptr<Half_If>>(&expr.expr))
    {
        auto& _if = **pif;
        return Trans_If(table, _if);
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

        auto funcscope = Table::begin_scope(table);
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            Symbol smb;
            smb.name = func.parameters[i].var_name;
            funcscope->insert(smb);
            funcscope->labels.insert({ smb.name, Temp::NewLabel(funcscope->labels.size()) });
        }
        auto block_entry = Default_builder.NewBlock();
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
    //else if (auto pvalue = std::get_if<Half_Value>(&op))
    //{
    //    auto& value = *pvalue;
    //    return Trans_Value_Builder(table, value, builder);
    //}
    //else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    //{
    //    auto& funcall = *pfuncall;
    //    return Trans_Funcall_Builder(table, funcall, builder);
    //}
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

        auto alloc = Half_Ir_Alloc(s.offset, Temp::NewLabel());
        builder.AddExp(alloc);

        auto assign = Half_Expr(let.def);
        return Trans_Expr(table, builder, assign);
        /*auto assign = let.def;
        auto rval = Trans_Expr(table, builder, assign.right);
        auto symbol = table->find(assign.left.name());
        if (!symbol)
        {
            _ASSERT(false);
            return Half_Ir_Name("Varabile not defined:(" + assign.left.name() + ")");
        }
        auto store = Half_Ir_Store(symbol.value().offset, rval.name);
        builder.AddExp(store);
        return rval.name;*/
    }
    else if (auto pvar = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
    {
        auto& var = **pvar;
        return Trans_Var_Builder(table, var, builder);
    }
    else if (auto pvalue = std::get_if<std::shared_ptr<Half_Value>>(&expr.expr))
    {
        auto& value = **pvalue;

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
        auto current_block_size = builder.blocks.size();
        Half_Ir_Function func_ir;
        func_ir.name = func.name;
        func_ir.args = func.parameters;

        // 1. Create a new block for the function
        auto block_entry = builder.NewBlock();
        builder.SetInsertPoint(block_entry);
        //Half_Ir_Label func_label(Temp::Label(func.name));
        //Half_Ir_Exp func_label_exp(func_label);
        //builder.AddExp(block_entry, func_label_exp);

        // 2. Create a new scope for the function
        auto funcscope = Table::begin_scope(table);
        // 3. Add the function parameters to the scope(allocation stack to store the parameters)
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            Symbol smb;
            smb.name = func.parameters[i].var_name;
            funcscope->insert(smb);
            funcscope->labels.insert({ smb.name, Temp::NewLabel(funcscope->labels.size()) });
            auto alloc = Half_Ir_Alloc(smb.offset, Temp::NewLabel());
            builder.AddExp(alloc);
        }
        
        // 4. Translate the function body
        auto last_val = Trans_Expr(funcscope, builder, func.body);
        Half_Ir_Return ret(last_val);
        builder.AddExp(ret);

        // 5. Move all blocks in builder to function ir
        for (size_t i = block_entry; i < builder.blocks.size(); i++)
        {
            func_ir.blocks.push_back(builder.blocks[i]);
        }
        // 6. Remove the blocks from the builder
        builder.blocks.erase(builder.blocks.begin() + block_entry, builder.blocks.end());
        builder.insert_point = builder.blocks.size() - 1;
        builder.AddExp(func_ir);
        return Half_Ir_Name("FuncDecl, should't use this label " + func.name);
    }
    //else if (auto pif = std::get_if<std::shared_ptr<Half_If>>(&expr.expr))
    //{
    //    auto& _if = **pif;
    //}
    //else if (auto pwhile = std::get_if<std::shared_ptr<Half_While>>(&expr.expr))
    //{
    //    auto& _while = **pwhile;
    //}
    //else if (auto pfor = std::get_if<std::shared_ptr<Half_For>>(&expr.expr))
    //{
    //    auto& _for = **pfor;
    //}
    else if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&expr.expr))
    {
        auto& vec = **pvec;
        for (auto begin = vec.begin(); begin != vec.end() - 1; ++begin)
        {
            Trans_Expr(table, builder, *begin);
        }
        return Trans_Expr(table, builder, vec.back());
        /*Half_Ir_Name name;
        for (auto& v : vec)
        {
            name = Trans_Expr(table, builder, v);
        }
        return name;*/
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

