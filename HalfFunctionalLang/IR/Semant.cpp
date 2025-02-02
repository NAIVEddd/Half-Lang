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

Half_Type_Info Get_Expr_Type(std::shared_ptr<Table>& table, Half_Expr& expr)
{
    if (auto pval = std::get_if<std::shared_ptr<Half_Value>>(&expr.expr))
    {
        auto& val = **pval;
        if (auto pconst = std::get_if<char>(&val.value))
        {
            return Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Char);
        }
        else if (auto pfloat = std::get_if<float>(&val.value))
        {
            return Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Float);
        }
        else if (auto pstr = std::get_if<std::string>(&val.value))
        {
            return Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::String);
        }
        else if (auto pint = std::get_if<int>(&val.value))
        {
            return Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Int);
        }
        _ASSERT(false);
        return Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Int);
    }
    else if (auto pvar = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
    {
        auto& var = **pvar;
        auto opt_sym = table->find(var.name());
        if (!opt_sym)
        {
            printf("Symbol not found: %s\n", var.name().c_str());
            // TODO: error message and interrupt the compilation
            _ASSERT(false);
        }
        if (auto psimple = std::get_if<Half_Var::SimpleVar>(&var.var))
        {
            return opt_sym.value().type;
        }
        else if (auto pfield = std::get_if<Half_Var::FieldVar>(&var.var))
        {
            auto& field = *pfield;
            auto& sym = opt_sym.value();
            auto& type = sym.type;
            if (auto pstruct_ty = std::get_if<Half_Type_Info::StructType>(&type.type))
            {
                auto& struct_ty = *pstruct_ty;
                auto& struct_field = struct_ty.GetField(field.id);
                return *struct_field.type;
            }
            _ASSERT(false);
            return {};
        }
        if (auto psubscript = std::get_if<Half_Var::SubscriptVar>(&var.var))
        {
            auto& subfield = *psubscript;
            auto& sym = opt_sym.value();
            auto& type = sym.type;
            if (auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&type.type))
            {
                return *parray_ty->type;
            }
            else if (auto parray_ty = std::get_if<Half_Type_Info::PointerType>(&type.type))
            {
                auto& ty = *parray_ty->type;
                return ty.type;
            }
            // TODO: not array type, error message
            _ASSERT(false);
            return {};
        }
        _ASSERT(false);
        return {};
    }
    else if (auto pfuncall = std::get_if<std::shared_ptr<Half_Funcall>>(&expr.expr))
    {
        return Half_Type_Info::BasicType::BasicT::Int;
    }
    return Half_Type_Info::BasicType::BasicT::Int;
    //_ASSERT(false);
    //return Half_Type_Info();
}

