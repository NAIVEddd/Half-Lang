#pragma once
#include <memory>
#include <string>
#include <vector>
#include <variant>

enum class Half_TypeID
{
    Basic,
    Pointer,
    Tuple,
    Rename,
    Array,
    Struct,
    Func
};

struct Half_Type_Info
{
    struct BasicType
    {
        enum class BasicT
        {
            Char,
            Int,
            Float,
            String
        };
        BasicType() = default;
        BasicType(BasicT t) : type(t) {}

        BasicT type;
    };
    struct PointerType
    {
        PointerType(std::shared_ptr<Half_Type_Info> t) : type(std::move(t)) {}
        std::shared_ptr<Half_Type_Info> type;
    };
    struct TupleType
    {
        using T = std::shared_ptr<Half_Type_Info>;
        TupleType(std::vector<T>& t) : type_list(t) {}
        std::vector<T> type_list;
    };
    struct RenameType
    {
        RenameType(std::string n, std::shared_ptr<Half_Type_Info> t) : name(n), type(t) {}
        std::string name;
        std::shared_ptr<Half_Type_Info> type;
    };
    struct ArrayType
    {
        ArrayType(std::shared_ptr<Half_Type_Info> t, size_t c) : type(t), count(c) {}
        std::shared_ptr<Half_Type_Info> type;
        size_t count;
    };
    struct StructType
    {
        struct TypePair
        {
            std::string name;
            std::shared_ptr<Half_Type_Info> type;
            size_t offset;
            TypePair(std::string n, std::shared_ptr<Half_Type_Info> t, size_t _offset) : name(n), type(t), offset(_offset) {}
            TypePair(const TypePair& o) : name(o.name), type(o.type), offset(o.offset) {}
        };
        StructType(std::string n, std::vector<TypePair> f) : name(n), field_list(f) {}
        TypePair& GetField(std::string name);

        std::string name;
        std::vector<TypePair> field_list;
    };
    struct FuncType
    {
        FuncType(std::shared_ptr<Half_Type_Info> ret_, std::vector<std::shared_ptr<Half_Type_Info>> args_)
            : ret(std::move(ret_)), args(std::move(args_)){}

        std::shared_ptr<Half_Type_Info> ret;
        std::vector<std::shared_ptr<Half_Type_Info>> args;
        
    };
    using Type = std::variant<BasicType, PointerType, TupleType, RenameType, ArrayType, StructType, FuncType>;
    Type type;
    Half_Type_Info() = default;
    template<typename T>
    Half_Type_Info(const T& t) : type(t) {}
    Half_Type_Info(const Half_Type_Info& o) : type(o.type) {}

    size_t GetSize();
};