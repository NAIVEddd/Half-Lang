#include "Semant.h"
#include "Symbol.h"
#include "Builder.h"
#include <algorithm>
#include <variant>


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

size_t Access_Var_Offset(std::shared_ptr<Table>& table, Half_Var& var, Half_Type_Info& type)
{
    if (auto psimple = std::get_if<Half_Var::SimpleVar>(&var.var))
    {
        return 0;
    }
    else if (auto pfield = std::get_if<Half_Var::FieldVar>(&var.var))
    {
        auto& field = *pfield;
        auto pstruct_ty = std::get_if<Half_Type_Info::StructType>(&type.type);
        _ASSERT(pstruct_ty);
        auto base_offset = Access_Var_Offset(table, *field.var, type);
        auto& struct_ty = *pstruct_ty;
        auto& struct_field = struct_ty.GetField(field.id);
        auto field_offset = struct_field.offset;
        return base_offset + field_offset;
    }
    else if (auto psub = std::get_if<Half_Var::SubscriptVar>(&var.var))
    {
        auto& sub = *psub;
        auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&type.type);
        _ASSERT(parray_ty);
        auto base_offset = Access_Var_Offset(table, *sub.var, type);
        auto one_elem_size = parray_ty->type->GetSize();
        //auto offset = one_elem_size * sub.index;
        return 0;
    }

    _ASSERT(false);
    return 0;
}

void Init_Basic_Type(std::shared_ptr<Table>& table)
{
    table->insert("char", std::make_shared<Half_Type_Info>(Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Char)));
    table->insert("int", std::make_shared<Half_Type_Info>(Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Int)));
    table->insert("float", std::make_shared<Half_Type_Info>(Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Float)));
    table->insert("string", std::make_shared<Half_Type_Info>(Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::String)));
}

std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl& type)
{
    if (auto prename = std::get_if<Half_TypeDecl::RenameType>(&type.type))
    {
        auto target = Trans_Type(table, *prename->type);
        table->insert(prename->name, target);
        return table->findType(prename->name).value();
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::BasicType>(&type.type))
    {
        // do nothing
        auto opt_ty = table->findType(ptype->type_name);
        return opt_ty.value_or(nullptr);
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::name_ref_t>(&type.type))
    {
        auto opt_ty = table->findType(*ptype);
        return opt_ty.value_or(nullptr);
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::Ptr>(&type.type))
    {
        // do nothing
        auto opt_ty = table->findType(ptype->target);
        if (!opt_ty)
        {
            printf("Type not found: %s\n", ptype->target.c_str());
            _ASSERT(false);
        }
        auto ptr = Half_Type_Info::PointerType(opt_ty.value());
        return std::make_shared<Half_Type_Info>(ptr);
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::TupleType>(&type.type))
    {
        for (auto& t : ptype->type_list)
        {
        }
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::IncompleteArrayType>(&type.type))
    {
        auto opt_ty = table->findType(ptype->type_name);
        if (!opt_ty)
        {
            printf("Type not found: %s\n", ptype->type_name.c_str());
            _ASSERT(false);
        }
        auto arr = Half_Type_Info::ArrayType(opt_ty.value(), 0);
        return std::make_shared<Half_Type_Info>(arr);
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::ArrayType>(&type.type))
    {
        auto opt_ty = table->findType(ptype->type_name);
        if (!opt_ty)
        {
            printf("Type not found: %s\n", ptype->type_name.c_str());
            _ASSERT(false);
        }
        auto arr = Half_Type_Info::ArrayType(opt_ty.value(), ptype->count);
        return std::make_shared<Half_Type_Info>(arr);
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::StructType>(&type.type))
    {
        std::vector<Half_Type_Info::StructType::TypePair> fields;
        size_t offset = 0;
        for (auto& f : ptype->field_list)
        {
            auto opt_ty = table->findType(f.type);
            if (!opt_ty)
            {
                printf("Type not found: %s\n", f.type.c_str());
                _ASSERT(false);
            }
            auto ty = opt_ty.value();
            auto field = Half_Type_Info::StructType::TypePair(f.name, opt_ty.value(), offset);
            fields.push_back(field);

            // TODO: align
            //    eg. int a, char b, int c, have to align char b to 4 bytes
            offset += ty->GetSize();
        }
        auto str = Half_Type_Info::StructType(ptype->name, fields);
        auto pty = std::make_shared<Half_Type_Info>(str);
        table->insert(ptype->name, pty);
        return pty;
    }
    else if (auto ptype = std::get_if<Half_TypeDecl::FuncType>(&type.type))
    {
        std::vector<std::shared_ptr<Half_Type_Info>> args;
        for (auto& a : ptype->parameter_types)
        {
            auto opt_ty = table->findType(a);
            if (!opt_ty)
            {
                printf("Type not found: %s\n", a.c_str());
                _ASSERT(false);
            }
            args.push_back(opt_ty.value());
        }
        auto ret = table->findType(ptype->return_type);
        if (!ret)
        {
            printf("Type not found: %s\n", ptype->return_type.c_str());
            _ASSERT(false);
        }
        auto func = Half_Type_Info::FuncType(ret.value(), args);
        return std::make_shared<Half_Type_Info>(func);
    }
    return nullptr;
}