//std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl& type);
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, std::monostate& type)
{
    _ASSERT(false);
    return nullptr;
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::name_ref_t& type)
{
    //_ASSERT(false);
    auto pty = table->findType(type).value_or(nullptr);
    if (!pty)
    {
        printf("Type not found: %s\n", type.c_str());
        _ASSERT(false);
    }
    return pty;
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::Nil& type)
{
    return std::make_shared<Half_Type_Info>(Half_Type_Info::BasicType(Half_Type_Info::BasicType::BasicT::Int));
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::Additional& type)
{
    return nullptr;
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::Ptr& type)
{
    auto opt_ty = table->findType(type.target);
    if (!opt_ty)
    {
        printf("Type not found: %s\n", type.target.c_str());
        _ASSERT(false);
    }
    auto ptr = Half_Type_Info::PointerType(opt_ty.value());
    return std::make_shared<Half_Type_Info>(ptr);
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::BasicType& type)
{
    auto opt_ty = table->findType(type.type_name);
    return opt_ty.value_or(nullptr);
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::TupleType& type)
{
    _ASSERT(false);
    return nullptr;
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::RenameType& type)
{
    auto target = Trans_Type(table, *type.type);
    table->insert(type.name, target);
    return table->findType(type.name).value();
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::IncompleteArrayType& type)
{
    auto opt_ty = table->findType(type.type_name);
    if (!opt_ty)
    {
        printf("Type not found: %s\n", type.type_name.c_str());
        _ASSERT(false);
    }
    auto arr = Half_Type_Info::PointerType(opt_ty.value());
    return std::make_shared<Half_Type_Info>(arr);
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::CompleteArrayType& type)
{
    auto opt_ty = table->findType(type.incomplete_array_name);
    if (!opt_ty)
    {
        printf("Type not found: %s\n", type.incomplete_array_name.c_str());
        _ASSERT(false);
    }
    auto arr = Half_Type_Info::ArrayType(opt_ty.value(), type.count);
    return std::make_shared<Half_Type_Info>(arr);
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::ArrayType& type)
{
    auto opt_ty = table->findType(type.type_name);
    if (!opt_ty)
    {
        printf("Type not found: %s\n", type.type_name.c_str());
        _ASSERT(false);
    }
    auto arr = Half_Type_Info::ArrayType(opt_ty.value(), type.count);
    return std::make_shared<Half_Type_Info>(arr);
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::StructType& type)
{
    std::vector<Half_Type_Info::StructType::TypePair> fields;
    size_t offset = 0;
    for (auto& f : type.field_list)
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
    auto str = Half_Type_Info::StructType(type.name, fields);
    auto pty = std::make_shared<Half_Type_Info>(str);
    table->insert(type.name, pty);
    return pty;
}
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl::FuncType& type)
{
    std::vector<std::shared_ptr<Half_Type_Info>> args;
    for (auto& a : type.parameter_types)
    {
        auto opt_ty = table->findType(a);
        if (!opt_ty)
        {
            printf("Type not found: %s\n", a.c_str());
            _ASSERT(false);
        }
        args.push_back(opt_ty.value());
    }
    auto ret = table->findType(type.return_type);
    if (!ret)
    {
        printf("Type not found: %s\n", type.return_type.c_str());
        _ASSERT(false);
    }
    auto func = Half_Type_Info::FuncType(ret.value(), args);
    return std::make_shared<Half_Type_Info>(func);
}


std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl& type)
{
    return std::visit([&](auto&& arg)
        {
            return Trans_Type(table, arg);
        }, type.type);
}

Address Trans_GetElementPtr(std::shared_ptr<Table>& table, Half_Ir_GetElementPtr& ptr, Builder& builder)
{
    Half_Type_Info type = ptr.source_element_type;
    for (size_t i = 0; i < ptr.in_indexs.size(); i++)
    {
        auto& idx = ptr.in_indexs[i];

        // check if it is a pointer
        if (auto pptr_ty = std::get_if<Half_Type_Info::PointerType>(&type.type))
        {
            auto elem_type = pptr_ty->type;
            if (auto pconst = std::get_if<Half_Ir_Const>(&idx))
            {
                auto n = pconst->n;
                auto sz = elem_type->GetSize();
                ptr.offset += n * sz;
            }
            else if (auto pvar = std::get_if<Value>(&idx))
            {
                /*auto sz = (int)elem_type->GetSize();
                Half_Ir_Const ty_sz(sz);
                auto ty_sz_res = ty_sz.GetResult();
                Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Multy, std::get<Register>(ty_sz_res.value), std::get<Register>(pvar->value), Temp::NewLabel());
                auto binop_res = binop.GetResult();
                Half_Ir_Ext ext(std::get<Register>(binop_res.value), binop_res.GetType(), Temp::NewLabel());
                Half_Ir_Load fetch(Address{ elem_type, ptr.base, ptr.offset });
                auto ext_res = ext.GetResult();
                auto fetch_res = fetch.GetResult();
                Half_Ir_BinOp binop2(Half_Ir_BinOp::Oper::Plus, std::get<Register>(ext_res.value), std::get<Register>(fetch_res.value), Temp::NewLabel());
                builder.AddExp(ty_sz);
                builder.AddExp(binop);
                builder.AddExp(ext);
                builder.AddExp(fetch);
                builder.AddExp(binop2);
                ptr.base = binop2.GetResult().GetLabel();
                ptr.offset = 0;*/
            }

            auto& ptr_ty = *pptr_ty;
            type = *ptr_ty.type;
        }
        else if (auto pstruct_ty = std::get_if<Half_Type_Info::StructType>(&type.type))
        {
            auto& struct_ty = *pstruct_ty;
            auto index = std::get<Half_Ir_Const>(idx);
            auto field = struct_ty.GetField(index.n);
            auto offset = field.offset;
            ptr.offset += offset;

            type = *field.type;
        }
        else if (auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&type.type))
        {
            _ASSERT(std::holds_alternative<Value>(idx));
            auto sz = (int)parray_ty->type->GetSize();
            if (auto pvar = std::get_if<Value>(&idx))
            {
                //Half_Ir_Const ty_sz(sz);
                //auto ty_sz_res = ty_sz.GetResult();
                //Half_Ir_BinOp binop(Half_Ir_BinOp::Oper::Multy, std::get<Register>(ty_sz_res.value), std::get<Register>(pvar->value), Temp::NewLabel());
                //Half_Ir_FetchPtr fetch(Address{ parray_ty->type, ptr.base, ptr.offset });
                //auto binop_res = binop.GetResult();
                //Half_Ir_Ext ext(std::get<Register>(binop_res.value), binop_res.GetType(), Temp::NewLabel());
                //auto ext_res = ext.GetResult();
                //auto fetch_res = fetch.GetResult();
                //Half_Ir_BinOp binop2(Half_Ir_BinOp::Oper::Plus, std::get<Register>(ext_res.value), std::get<Register>(fetch_res.value), Temp::NewLabel());
                ////Half_Ir_BinOp binop2(Half_Ir_BinOp::Oper::Plus, binop.GetResult().GetLabel(), fetch.GetResult().GetLabel(), Temp::NewLabel());
                //builder.AddExp(ty_sz);
                //builder.AddExp(binop);
                //builder.AddExp(ext);
                //builder.AddExp(fetch);
                //builder.AddExp(binop2);
                //ptr.base = binop2.GetResult().GetLabel();
                //ptr.offset = 0;
            }

            auto& array_ty = *parray_ty;
            type = *array_ty.type;
        }
    }
    //return Address{type, ptr.base, ptr.offset};
    return Address{};
}

