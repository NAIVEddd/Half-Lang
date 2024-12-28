#include "TypeCheck.h"

bool Scope::insert_var(const std::string& name, Half_TypeDecl var)
{
    if (vardecls.find(name) != vardecls.end())
    {
        return false;
    }
    vardecls.insert({name, var});
    return true;
}

bool Scope::insert_type(const std::string& name, Half_TypeDecl type)
{
    if (typedecls.find(name) != typedecls.end())
    {
        return false;
    }
    typedecls.insert({ name, type });
    return true;
}

bool Scope::insert_func(const std::string& name, Half_TypeDecl func)
{
    if (funcdecls.find(name) != funcdecls.end())
    {
        return false;
    }
    funcdecls.insert({ name, func });
    return true;
}

std::optional<Half_TypeDecl> Scope::find_var(const std::string& name)
{
    auto it = vardecls.find(name);
    if (it != vardecls.end())
    {
        return it->second;
    }
    if (parent)
    {
        return parent->find_var(name);
    }
    return std::nullopt;
}

std::optional<Half_TypeDecl> Scope::find_type(const std::string& name)
{
    auto it = typedecls.find(name);
    if (it != typedecls.end())
    {
        return it->second;
    }
    if (parent)
    {
        return parent->find_type(name);
    }
    return std::nullopt;
}

std::optional<Half_TypeDecl> Scope::find_func(const std::string& name)
{
    auto it = funcdecls.find(name);
    if (it != funcdecls.end())
    {
        return it->second;
    }
    if (parent)
    {
        return parent->find_func(name);
    }
    return std::nullopt;
}

bool TypeCheck::Check(const Half_Expr& expr)
{
    return std::visit([this](const auto& e) {return Check(*e); }, expr.expr);
}

// basic type check
//   TODO: complex type check
Half_TypeDecl TypeCheck::GetType(const Half_Expr& expr)
{
//    if (auto ptr = std::get_if<std::shared_ptr<Half_Var>>(&expr.expr))
//    {
//        auto t = scope->find_var((*ptr)->name());
//        return t.value_or(Half_TypeDecl());
//    }
//    else if (auto ptr = std::get_if<std::shared_ptr<Half_Value>>(&expr.expr))
//    {
//        if (Half_TypeDecl::BasicType::is_basic_t(**ptr))
//        {
//            auto t = (Half_TypeDecl::BasicType::BasicT)(*ptr)->value.index();
//            return Half_TypeDecl(Half_TypeDecl::BasicType(t));
//        }
//        return Half_TypeDecl();
//    }
//    else if (auto ptr = std::get_if<std::shared_ptr<Half_Op>>(&expr.expr))
//    {
//        return GetType(**ptr);
//    }
//    else if (auto ptr = std::get_if<std::shared_ptr<Half_Funcall>>(&expr.expr))
//    {
//        return GetType(**ptr);
//    }
    return Half_TypeDecl();
}

Half_TypeDecl TypeCheck::GetType(const Half_Op::Half_OpExpr& expr)
{
    return std::visit([this](const auto& e) {return GetType(e); }, expr);
}

Half_TypeDecl TypeCheck::GetType(const Half_Var& expr)
{
    auto t = scope->find_var(expr.name());
    if (t)
    {
        printf("GetType(Half_Var) %s\n", expr.name().c_str());
    }
    return t.value_or(Half_TypeDecl());
}

Half_TypeDecl TypeCheck::GetType(const Half_Value& expr)
{
//    _ASSERT(expr.value.index() < 4);
//    auto bt = (Half_TypeDecl::BasicType::BasicT)expr.value.index();
//    printf("GetType(Half_Value) %d\n", (int)bt);
//    return Half_TypeDecl::BasicType(bt);
    return{};
}

Half_TypeDecl TypeCheck::GetType(const Half_Funcall& expr)
{
    printf("GetType(Half_Funcall)\n");
    return Half_TypeDecl();
}

Half_TypeDecl TypeCheck::GetType(const Half_Op& expr)
{
    auto tl = GetType(*expr.left);
    auto tr = GetType(*expr.right);
    if (tl == Half_TypeDecl() || tl != tr)
    {
        // error log
        return Half_TypeDecl();
    }
    return tl;
}