Half_Ir_GetElementPtr Trans_LeftVar_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder)
{
    auto symbol = table->find(var.name());
    if (!symbol)
    {
        _ASSERT(false);
    }
    if (auto pfield = std::get_if<Half_Var::FieldVar>(&var.var))
    {
        auto& field = *pfield;
        auto offset = symbol.value().offset;
        auto pstruct_ty = std::get_if<Half_Type_Info::StructType>(&symbol.value().type.type);
        _ASSERT(pstruct_ty);
        auto base_offset = Access_Var_Offset(table, *field.var, symbol->type);
        auto& struct_ty = *pstruct_ty;
        auto& struct_field = struct_ty.GetField(field.id);
        auto field_offset = struct_field.offset;
        Half_Ir_GetElementPtr gep(field_offset + base_offset);
        //builder.AddExp(gep);
        return gep;
    }
    else if (auto parray = std::get_if<Half_Var::SubscriptVar>(&var.var))
    {
        auto& array = *parray;
        auto base_offset = Access_Var_Offset(table, var, symbol->type);
        Builder new_builder;
        auto idx = Trans_Expr(table, new_builder, *array.index);
        auto sz = 0;
        if (auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&symbol.value().type.type))
        {
            _ASSERT(parray_ty);
            sz = parray_ty->type->GetSize();
        }
        _ASSERT(new_builder.blocks[0].exps.size() == 1);
        Half_Ir_GetElementPtr gep(base_offset);
        gep.elem_sizes.push_back(sz);
        gep.in_index.push_back(new_builder.blocks[0].exps[0]);
        gep.exp_out_labels.push_back(idx.name);
        return gep;
    }
    auto base_offset = symbol.value().offset;
    Half_Ir_GetElementPtr gep(base_offset);
    return gep;
}