Half_Ir_GetElementPtr Trans_LeftVar_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder)
{
    if (auto psimple = std::get_if<Half_Var::SimpleVar>(&var.var))
    {
        auto symbol = table->find(var.name());
        auto s_ty = symbol.value().type.to_string();
        auto addr_ty = symbol.value().addr.type.to_string();
        if (!symbol)
        {
            printf("Symbol not found: %s\n", var.name().c_str());
            _ASSERT(false);
        }
        if (auto pptr = std::get_if<Half_Type_Info::PointerType>(&symbol.value().type.type))
        {
            //Half_Ir_FetchPtr fetch(symbol.value().addr);
            //builder.AddExp(fetch);
            //RealAddress addr = { fetch.GetResult().GetType(), fetch.GetResult().GetLabel(), 0 };;
            //Address address(*pptr->type, std::make_shared<RealAddress>(addr), Temp::NewLabel());
            RealAddress addr = { symbol.value().addr.type, symbol.value().addr.real_address->base, 0 };
            Address address(symbol.value().type.type, std::make_shared<RealAddress>(addr), Temp::NewLabel());
            Half_Ir_GetElementPtr gep(address);
            gep.result_element_type = *pptr->type;
            return gep;
        }
        if (auto parr = std::get_if<Half_Type_Info::ArrayType>(&symbol.value().type.type))
        {
            //Half_Ir_FetchPtr fetch(symbol.value().addr);
            //builder.AddExp(fetch);
            //RealAddress addr = { fetch.GetResult().GetType(), fetch.GetResult().GetLabel(), 0 };;
            //Address address(*parr->type, std::make_shared<RealAddress>(addr), Temp::NewLabel());
            RealAddress addr = { symbol.value().addr.type, symbol.value().addr.real_address->base, 0 };
            Address address(symbol.value().type.type, std::make_shared<RealAddress>(addr), Temp::NewLabel());
            Half_Ir_GetElementPtr gep(address);
            gep.result_element_type = *parr->type;
            return gep;
        }
        Address address = symbol.value().addr;
        address.real_address = std::make_shared<RealAddress>(*address.real_address);
        Half_Ir_GetElementPtr gep(address);
        gep.in_indexs.push_back(Half_Ir_Const(0));
        gep.result_element_type = symbol.value().type;
        return gep;
    }
    else if (auto pfield = std::get_if<Half_Var::FieldVar>(&var.var))
    {
        auto& field = *pfield;
        Half_Ir_GetElementPtr get_ptr = Trans_LeftVar_Builder(table, *field.var, builder);
        auto symbol = table->find(var.name());
        if (auto pstruct_ty = std::get_if<Half_Type_Info::StructType>(&symbol.value().type.type))
        {
            auto& struct_ty = *pstruct_ty;
            get_ptr.in_indexs.push_back(Half_Ir_Const((int)struct_ty.GetFieldIndex(field.id)));
            get_ptr.result_element_type = struct_ty.GetField(field.id).type;
            return get_ptr;
        }
        get_ptr.result_element_type = Half_Type_Info::BasicType::BasicT::Invalid;
        return get_ptr;
    }
    else if (auto parray = std::get_if<Half_Var::SubscriptVar>(&var.var))
    {
        auto& array = *parray;
        Half_Ir_GetElementPtr get_ptr = Trans_LeftVar_Builder(table, *array.var, builder);
        auto idx = Trans_Expr(table, builder, *array.index);
        get_ptr.in_indexs.push_back(idx);

        auto symbol = table->find(var.name());
        if (auto parray_ty = std::get_if<Half_Type_Info::ArrayType>(&symbol.value().type.type))
        {
            get_ptr.result_element_type = *parray_ty->type;
            get_ptr.out_address.type = *parray_ty->type;
            return get_ptr;
        }
        else if (auto pptr = std::get_if<Half_Type_Info::PointerType>(&symbol.value().type.type))
        {
            get_ptr.result_element_type = *pptr->type;
            get_ptr.out_address.type = *pptr->type;
            return get_ptr;
        }
        get_ptr.result_element_type = Half_Type_Info::BasicType::BasicT::Invalid;
        return get_ptr;
    }
    
    _ASSERT(false);
    return Half_Ir_GetElementPtr(Address{});
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Var& var);
Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Value& value);
Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Funcall& funcall);
Value Trans_Op_Builder(std::shared_ptr<Table>& table, Half_Op& op, Builder& builder);
Value Trans_OpExpr_Builder(std::shared_ptr<Table>& table, Half_Op::Half_OpExpr& op, Builder& builder)
{
    if (auto pvar = std::get_if<Half_Var>(&op))
    {
        auto& var = *pvar;
        auto addr = Trans_Expr(table, builder, var);
        return addr;
    }
    else if (auto pvalue = std::get_if<Half_Value>(&op))
    {
        auto& value = *pvalue;
        auto res = Trans_Expr(table, builder, value);
        return res;
    }
    else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    {
        auto& funcall = *pfuncall;
        auto res = Trans_Expr(table, builder, funcall);
        return res;
    }
    else if (auto pop = std::get_if<Half_Op>(&op))
    {
        auto& binop = *pop;
        return Trans_Op_Builder(table, binop, builder);
    }

    _ASSERT(false);
    return Register{};
}

