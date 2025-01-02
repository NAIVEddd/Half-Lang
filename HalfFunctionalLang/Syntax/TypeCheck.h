#pragma once
#include"Base.h"
#include"ExprParser.h"

#include<map>
#include<set>

struct Scope
{
    std::shared_ptr<Scope> parent;
    std::map<std::string, Half_TypeDecl> vardecls;
    //std::map<std::string, Half_Type> types;
    std::map<std::string, Half_TypeDecl> typedecls;
    std::map<std::string, Half_TypeDecl> funcdecls;
    Scope() = default;
    Scope(std::shared_ptr<Scope> p) : parent(std::move(p)) {}
    bool insert_var(const std::string& name, Half_TypeDecl var);
    bool insert_type(const std::string& name, Half_TypeDecl type);
    bool insert_func(const std::string& name, Half_TypeDecl func);
    std::optional<Half_TypeDecl> find_var(const std::string& name);
    std::optional<Half_TypeDecl> find_type(const std::string& name);
    std::optional<Half_TypeDecl> find_func(const std::string& name);
    std::shared_ptr<Scope> CreateChild(std::shared_ptr<Scope> p)
    {
        return std::make_shared<Scope>(p);
    }
};

struct TypeCheck
{
    std::shared_ptr<Scope> scope;
    TypeCheck() : scope(std::make_shared<Scope>())
    {
    /*    scope->insert_type("char", Half_TypeDecl(Half_TypeDecl::BasicType(Half_TypeDecl::BasicType::BasicT::Char)));
        scope->insert_type("int", Half_TypeDecl(Half_TypeDecl::BasicType(Half_TypeDecl::BasicType::BasicT::Int)));
        scope->insert_type("float", Half_TypeDecl(Half_TypeDecl::BasicType(Half_TypeDecl::BasicType::BasicT::Float)));
        scope->insert_type("string", Half_TypeDecl(Half_TypeDecl::BasicType(Half_TypeDecl::BasicType::BasicT::String)));
    */}
    TypeCheck(std::shared_ptr<Scope> s) : scope(std::move(s)) {}
    bool Check(const Half_Expr& expr);
    Half_TypeDecl GetType(const Half_Expr& expr);

private:
    // get expression type
    Half_TypeDecl GetType(const Half_Op::Half_OpExpr& expr);
    Half_TypeDecl GetType(const Half_Var& expr);
    Half_TypeDecl GetType(const Half_Value& expr);
    Half_TypeDecl GetType(const Half_Funcall& expr);
    Half_TypeDecl GetType(const Half_Op& expr);

    bool Check(const Half_Var& var);
    bool Check(const Half_ArrayInit& expr);
    bool Check(const Half_ArrayNew& expr);
    bool Check(const Half_StructInit& expr);
    bool Check(const Half_FuncDecl& func);
    bool Check(const Half_TypeDecl& type);
    //bool Check(const Half_Type& type);
    bool Check(const Half_Op& op);
    bool Check(const Half_Assign& assign);
    bool Check(const Half_Let& let);
    bool Check(const Half_If& ifexpr);
    bool Check(const Half_For& forloop);
    bool Check(const Half_While& whileloop);
    bool Check(const Half_Funcall& funcall);
    bool Check(const Half_Value& value);
    bool Check(const Half_Var::SimpleVar& var);
    bool Check(const Half_Var::FieldVar& var);
    bool Check(const Half_Var::SubscriptVar& var);
    bool Check(const std::vector<Half_Expr>& exprs);
    //bool Check(const Def_Var& var);
};