Half_Ir_Name Trans_Var_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder)
{
    auto symbol = table->find(var.name());
    if (!symbol)
    {
        _ASSERT(false);
        return Half_Ir_Name("Varabile not defined:(" + var.name() + ")");
    }
    if (auto parray = std::get_if<Half_Var::SubscriptVar>(&var.var))
    {
        auto& array = *parray;
        auto offset = symbol.value().offset;
        auto i = Trans_Expr(table, builder, *array.index);
        
        auto sz = 0;
        if (auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&symbol.value().type.type))
        {
            _ASSERT(parray_ty);
            sz = parray_ty->type->GetSize();
        }
        Half_Ir_ArrayElemLoad load(offset, i.name, sz, Temp::NewLabel());
        builder.AddExp(load);
        return load.out_label;
    }
    auto offset = Access_Var_Offset(table, var, symbol->type);
    auto load = Half_Ir_Load(symbol->offset + offset, Temp::NewLabel());
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
    Init_Basic_Type(table);
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

        // 1. temp solution for type(only support struct type)
        // TODO: support other type
        if (auto pstructinit = std::get_if<std::shared_ptr<Half_StructInit>>(&let.def.right.expr))
        {
            auto& structinit = **pstructinit;
            auto pty = table->findType(structinit.type_name);
            if (!pty)
            {
                printf("Type not found: %s\n", structinit.type_name.c_str());
                _ASSERT(false);
            }
            auto& p = pty.value();
            s.type = *p;
        }
        else if (auto parrayinit = std::get_if<std::shared_ptr<Half_ArrayInit>>(&let.def.right.expr))
        {
            auto& arrayinit = **parrayinit;
            auto pty = table->findType(arrayinit.type_name);
            if (!pty)
            {
                printf("Type not found: %s\n", arrayinit.type_name.c_str());
                _ASSERT(false);
            }
            auto& p = pty.value();
            s.type = *p;
        }
        else
        {
            // for now, default type is int
            s.type = *table->findType("int").value();
        }
        

        table->insert(s);

        _ASSERT(builder.block_alloc_entry != -1);
        // alloc space, one time 4 bytes
        for (size_t i = 0; i < s.type.GetSize() / 4; i++)
        {
            auto alloc = Half_Ir_Alloc(s.offset + i * 4, Temp::NewLabel());
            builder.AddExp(builder.block_alloc_entry, alloc);
        }
        /*auto alloc = Half_Ir_Alloc(s.offset, Temp::NewLabel());
        builder.AddExp(builder.block_alloc_entry, alloc);*/

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

        if (auto pstructinit = std::get_if<std::shared_ptr<Half_StructInit>>(&assign.right.expr))
        {
            auto& structinit = **pstructinit;
            auto pty = table->findType(structinit.type_name);
            if (!pty)
            {
                printf("Type not found: %s\n", structinit.type_name.c_str());
                _ASSERT(false);
            }

            auto symbol = table->find(assign.left.name());
            if (!symbol)
            {
                _ASSERT(false);
                return Half_Ir_Name("Varabile not defined:(" + assign.left.name() + ")");
            }
            /*auto left_ptr = Trans_LeftVar_Builder(table, assign.left, builder);
            auto base_offset = left_ptr.GetOffset();
            if (base_offset == -1)
            {
                builder.AddExp(left_ptr);
            }*/
            if (auto pstruct = std::get_if<Half_Type_Info::StructType>(&pty.value()->type))
            {
                auto& struct_ty = *pstruct;
                auto left = Trans_Var_Builder(table, assign.left, builder);
                for (size_t i = 0; i < structinit.fields.size(); i++)
                {
                    auto& init_field = structinit.fields[i];
                    auto& ty_field = init_field.name.empty()? struct_ty.field_list[i] : struct_ty.GetField(init_field.name);
                    // TODO: recursive call to translate the field
                    // for now, only support simple type(first level of struct must be simple type)

                    Half_Assign init;
                    auto t_v = std::make_shared<Half_Var>(assign.left);
                    init.left = Half_Var::FieldVar(t_v, ty_field.name);
                    init.right = init_field.value;
                    auto init_exp = Half_Expr(init);
                    Trans_Expr(table, builder, init_exp);
                }
                return Half_Ir_Name("StructInit, should't use this label " + structinit.type_name);
            }
            else
            {
                printf("Type not supported: %s\n", structinit.type_name.c_str());
                _ASSERT(false);
            }
        }
        else if (auto parrayinit = std::get_if<std::shared_ptr<Half_ArrayInit>>(&assign.right.expr))
        {
            auto& arrayinit = **parrayinit;
            auto pty = table->findType(arrayinit.type_name);
            if (!pty)
            {
                printf("Type not found: %s\n", arrayinit.type_name.c_str());
                _ASSERT(false);
            }
            auto symbol = table->find(assign.left.name());
            if (!symbol)
            {
                _ASSERT(false);
                return Half_Ir_Name("Varabile not defined:(" + assign.left.name() + ")");
            }
            if (auto parray = std::get_if<Half_Type_Info::ArrayType>(&pty.value()->type))
            {
                auto& array_ty = *parray;
                auto array_offset = symbol.value().offset;
                auto elem_size = array_ty.type->GetSize();

                // TODO: use left var builder to get the array address
                for (size_t i = 0; i < arrayinit.values.size(); i++)
                {
                    auto ci = Half_Ir_Const(i);

                    Half_Assign init;
                    Half_Var l_var;
                    auto t_v = std::make_shared<Half_Var>(assign.left);
                    auto idx = std::make_shared<Half_Expr>(Half_Value((int)i));
                    l_var = Half_Var::SubscriptVar(t_v, idx);
                    init.left = l_var;
                    init.right = arrayinit.values[i];
                    auto init_exp = Half_Expr(init);
                    Trans_Expr(table, builder, init_exp);
                }
                return Half_Ir_Name("ArrayInit, should't use this label " + arrayinit.type_name);
            }
            else
            {
                printf("Type not supported: %s\n", arrayinit.type_name.c_str());
                _ASSERT(false);
            }
        }
        // simple type : int, char ... (size less than 4 or 8 bytes)
        auto rval = Trans_Expr(table, builder, assign.right);
        auto symbol = table->find(assign.left.name());
        if (!symbol)
        {
            _ASSERT(false);
            return Half_Ir_Name("Varabile not defined:(" + assign.left.name() + ")");
        }

        // left value is a simple type, store the value directly
        // if left value is pointer, get the address of right value and store it to the left value

        auto left_ptr = Trans_LeftVar_Builder(table, assign.left, builder);
        auto base_offset = left_ptr.GetOffset();
        if (base_offset != -1)
        {
            auto store = Half_Ir_Store(base_offset, rval.name);
            builder.AddExp(store);
        }
        else
        {
            builder.AddExp(left_ptr);
            auto store = Half_Ir_ElemStore(0, symbol.value().type.GetSize(), left_ptr.out_label, rval.name);
            builder.AddExp(store);
        }
        
        return rval.name;
    }
    else if (auto pfuncall = std::get_if<std::shared_ptr<Half_Funcall>>(&expr.expr))
    {
        auto& funcall = **pfuncall;

        std::vector<Half_Ir_Name> arg_exp(funcall.args.size());
        for (size_t i = 0; i < funcall.args.size(); i++)
        {
            auto arg = Trans_Expr(table, builder, funcall.args[i]);
            arg_exp[i] = arg.name;
        }
        Half_Ir_Call call(Temp::NewLabel(), Temp::Label(funcall.name), arg_exp);
        builder.AddExp(call);
        return call.out_label;
    }
    else if (auto pfunc = std::get_if<std::shared_ptr<Half_FuncDecl>>(&expr.expr))
    {
        auto& func = **pfunc;

        Stack stack(table);

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
            smb.type = *(table->findType(func.parameters[i].type_name).value());
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

        // 7. Add the function to the table
        std::vector<std::string> parameter_types(func.parameters.size());
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            parameter_types[i] = func.parameters[i].type_name;
        }
        auto func_type_decl = Half_TypeDecl(Half_TypeDecl::FuncType(func.return_type, parameter_types));
        auto pfunc_ty = Trans_Type(table, func_type_decl);
        auto& func_ty = std::get<Half_Type_Info::FuncType>(pfunc_ty->type);
        table->insert(FunctionSymbol(func.name, func_ty));

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
        Half_Var termi_var(_for.var.name() + "_@for_termi");
        auto for_termi = Half_Let(Half_Assign(termi_var, _for.end));
        auto termi_expr = Half_Expr(for_termi);
        Trans_Expr(for_table, builder, termi_expr);

        // jump to condition block, if current block is not empty
        if (builder.blocks[builder.insert_point].exps.size() > 0)
        {
            auto jmp_to_cond = Half_Ir_Jump(cond_label);
            builder.AddExp(jmp_to_cond);
        }

        // condition block
        auto cond_block = builder.NewBlock(cond_label);
        builder.SetInsertPoint(cond_block);
        auto cond = Half_Op(_for.isup? "<" : ">", Half_Op::Half_OpExpr(_for.var), Half_Op::Half_OpExpr(termi_var));
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
        Trans_Type(table, type);
        return Half_Ir_Name();
    }
    else
    {
        _ASSERT(false);
    }

    return Half_Ir_Name();
}

