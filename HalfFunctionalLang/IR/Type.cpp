#include"Type.h"


size_t GetSize(Half_Type_Info::BasicType& type)
{
    using BasicT = Half_Type_Info::BasicType::BasicT;
    switch (type.type)
    {
    case BasicT::Char:
        return sizeof(char);
    case BasicT::Int:
        return sizeof(int);
    case BasicT::Float:
        return sizeof(float);
    case BasicT::String:
        return sizeof(char*);
    default:
        break;
    }
    // TODO: other type
    //_ASSERT(false);
    //return 0;
    return 4;
}

size_t GetSize(Half_Type_Info::PointerType& type)
{
    return sizeof(void*);
}

size_t GetSize(Half_Type_Info::TupleType& type)
{
    size_t size = 0;
    for (auto& t : type.type_list)
    {
        size += t->GetSize();
    }
    return size;
}

size_t GetSize(Half_Type_Info::ArrayType& type)
{
    return type.count * type.type->GetSize();
}

size_t GetSize(Half_Type_Info::StructType& type)
{
    size_t size = 0;
    for (auto& t : type.field_list)
    {
        size += t.type->GetSize();
    }
    return size;
}

size_t GetSize(Half_Type_Info::FuncType& type)
{
    return sizeof(void*);
}

bool Half_Type_Info::is_pointer() const
{
    return std::holds_alternative<Half_Type_Info::PointerType>(type);
}

bool Half_Type_Info::is_basic() const
{
    return std::holds_alternative<Half_Type_Info::BasicType>(type);
}

bool Half_Type_Info::is_float() const
{
    return std::holds_alternative<Half_Type_Info::BasicType>(type)
        && std::get<Half_Type_Info::BasicType>(type).type == Half_Type_Info::BasicType::BasicT::Float;
}

std::string Half_Type_Info::to_string() const
{
    if (std::holds_alternative<Half_Type_Info::BasicType>(type))
    {
        auto& t = std::get<Half_Type_Info::BasicType>(type);
        using BasicT = Half_Type_Info::BasicType::BasicT;
        switch (t.type)
        {
        case BasicT::Char:
            return "char";
        case BasicT::Int:

            return "int";
        case BasicT::Float:
            return "float";
        case BasicT::String:
            return "string";
        default:
            return "invalid";
        }
    }
    else if (std::holds_alternative<Half_Type_Info::PointerType>(type))
    {
        return "pointer of " + std::get<Half_Type_Info::PointerType>(type).type->to_string();
    }
    else if (std::holds_alternative<Half_Type_Info::TupleType>(type))
    {
        return "tuple";
    }
    else if (std::holds_alternative<Half_Type_Info::ArrayType>(type))
    {
        return "array of " + std::get<Half_Type_Info::ArrayType>(type).type->to_string();
    }
    else if (std::holds_alternative<Half_Type_Info::StructType>(type))
    {
        return "struct " + std::get<Half_Type_Info::StructType>(type).name;
    }
    else if (std::holds_alternative<Half_Type_Info::FuncType>(type))
    {
        return "func " + std::get<Half_Type_Info::FuncType>(type).ret->to_string() + " (" + std::to_string(std::get<Half_Type_Info::FuncType>(type).args.size()) + " args)";
    }
    return std::string();
}

size_t Half_Type_Info::GetSize()
{
    return std::visit([](auto&& arg) -> size_t {
        return ::GetSize(arg);
    }, type);
}

Half_Type_Info::StructType::TypePair& Half_Type_Info::StructType::GetField(size_t index)
{
    _ASSERT(index < field_list.size());
    return field_list[index];
}

Half_Type_Info::StructType::TypePair& Half_Type_Info::StructType::GetField(std::string name)
{
    for (auto& f : field_list)
    {
        if (f.name == name)
        {
            return f;
        }
    }
    // TODO: error report
    _ASSERT(false);
    return field_list[0];
}

size_t Half_Type_Info::StructType::GetFieldIndex(std::string name)
{
    for (size_t i = 0; i < field_list.size(); i++)
    {
        if (field_list[i].name == name)
        {
            return i;
        }
    }
    _ASSERT(false);
    return -1;
}