Value Trans_Op_Builder(std::shared_ptr<Table>& table, Half_Op& op, Builder& builder)
{
    auto l = Trans_OpExpr_Builder(table, *op.left, builder);
    auto r = Trans_OpExpr_Builder(table, *op.right, builder);

    auto op_ir = Half_Ir_BinOp(op.op, std::get<Register>(l.value), std::get<Register>(r.value), Temp::NewLabel());
    builder.AddExp(op_ir);

    Half_Type_Info type = Half_Type_Info::BasicType::BasicT::Int;


    auto res = op_ir.GetResult();
    if (auto preg = std::get_if<Register>(&res.value))
    {
        preg->type = type;
    }
    else if (auto paddr = std::get_if<Address>(&res.value))
    {
        // impossible
        _ASSERT(false);
        paddr->type = type;
    }
    return res;
}

Half_Ir_Name Trans_CondOp(std::shared_ptr<Table>& table, Builder& builder, Half_Op::Half_OpExpr& op, std::string prefix, Temp::Label true_label, Temp::Label false_label)
{
    if (auto pvar = std::get_if<Half_Var>(&op))
    {
        auto& var = *pvar;
        auto tmp = Half_Expr(var);
        auto res = Trans_Expr(table, builder, tmp);
        return res.GetLabel();
    }
    else if (auto pvalue = std::get_if<Half_Value>(&op))
    {
        auto& value = *pvalue;
        auto tmp = Half_Expr(value);
        auto res = Trans_Expr(table, builder, tmp);
        return res.GetLabel();
    }
    else if (auto pfuncall = std::get_if<Half_Funcall>(&op))
    {
        auto& funcall = *pfuncall;
        auto tmp = Half_Expr(funcall);
        auto res = Trans_Expr(table, builder, tmp);
        return res.GetLabel();
    }
    else if (auto pop = std::get_if<Half_Op>(&op))
    {
        auto& binop = *pop;

        Temp::Label new_block_label;
        Temp::Label left_true_label = true_label;
        Temp::Label left_false_label = false_label;
        if (binop.op == "&&")
        {
            new_block_label = builder.GenBlockLabel(prefix + "_right");
            left_true_label = new_block_label;
            left_false_label = false_label;
        }
        else if (binop.op == "||")
        {
            new_block_label = builder.GenBlockLabel(prefix + "_right");
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
            auto br = Half_Ir_Branch(cmp, true_label, false_label);
            builder.AddExp(br);
            return cmp.out_label;
        }
        
        return r;
    }

    // Todo: support other type
    _ASSERT(false);
    return Half_Ir_Name();
}

