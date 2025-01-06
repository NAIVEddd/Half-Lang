#pragma once
#include"../Syntax/Base.h"
#include"Temp.h"
#include"IR.h"
#include"Type.h"
#include<string>
#include<map>
#include<optional>


//enum class TypeCategory
//{
//    Basic,
//    Compose
//};
//enum class BasicType
//{
//    Char,
//    Int,
//    Float,
//    String
//};
//struct TupleTypeDesc
//{
//    using Type = std::variant<BasicType, TupleTypeDesc>;
//    TypeCategory Category;
//    std::vector<Type> Storage;
//};
//struct TupleValueDesc
//{
//    using Type = std::variant<BasicValue, TupleValueDesc>;
//    TypeCategory Category;
//    std::vector<Type> Storage;
//};
// 1, 2, (1, 2), (1, (2, 3)), (1, (2, 3), 4)
// int, int, tuple int * int, tuple int * (int * int)
// compose[int, int], compose[int, compose[int, int]]
// TypeDesc(Compose, [Int, Int])
// TypeDesc(Compose, [Int, TypeDesc(Compose, [Int, Int])])
// TypeDesc(Compose, [Int, TypeDesc(Compose, [Int, Int]), int])
// (Basic, BasicValue(1)), (Basic, BasicValue(2)),
//	 (Compose, ((Basic, BasicValue(2)), (Basic, BasicValue(3))))

struct Table;

//struct Type
//{
//    BasicType type;
//    int offset;
//    int size;
//};

struct Symbol
{
    Symbol() = default;
    Symbol(const Symbol& o)
        : name(o.name)
        , type(o.type)
        , value(o.value)
        , offset(o.offset)
        , addr(o.addr)
    {}


    std::string name;
    Half_Type_Info type;
    BasicValue value;
    ptrdiff_t offset = 0;
    Address addr;
};

struct FunctionSymbol
{
    FunctionSymbol()
        = default; //: label(Temp::NewLabel("")) {}
    FunctionSymbol(std::string n, Half_Type_Info::FuncType t) : name(std::move(n)), type(std::move(t)) {}
    FunctionSymbol(const FunctionSymbol& o)
        : name(o.name) , type(o.type){}

    std::string name;
    //Temp label;
    //std::shared_ptr<Table> scope;
    //Half_Ir_Exp body;
    Half_Type_Info::FuncType type;
};

struct Stack;
struct Table
{
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
    void insert(std::string name, std::shared_ptr<Half_Type_Info> t)
    {
        types.insert({ name, t });
    }
    std::optional<Symbol> find(std::string n, bool rec = true) const;
    std::optional<FunctionSymbol> findFunc(std::string n, bool rec = true);
    std::optional<std::shared_ptr<Half_Type_Info>> findType(std::string n, bool rec = true);

    std::shared_ptr<Table> parent;
    Stack* stack = nullptr;
    size_t total_size = 0;
    std::map<std::string, Symbol> values;
    std::map<std::string, FunctionSymbol> funcs;
    //std::map<std::string, Temp::Label> labels;
    std::map<std::string, std::shared_ptr<Half_Type_Info>> types;
};

struct Stack
{
    Stack() : table(std::make_shared<Table>()) {}
    Stack(std::shared_ptr<Table> t) : table(t) {}

    // TODO: stack alloc need to know the data size and align size
    //   also need to know the start from ebp(top) or esp(bottom)
    ptrdiff_t Alloc(size_t size)
    {
        size_t res = offset;
        offset += size;

        if (offset > max_size)
        {
            max_size = offset;
        }
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
    /*void insert(Symbol& s)
    {
        table->insert(s);
    }*/
    /*void insert(FunctionSymbol f)
    {
        table->insert(f);
    }*/
    std::optional<Symbol> find(std::string n) const
    {
        return table->find(n);
    }
    std::optional<FunctionSymbol> findFunc(std::string n)
    {
        return table->findFunc(n);
    }
    std::optional<std::shared_ptr<Half_Type_Info>> findType(std::string n)
    {
        return table->findType(n);
    }
    size_t offset = 0;
    size_t max_size = 0;        // stack alloc need to know the max size
    std::shared_ptr<Table> table;
};