bool TypeCheck::Check(const Half_Var& var)
{
    auto t = scope->find_var(var.name());
    return t.has_value();
}

bool TypeCheck::Check(const Half_StructInit& expr)
{
    return false;
}

// Todo: support function template
bool TypeCheck::Check(const Half_FuncDecl& func)
{
    // check return type
    auto t = scope->find_type(func.return_type);
    if (!t)
    {
        printf("Check(Half_FuncDecl) %s return type not found\n", func.name.c_str());
        return false;
    }
    // check parameters type
    /*std::vector<Half_TypeDecl::FuncType::TypeField> fields;
    fields.reserve(func.parameters.size());
    for (auto& f : func.parameters)
    {
        auto t = scope->find_type(f.type_name);
        if (!t)
        {
            printf("Check(Half_FuncDecl) %s parameter %s type not found\n", func.name.c_str(), f.type_name.c_str());
            return false;
        }
        fields.push_back({ f.var_name, f.type_name });
    }
    Half_TypeDecl::FuncType def(fields, func.return_type);
    return scope->insert_func(func.name, def);*/
    return false;
}

bool TypeCheck::Check(const Half_TypeDecl& type)
{
    return false;
}

bool TypeCheck::Check(const Half_Op& op)
{
    auto tl = GetType(*op.left);
    auto tr = GetType(*op.right);
    printf("Check(Half_Op) %zd %zd\n", tl.type.index(), tr.type.index());
    return tl != Half_TypeDecl() && tl == tr;
}

bool TypeCheck::Check(const Half_Assign& assign)
{
    auto t1 = GetType(assign.left);
    auto t2 = GetType(assign.right);
    return t1 != Half_TypeDecl() && t1 == t2;
}

bool TypeCheck::Check(const Half_Let& let)
{
    auto t = GetType(let.def.right);
    if (t == Half_TypeDecl())
    {
        return false;
    }
    
    return scope->insert_var(let.def.left.name(), t);
}

bool TypeCheck::Check(const Half_If& ifexpr)
{
    return false;
}

bool TypeCheck::Check(const Half_For& forloop)
{
    return false;
}

bool TypeCheck::Check(const Half_While& whileloop)
{
    return false;
}

bool TypeCheck::Check(const Half_Funcall& funcall)
{
    auto f = scope->find_func(funcall.name);
    if (!f)
    {
        printf("Check(Half_Funcall) %s not found\n", funcall.name.c_str());
        return false;
    }
    auto func_def = f.value();
    // Todo: support function template
    if (auto ptr = std::get_if<Half_TypeDecl::FuncType>(&func_def.type))
    {
        auto& func = *ptr;
        /*if (func.parameters.size() != funcall.args.size())
        {
            printf("Check(Half_Funcall) %s parameter size not match\n", funcall.name.c_str());
            return false;
        }
        for (size_t i = 0; i < func.parameters.size(); i++)
        {
            auto req_t = scope->find_type(func.parameters[i].type_name);
            if (!req_t)
            {
                printf("Check(Half_Funcall) %s parameter %zd type not found\n", funcall.name.c_str(), i);
                return false;
            }
            auto t = GetType(funcall.args[i]);
            if (t != req_t)
            {
                printf("Check(Half_Funcall) %s parameter %zd type not match\n", funcall.name.c_str(), i);
                return false;
            }
        }*/
        return true;
    }

    return false;
}

bool TypeCheck::Check(const Half_Value& value)
{
    return false;
}

bool TypeCheck::Check(const Half_Var::SimpleVar& var)
{
    return false;
}

bool TypeCheck::Check(const Half_Var::FieldVar& var)
{
    return false;
}

bool TypeCheck::Check(const Half_Var::SubscriptVar& var)
{
    return false;
}

bool TypeCheck::Check(const std::vector<Half_Expr>& exprs)
{
    for (size_t i = 0; i < exprs.size(); i++)
    {
        if (!Check(exprs[i]))
        {
            return false;
        }
    }
    return true;
}

//bool TypeCheck::Check(const Def_Var& var)
//{
//    return Check(var.left) && Check(var.right);
//}
