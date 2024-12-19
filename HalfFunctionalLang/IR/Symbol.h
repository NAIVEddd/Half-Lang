#pragma once
#include"../Syntax/Base.h"
#include"Temp.h"
#include"IR.h"
#include<string>
#include<map>
#include<optional>


enum class TypeCategory
{
    Basic,
    Compose
};
enum class BasicType
{
    Char,
    Int,
    Float,
    String
};
struct TupleTypeDesc
{
    using Type = std::variant<BasicType, TupleTypeDesc>;
    TypeCategory Category;
    std::vector<Type> Storage;
};
struct TupleValueDesc
{
    using Type = std::variant<BasicValue, TupleValueDesc>;
    TypeCategory Category;
    std::vector<Type> Storage;
};
// 1, 2, (1, 2), (1, (2, 3)), (1, (2, 3), 4)
// int, int, tuple int * int, tuple int * (int * int)
// compose[int, int], compose[int, compose[int, int]]
// TypeDesc(Compose, [Int, Int])
// TypeDesc(Compose, [Int, TypeDesc(Compose, [Int, Int])])
// TypeDesc(Compose, [Int, TypeDesc(Compose, [Int, Int]), int])
// (Basic, BasicValue(1)), (Basic, BasicValue(2)),
//	 (Compose, ((Basic, BasicValue(2)), (Basic, BasicValue(3))))

struct Table;

struct Type
{
    BasicType type;
    int offset;
    int size;
};

struct Symbol
{
    std::string name;
    Type type;
    BasicValue value;
    size_t offset;

    Symbol() = default;
    Symbol(const Symbol& o)
        : name(o.name)
        , type(o.type)
        , value(o.value)
        , offset(o.offset)
    {}
};

struct FunctionSymbol
{
    std::string name;
    Temp label;
    std::shared_ptr<Table> scope;
    Half_Ir_Exp body;

    FunctionSymbol()
        : label(Temp::NewLabel("")) {}
};

struct Stack;
struct Table
{
    std::shared_ptr<Table> parent;
    Stack* stack = nullptr;
    std::map<std::string, Symbol> values;
    std::map<std::string, FunctionSymbol> funcs;
    std::map<std::string, Temp::Label> labels;

    Table() = default;
    Table(std::shared_ptr<Table>& p) : parent(p), stack(p->stack) {}
    ~Table();

    static std::shared_ptr<Table> begin_scope(std::shared_ptr<Table>& p)
    {
        auto ptr = std::make_shared<Table>(p);
        ptr->stack = p->stack;
        return ptr;
    }
    static std::shared_ptr<Table> end_scope(std::shared_ptr<Table>& t)
    {
        return t->parent;
    }

    void insert(Symbol& s);
    void insert(FunctionSymbol f)
    {
        funcs.insert({ f.name, f });
    }
    std::optional<Symbol> find(std::string n, bool rec = true) const
    {
        auto search = values.find(n);
        if (search == values.end())
        {
            if (rec)
            {
                if (!parent)
                {
                    return std::nullopt;
                }
                return parent->find(n);
            }
            return std::nullopt;
        }
        return search->second;
    }
    std::optional<FunctionSymbol> findFunc(std::string n, bool rec = true)
    {
        auto search = funcs.find(n);
        if (search == funcs.end())
        {
            if (rec)
            {
                if (!parent)
                {
                    return std::nullopt;
                }
                return parent->findFunc(n);
            }
            return std::nullopt;
        }
        return search->second;
    }
};

struct Stack
{
    Stack() : table(std::make_shared<Table>()) {}
    Stack(std::shared_ptr<Table> t) : table(t) {}

    size_t Alloc(size_t size)
    {
        size_t res = offset;
        offset += size;
        return res;
    }
    void Release(size_t size)
    {
        offset -= size;
    }
    std::shared_ptr<Table> NewScope()
    {
        auto t = table->begin_scope(table);
        t->stack = this;
        return t;
    }
    void insert(Symbol& s)
    {
        table->insert(s);
    }
    void insert(FunctionSymbol f)
    {
        table->insert(f);
    }
    std::optional<Symbol> find(std::string n) const
    {
        return table->find(n);
    }
    std::optional<FunctionSymbol> findFunc(std::string n)
    {
        return table->findFunc(n);
    }
    size_t offset = 0;
    std::shared_ptr<Table> table;
};
