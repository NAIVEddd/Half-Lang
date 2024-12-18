#include "Base.h"




Half_Var::FieldVar::FieldVar(std::shared_ptr<Half_Var>& v, std::string s)
    :var(v), id(s)
{
}

Half_Var::FieldVar::FieldVar(const FieldVar& v) noexcept
    :var(v.var), id(v.id)
{
}

Half_Var::FieldVar& Half_Var::FieldVar::operator=(const FieldVar& v) noexcept
{
    var = v.var;
    id = v.id;
    return *this;
}

Half_Var::SubscriptVar::SubscriptVar(std::shared_ptr<Half_Var>& v, std::shared_ptr<Half_Expr>& e)
    :var(v), index(e)
{
}

Half_Var::SubscriptVar::SubscriptVar(const SubscriptVar& v)
    :var(v.var), index(v.index)
{
}

Half_Var::SubscriptVar& Half_Var::SubscriptVar::operator=(const SubscriptVar& v) noexcept
{
    var = v.var;
    index = v.index;
    return *this;
}

std::string Half_Var::name() const
{
    /*return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, SimpleVar>) {
            return arg.id;
        }
        else if constexpr (std::is_same_v<T, FieldVar>) {
            return arg.id;
        }
        else if constexpr (std::is_same_v<T, SubscriptVar>) {
            return "SubscriptVar";
        }
        else {
            return "Unknown";
        }
        }, var);*/
    if (auto pvar = std::get_if<SimpleVar>(&var))
    {
        return pvar->id;
    }
    return std::string();
}

Half_Var::Half_Var(SimpleVar&& v)
{
    var = v;
}

Half_Var::Half_Var(FieldVar&& v)
{
    var.emplace<FieldVar>(std::move(v));
}

Half_Var::Half_Var(SubscriptVar&& v)
{
    var.emplace<SubscriptVar>(std::move(v));
}

Half_Var::Half_Var(const Half_Var& v)
    :var(v.var)
{
}

Half_Var& Half_Var::operator=(const Half_Var& v)
{
    var = v.var;
    return *this;
}

Half_Value& Half_Value::operator=(const Half_Value& v)
{
    value = v.value;
    return *this;
}

Half_Funcall::Half_Funcall(std::string n, std::vector<Half_Expr>&& args_)
    : name(n), args(std::move(args_))
{
}

Half_Funcall::Half_Funcall(const Half_Funcall& other)
    : name(other.name), args(other.args)
{
}

Half_Funcall& Half_Funcall::operator=(const Half_Funcall& other)
{
    name = other.name;
    args = other.args;
    return *this;
}

Half_Op::Half_Op(std::string o, Half_OpExpr&& l, Half_OpExpr&& r)
    : op(o)
{
    left = std::make_unique<Half_OpExpr>(std::move(l));
    right = std::make_unique<Half_OpExpr>(std::move(r));
}

Half_Op::Half_Op(const Half_Op& other)
    : op(other.op)//, left(other.left), right(other.right)
{
    left = std::make_unique<Half_OpExpr>(*other.left);
    right = std::make_unique<Half_OpExpr>(*other.right);
}

Half_Op& Half_Op::operator=(const Half_Op& other)
{
    op = other.op;
    left = std::make_unique<Half_OpExpr>(*other.left);
    right = std::make_unique<Half_OpExpr>(*other.right);
    return *this;
}

Half_Assign::Half_Assign(const Half_Var& l, const Half_Expr& r)
    : left(l), right(r)
{
}

Half_Assign::Half_Assign(const Half_Assign& other)
    : left(other.left), right(other.right)
{
}

Half_Assign& Half_Assign::operator=(const Half_Assign& other)
{
    left = other.left;
    right = other.right;
    return *this;
}

Half_Let::Half_Let(const Def_Var& v)
    : def(v)
{
}

Half_Let::Half_Let(const Half_Let& o)
    : def(o.def)
{
}

Half_Let& Half_Let::operator=(const Half_Let& o)
{
    def = o.def;
    return *this;
}

Half_If::Half_If(const Half_Expr& cond, const Half_Expr& t, const Half_Expr& f)
    : condition(cond), trueExpr(t), falseExpr(f)
{
}

Half_If::Half_If(const Half_If& other)
    : condition(other.condition), trueExpr(other.trueExpr), falseExpr(other.falseExpr)
{
}

Half_If& Half_If::operator=(const Half_If& other)
{
    condition = other.condition;
    trueExpr = other.trueExpr;
    falseExpr = other.falseExpr;
    return *this;
}

Half_For::Half_For(const Half_Var& v, const Half_Expr& s, const Half_Expr& e, const Half_Expr& b, bool up)
    : var(v), start(s), end(e), body(b), isup(up)
{
}

Half_For::Half_For(const Half_For& o)
    : var(o.var), start(o.start), end(o.end), body(o.body), isup(o.isup)
{
}

Half_For& Half_For::operator=(const Half_For& o)
{
    var = o.var;
    start = o.start;
    end = o.end;
    body = o.body;
    isup = o.isup;
    return *this;
}

Half_While::Half_While(const Half_Expr& c, const Half_Expr& b)
    : condition(c), body(b)
{
}

Half_While::Half_While(const Half_While& o)
    : condition(o.condition), body(o.body)
{
}

Half_While& Half_While::operator=(const Half_While& o)
{
    condition = o.condition;
    body = o.body;
    return *this;
}

Def_Type::Def_Type(const RenameType& t)
    : type(t)
{
}

Def_Type::Def_Type(const ArrayType& t)
    : type(t)
{
}

Def_Type::Def_Type(const StructType& t)
    : type(t)
{
}

Def_Type::Def_Type(const Def_Type& o)
    : type(o.type)
{
}

Def_Type& Def_Type::operator=(const Def_Type& o)
{
    type = o.type;
    return *this;
}

Half_TypeDecl::Half_TypeDecl(const BasicType& t)
    : type(t)
{
}

Half_TypeDecl::Half_TypeDecl(const RenameType& t)
    : type(t)
{
}

Half_TypeDecl::Half_TypeDecl(const ArrayType& t)
    : type(t)
{
}

Half_TypeDecl::Half_TypeDecl(const StructType& t)
    : type(t)
{
}

Half_TypeDecl::Half_TypeDecl(const FuncType& t)
    : type(t)
{
}

Half_TypeDecl::Half_TypeDecl(const Half_TypeDecl& o)
    : type(o.type)
{
}

Half_TypeDecl& Half_TypeDecl::operator=(const Half_TypeDecl& o)
{
    type = o.type;
    return *this;
}

bool Half_TypeDecl::operator==(const Half_TypeDecl& o) const
{
    return type == o.type;
}

bool Half_TypeDecl::operator!=(const Half_TypeDecl& o) const
{
    return !(*this == o);
}

bool Half_TypeDecl::BasicType::is_basic_t(const Half_Value& v)
{
    auto idx = v.value.index();
    if (idx >= 0 && idx < 4)
    {
        return true;
    }
    return false;
}
