#include "IrFormatPass.h"

void IR_Print_Pass::dump(std::vector<std::string>& out)
{
    out = lines;
}

void IR_Print_Pass::Run(Half_Ir_Exp& program)
{
    auto exp_ptr = &program.exp;
    if (auto palloc = std::get_if<std::shared_ptr<Half_Ir_Alloca>>(exp_ptr))
    {
        RunOn(**palloc);
    }
    else if (auto pload = std::get_if<std::shared_ptr<Half_Ir_Load>>(exp_ptr))
    {
        RunOn(**pload);
    }
    else if (auto pstore = std::get_if<std::shared_ptr<Half_Ir_Store>>(exp_ptr))
    {
        RunOn(**pstore);
    }
    else if (auto pext = std::get_if<std::shared_ptr<Half_Ir_Ext>>(exp_ptr))
    {
        RunOn(**pext);
    }
    else if (auto pconst = std::get_if<std::shared_ptr<Half_Ir_Const>>(exp_ptr))
    {
        RunOn(**pconst);
    }
    else if (auto pgetptr = std::get_if<std::shared_ptr<Half_Ir_GetElementPtr>>(exp_ptr))
    {
        RunOn(**pgetptr);
    }
    else if (auto pfetch = std::get_if<std::shared_ptr<Half_Ir_FetchPtr>>(exp_ptr))
    {
        RunOn(**pfetch);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Return>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(exp_ptr))
    {
        indent += 4;
        for (auto& alloc : (*pfunc)->alloc.exps)
        {
            Run(alloc);
        }
        for (auto& block : (*pfunc)->blocks)
        {
            RunOn(block.label);
            for (auto& exp : block.exps)
            {
                Run(exp);
            }
        }
        indent -= 4;
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Name>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Value>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Call>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_BinOp>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Branch>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Move>>(exp_ptr))
    {
        // RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Label>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Phi>>(exp_ptr))
    {
        RunOn(**ptr);
    }
    else if (auto ptr = std::get_if<std::shared_ptr<Half_Ir_Jump>>(exp_ptr))
    {
        RunOn(**ptr);
    }
}

void IR_Print_Pass::RunOn(Half_Ir_Alloca& alloc)
{
    std::string line = std::string(indent, ' ') + "alloca at ";
    line += alloc.out_address.base.l + " offset " + std::to_string(alloc.out_address.offset);
    line += " size " + std::to_string(alloc.out_address.type.GetSize());
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Load& load)
{
    std::string line = std::string(indent, ' ') + load.out_register.reg.l + " = load ";
    line += load.address.base.l + " offset " + std::to_string(load.address.offset);
    line += " size " + std::to_string(load.address.type.GetSize());
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Store& store)
{
    std::string line = std::string(indent, ' ') + "store ";
    line += store.value.GetLabel().l + " to ";
    line += store.address.base.l + " offset " + std::to_string(store.address.offset);
    line += " size " + std::to_string(store.address.type.GetSize());
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Ext& ext)
{
    std::string line = std::string(indent, ' ') + ext.out_label.l + " = ext ";
    line += ext.value.GetLabel().l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Const& c)
{
    std::string line = std::string(indent, ' ') + c.out_label.l + " = const ";
    line += std::to_string(c.n);
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_GetElementPtr& gep)
{
    std::string line = std::string(indent, ' ') + gep.out_label.l + " = getelementptr ";
    line += gep.base.l + "(" + std::to_string(gep.offset) + ")";
    for (auto& idx : gep.in_indexs)
    {
        if (auto pconst = std::get_if<Half_Ir_Const>(&idx))
        {
            auto& cons = *pconst;
            line += ", " + std::to_string(cons.n);
        }
        else
        {
            auto& val = std::get<Value>(idx);
            line += ", " + val.GetLabel().l;
        }
    }
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_FetchPtr& fetch)
{
    std::string line = std::string(indent, ' ') + fetch.out_label.l + " = fetch ";
    line += fetch.ptr.base.l + " offset " + std::to_string(fetch.ptr.offset);
    line += " size " + std::to_string(fetch.ptr.type.GetSize());
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Return& ret)
{
    std::string line = std::string(indent, ' ') + "return ";
    line += ret.value.GetLabel().l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Function& func)
{
    std::string line = std::string(indent, ' ') + "function " + func.name;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Name& name)
{
    std::string line = std::string(indent, ' ') + name.name.l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Value& value)
{
    std::string line = std::string(indent, ' ') + value.out_label.l + " = ";
    if (auto pconst = std::get_if<Half_Ir_Const>(&value.val))
    {
        line += "const " + std::to_string(pconst->n);
    }
    else if (auto pfloat = std::get_if<Half_Ir_Float>(&value.val))
    {
        line += "float " + std::to_string(pfloat->f);
    }
    else if (auto pstr = std::get_if<Half_Ir_String>(&value.val))
    {
        line += "string " + pstr->str;
    }
    else if (auto pname = std::get_if<Half_Ir_Name>(&value.val))
    {
        line += "name " + pname->name.l;
    }
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Call& call)
{
    std::string line = std::string(indent, ' ') + call.out_label.l + " = call ";
    line += call.fun_name.l + "(";
    for (auto& arg : call.args)
    {
        line += arg.name.l + ", ";
    }
    line.pop_back();
    line.pop_back();
    line += ")";
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_BinOp& binop)
{
    std::string line = std::string(indent, ' ') + binop.out_label.l + " = ";
    switch (binop.op)
    {
    case Half_Ir_BinOp::Oper::Plus:
        line += "add ";
        break;
    case Half_Ir_BinOp::Oper::Minus:
        line += "sub ";
        break;
    case Half_Ir_BinOp::Oper::Multy:
        line += "mul ";
        break;
    case Half_Ir_BinOp::Oper::Divide:
        line += "div ";
        break;
    case Half_Ir_BinOp::Oper::Mod:
        line += "mod ";
        break;
    case Half_Ir_BinOp::Oper::And:
        line += "and ";
        break;
    case Half_Ir_BinOp::Oper::Or:
        line += "or ";
        break;
    case Half_Ir_BinOp::Oper::Xor:
        line += "xor ";
        break;
    case Half_Ir_BinOp::Oper::LShift:
        line += "lshift ";
        break;
    case Half_Ir_BinOp::Oper::RShift:
        line += "rshift ";
        break;
    case Half_Ir_BinOp::Oper::Less:
        line += "less ";
        break;
    case Half_Ir_BinOp::Oper::LessEqual:
        line += "lessequal ";
        break;
    case Half_Ir_BinOp::Oper::Greater:
        line += "greater ";
        break;
    case Half_Ir_BinOp::Oper::GreaterEqual:
        line += "greaterequal ";
        break;
    case Half_Ir_BinOp::Oper::Equal:
        line += "equal ";
        break;
    case Half_Ir_BinOp::Oper::NotEqual:
        line += "notequal ";
        break;
    default:
        break;
    }
    line += binop.left.reg.l + ", " + binop.right.reg.l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Compare& compare)
{
    std::string line = std::string(indent, ' ') + compare.out_label.l + " = ";
    switch (compare.op)
    {
    case Half_Ir_Compare::Oper::Less:
        line += "less ";
        break;
    case Half_Ir_Compare::Oper::LessEqual:
        line += "lessequal ";
        break;
    case Half_Ir_Compare::Oper::Greater:
        line += "greater ";
        break;
    case Half_Ir_Compare::Oper::GreaterEqual:
        line += "greaterequal ";
        break;
    case Half_Ir_Compare::Oper::Equal:
        line += "equal ";
        break;
    case Half_Ir_Compare::Oper::NotEqual:
        line += "notequal ";
        break;
    default:
        break;
    }
    line += compare.left.name.l + ", " + compare.right.name.l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Branch& branch)
{
    std::string line = std::string(indent, ' ') + "branch ";
    line += branch.condition.out_label.l + ", ";
    line += branch.true_label.l + " : " + branch.false_label.l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Move& move)
{
    std::string line = std::string(indent, ' ') + "move ";
    // line += move.out_label.l + " = " + move.value.GetLabel().l;
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Temp::Label& label)
{
    lines.push_back(label.l);
}

void IR_Print_Pass::RunOn(Half_Ir_Label& label)
{
    lines.push_back(label.lab.l);
}

void IR_Print_Pass::RunOn(Half_Ir_Phi& phi)
{
    std::string line = std::string(indent, ' ') + phi.result.name.l + " = phi ";
    for (auto& [n, l] : phi.values)
    {
        line += "[" + n.name.l + " : " + l.lab.l + "], ";
    }
    line.pop_back();
    line.pop_back();
    lines.push_back(line);
}

void IR_Print_Pass::RunOn(Half_Ir_Jump& jump)
{
    std::string line = std::string(indent, ' ') + "jump ";
    line += jump.target.l;
    lines.push_back(line);
}