void Trans_If_Cond(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& cond, std::string prefix, Temp::Label true_label, Temp::Label false_label)
{
    //  now it's support multiple conditions like (a < b && c > d)
    if (auto pop = std::get_if<std::shared_ptr<Half_Op>>(&cond.expr))
    {
        auto& op = **pop;
        Half_Op::Half_OpExpr temp_op = op;
        Trans_CondOp(table, builder, temp_op, prefix, true_label, false_label);
        return;
    }
    printf("only support binary op exp type in if.cond\n");
    _ASSERT(false);
}

Value Trans_Expr(Half_Expr& expr, Builder& builder)
{
    auto table = std::make_shared<Table>();
    Init_Basic_Type(table);
    auto value = Trans_Expr(table, builder, expr);

    // move strings in table to builder
    for (auto& [l,s] : table->strings)
    {
        builder.InsertString(l, s);
    }

    return value;
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Let& let)
{
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
        // TODO: support other type
        s.type = Get_Expr_Type(table, let.def.right);
        //s.type = *table->findType("int").value();
    }
    

    table->insert(s);

    _ASSERT(builder.block_alloc_entry != -1);
    // alloc space, one time 4 bytes
    for (size_t i = 0; i < s.type.GetSize() / 4; i++)
    {
        Address addr;
        addr.real_address->offset = s.offset + i * 4;
        addr.real_address->base = Temp::Label("bottom");
        addr.type = Half_Type_Info::BasicType::BasicT::Int;
        auto alloc = Half_Ir_Alloca(addr);
        builder.AddExp(builder.block_alloc_entry, alloc);
    }

    auto assign = Half_Expr(let.def);
    return Trans_Expr(table, builder, assign);
}

