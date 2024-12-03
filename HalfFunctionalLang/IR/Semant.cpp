#include "Semant.h"
#include "Symbol.h"


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
    auto name = Half_Ir_Name(Temp::NewLabel(call.name));
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
    
    return Half_Ir_BinOp(op.op, l, r);
}

Half_Ir_Exp Trans_Assign(std::shared_ptr<Table>& table, Half_Assign& assign)
{
    auto l = Trans_Var(table, assign.left);
    auto r = Trans_Expr(table, assign.right);
    return Half_Ir_Move(l, r);
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

    _ASSERT(false);
    return Half_Ir_Const(0);
}