Address Trans_Left_Var(std::shared_ptr<Table>& table, Builder& builder, Half_Var& var)
{
    if (auto psimple = std::get_if<Half_Var::SimpleVar>(&var.var))
    {
        auto symbol = table->find(var.name());
        if (!symbol)
        {
            printf("Symbol not found: %s\n", var.name().c_str());
            _ASSERT(false);
        }
        // auto ty = symbol.value().type;

        return symbol.value().addr;
    }
    auto get_ptr = Trans_LeftVar_Builder(table, var, builder);
    builder.AddExp(get_ptr);
    return get_ptr.out_address;
    //return Trans_GetElementPtr(table, get_ptr, builder);
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Var& var)
{
    auto address = Trans_Left_Var(table, builder, var);
    auto load = Half_Ir_Load(address);
    builder.AddExp(load);
    return load.GetResult();
}



Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Value& value)
{
    if (auto pint = std::get_if<int>(&value.value))
    {
        auto const_ir = Half_Ir_Const(*pint);
        builder.AddExp(const_ir);
        return const_ir.GetResult();
    }
    else if (auto pstr = std::get_if<std::string>(&value.value))
    {
        auto str_ir = Half_Ir_String(*pstr);
        auto value_ir = Half_Ir_Value(str_ir);
        builder.AddExp(value_ir);
        table->insert(str_ir.label, *pstr);
        return str_ir.GetResult();
    }
    else if (auto pchar = std::get_if<char>(&value.value))
    {
        auto const_ir = Half_Ir_Const(*pchar);
        builder.AddExp(const_ir);
        return const_ir.GetResult();
    }
    else if (auto pfloat = std::get_if<float>(&value.value))
    {
        /*auto const_ir = Half_Ir_Const(*pfloat);
        builder.AddExp(const_ir);
        return const_ir.GetResult();*/
    }

    // TODO: support other value type
    _ASSERT(false);
    return Register{};
}


Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_ArrayInit& arrayinit)
{
    _ASSERT(false); // should't use this function
    return Register{};
}


Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_ArrayNew& arraynew)
{
    _ASSERT(false); // should't use this function
    return Register{};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_StructInit& structinit)
{
    _ASSERT(false); // should't use this function
    return Register{};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Op& op)
{
    auto op_val = Trans_Op_Builder(table, op, builder);
    return op_val;
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Assign& assign)
{
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
        }
        if (auto pstruct = std::get_if<Half_Type_Info::StructType>(&pty.value()->type))
        {
            auto& struct_ty = *pstruct;
            auto t_v = std::make_shared<Half_Var>(assign.left);
            for (size_t i = 0; i < structinit.fields.size(); i++)
            {
                auto& init_field = structinit.fields[i];
                auto& ty_field = init_field.name.empty() ? struct_ty.field_list[i] : struct_ty.GetField(init_field.name);
                // TODO: recursive call to translate the field
                // for now, only support simple type(first level of struct must be simple type)

                Half_Assign init;
                init.left = Half_Var::FieldVar(t_v, ty_field.name);
                init.right = init_field.value;
                Trans_Expr(table, builder, init);
            }
            return Register{};
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
            return Register{};
        }
        if (auto parray = std::get_if<Half_Type_Info::ArrayType>(&pty.value()->type))
        {
            auto& array_ty = *parray;
            auto array_offset = symbol.value().offset;
            auto elem_size = array_ty.type->GetSize();

            // TODO: use left var builder to get the array address
            auto t_v = std::make_shared<Half_Var>(assign.left);
            for (size_t i = 0; i < arrayinit.values.size(); i++)
            {
                auto ci = Half_Ir_Const((int)i);

                Half_Assign init;
                Half_Var l_var;
                auto idx = std::make_shared<Half_Expr>(Half_Value((int)i));
                l_var = Half_Var::SubscriptVar(t_v, idx);
                init.left = l_var;
                init.right = arrayinit.values[i];
                Trans_Expr(table, builder, init);
            }
            return Register{};
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
    }

    // left value is a simple type, store the value directly
    // if left value is pointer, get the address of right value and store it to the left value

    auto address = Trans_Left_Var(table, builder, assign.left);
    Half_Ir_Store store(rval, address);
    builder.AddExp(store);

    return rval;
}

Value Trans_Funcall_Args(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr)
{
    if (auto pvar = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
    {
        auto& var = **pvar;
        // check is var a array type
        auto symbol = table->find(var.name());
        if (!symbol)
        {
            _ASSERT(false);
        }
        if (auto parray = std::get_if<Half_Type_Info::ArrayType>(&symbol.value().type.type))
        {
            auto address = Trans_Left_Var(table, builder, var);
            Half_Ir_FetchPtr fetch(address);
            builder.AddExp(fetch);
            return fetch.GetResult();
        }
    }
    return Trans_Expr(table, builder, expr);
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Funcall& funcall)
{
    std::vector<Half_Ir_Name> arg_exp(funcall.args.size());
    for (size_t i = 0; i < funcall.args.size(); i++)
    {
        auto arg = Trans_Funcall_Args(table, builder, funcall.args[i]);
        arg_exp[i] = arg.GetLabel();
    }
    Register funcall_res = Register{ {}, Temp::NewLabel() };
    Half_Ir_Call call(funcall_res, Temp::Label(funcall.name), arg_exp);
    builder.AddExp(call);
    return call.out_register;
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_FuncDecl& funcDecl)
{
    Stack stack(table);

    auto current_block_size = builder.blocks.size();
    Half_Ir_Function func_ir;
    func_ir.name = funcDecl.name;
    func_ir.args = funcDecl.parameters;

    // 1. Create a new block for the function
    auto alloc_entry = builder.NewBlock("stack_alloc");
    auto block_entry = builder.NewBlock("entry");
    builder.SetAllocEntry(alloc_entry);
    builder.SetInsertPoint(block_entry);

    // 2. Create a new scope for the function
    auto funcscope = stack.NewScope();
    // 3. Add the function parameters to the scope(allocation stack to store the parameters)
    for (size_t i = 0; i < funcDecl.parameters.size(); i++)
    {
        Symbol smb;
        smb.name = funcDecl.parameters[i].var_name;
        smb.type = *(table->findType(funcDecl.parameters[i].type_name).value());
        funcscope->insert(smb);
        func_ir.args_size.push_back(smb.type.GetSize());

        Address addr = smb.addr;
        auto alloc = Half_Ir_Alloca(addr);
        builder.AddExp(alloc_entry, alloc);
    }
    
    // 4. Translate the function body
    auto last_val = Trans_Expr(funcscope, builder, funcDecl.body);
    Half_Ir_Return ret(last_val);
    builder.AddExp(ret);
    // 4.1 set func needed stack size
    func_ir.stack_size = funcscope->stack->max_size;

    // 5. Move all blocks in builder to function ir
    func_ir.alloc = builder.blocks[alloc_entry];
    for (size_t i = block_entry; i < builder.blocks.size(); i++)
    {
        func_ir.blocks.push_back(builder.blocks[i]);
    }

    // 5.1 init block (label, index) map
    std::map<Temp::Label, size_t> block_map;
    for (size_t i = 0; i < func_ir.blocks.size(); i++)
    {
        block_map[func_ir.blocks[i].label] = i;
    }

    // 5.2 update block's pred and succ
    for (size_t i = 0; i < func_ir.blocks.size(); i++)
    {
        auto& block = func_ir.blocks[i];
        // TODO: only last exp in block can be jump or branch
        for (auto& exp : block.exps)
        {
            if (auto pjump = std::get_if<std::shared_ptr<Half_Ir_Jump>>(&exp.exp))
            {
                auto idx = block_map[(*pjump)->target];
                block.succs.push_back(idx);
                func_ir.blocks[idx].preds.push_back(i);
            }
            else if (auto pbranch = std::get_if<std::shared_ptr<Half_Ir_Branch>>(&exp.exp))
            {
                auto t_idx = block_map[(*pbranch)->true_label];
                auto f_idx = block_map[(*pbranch)->false_label];
                block.succs.push_back(t_idx);
                block.succs.push_back(f_idx);
                func_ir.blocks[t_idx].preds.push_back(i);
                func_ir.blocks[f_idx].preds.push_back(i);
            }
        }
    }

    // 6. Remove the blocks from the builder
    builder.blocks.erase(builder.blocks.begin() + block_entry, builder.blocks.end());
    builder.blocks.erase(builder.blocks.begin() + builder.block_alloc_entry);
    builder.insert_point = builder.blocks.size() - 1;
    builder.block_alloc_entry = -1;
    builder.AddExp(func_ir);

    // 7. Add the function to the table
    std::vector<std::string> parameter_types(funcDecl.parameters.size());
    for (size_t i = 0; i < funcDecl.parameters.size(); i++)
    {
        parameter_types[i] = funcDecl.parameters[i].type_name;
    }
    auto func_type_decl = Half_TypeDecl(Half_TypeDecl::FuncType(funcDecl.return_type, parameter_types));
    auto pfunc_ty = Trans_Type(table, func_type_decl);
    auto& func_ty = std::get<Half_Type_Info::FuncType>(pfunc_ty->type);
    table->insert(FunctionSymbol(funcDecl.name, func_ty));

    auto s = "FuncDecl, should't use this label " + funcDecl.name;
    return Register{{}, s};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_If& _if)
{
    // use temp builder to translate condition and body
    //   insert these blocks to the main builder in the end
    //   to make sure the blocks are in the right order

    auto cond_label = builder.GenBlockLabel("if_cond");
    auto block_if_true_label = builder.GenBlockLabel("if_true");
    auto block_if_false_label = builder.GenBlockLabel("if_false");
    auto block_if_end_label = builder.GenBlockLabel("if_merge");

    auto if_result_phi = Half_Ir_Phi(Temp::NewLabel());

    // jump to condition block, if current block is not empty
    //if (builder.blocks[builder.insert_point].exps.size() > 0)
    {
        auto jmp_to_cond = Half_Ir_Jump(cond_label);
        builder.AddExp(jmp_to_cond);
    }

    /// translate condition
    auto cond_entry = builder.NewBlock(cond_label);
    builder.SetInsertPoint(cond_entry);
    Trans_If_Cond(table, builder, _if.condition, "if_cond",
        block_if_true_label, block_if_false_label);

    auto if_true_entry = builder.NewBlock(block_if_true_label);
    builder.SetInsertPoint(if_true_entry);
    if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_Expr>>>(&_if.trueExpr.expr))
    {
        auto& vec = **pvec;
        auto true_table = Table::begin_scope(table);
        Value result = Register{};
        for (size_t i = 0; i < vec.size(); i++)
        {
            result = Trans_Expr(true_table, builder, vec[i]);
            Half_Ir_Name l = result.GetLabel();
        }
        if_result_phi.Insert(Half_Ir_Name(result.GetLabel()), block_if_true_label);
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
        Value result = Register{};
        for (size_t i = 0; i < vec.size(); i++)
        {
            result = Trans_Expr(false_table, builder, vec[i]);
            Half_Ir_Name l = result.GetLabel();
        }
        if_result_phi.Insert(Half_Ir_Name(result.GetLabel()), block_if_false_label);
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

    builder.AddExp(if_result_phi);
    return if_result_phi.GetResult();
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_While& _while)
{
    auto cond_label = builder.GenBlockLabel("while_cond");
    auto body_label = builder.GenBlockLabel("while_body");
    auto end_label = builder.GenBlockLabel("while_end");

    // jump to condition block, if current block is not empty
    //if (builder.blocks[builder.insert_point].exps.size() > 0)
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
    return Register{};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_For& _for)
{
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
    //if (builder.blocks[builder.insert_point].exps.size() > 0)
    {
        auto jmp_to_cond = Half_Ir_Jump(cond_label);
        builder.AddExp(jmp_to_cond);
    }

    // condition block
    auto cond_block = builder.NewBlock(cond_label);
    builder.SetInsertPoint(cond_block);
    auto cond = Half_Op(_for.isup? "<=" : ">=", Half_Op::Half_OpExpr(_for.var), Half_Op::Half_OpExpr(termi_var));
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
    return Register{};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, std::vector<Half_Expr>& exprs)
{
    for (size_t i = 0; i < exprs.size() - 1; i++)
    {
        Trans_Expr(table, builder, exprs[i]);
    }
    return Trans_Expr(table, builder, exprs.back());
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_TypeDecl& typeDecl)
{
    Trans_Type(table, typeDecl);
    return Register{};
}

Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr)
{
    return std::visit([&](auto&& arg) {
        return Trans_Expr(table, builder, *arg);
        }, expr.expr);
}

void Trans_Outer(Half_OuterExpr& outer, Builder& builder)
{
    auto table = std::make_shared<Table>();
    Init_Basic_Type(table);
    Trans_Outer(table, builder, outer);

    // move strings in table to builder
    for (auto& [l, s] : table->strings)
    {
        builder.InsertString(l, s);
    }
}

void Trans_Outer(std::shared_ptr<Table>& table, Builder& builder, Half_OuterExpr& outer)
{
    if (auto pfunc = std::get_if<std::shared_ptr<Half_FuncDecl>>(&outer.expr))
    {
        Trans_Expr(table, builder, **pfunc);
    }
    else if (auto ptype = std::get_if<std::shared_ptr<Half_TypeDecl>>(&outer.expr))
    {
        Trans_Type(table, **ptype);
    }
    else if (auto pvec = std::get_if<std::shared_ptr<std::vector<Half_OuterExpr>>>(&outer.expr))
    {
        auto& vec = **pvec;
        for (size_t i = 0; i < vec.size(); i++)
        {
            Trans_Outer(table, builder, vec[i]);
        }
    }
}
