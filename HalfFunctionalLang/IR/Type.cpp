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
        return sizeof(std::string);
    default:
        break;
    }
    _ASSERT(false);
    return 0;
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

size_t GetSize(Half_Type_Info::RenameType& type)
{
    return type.type->GetSize();
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

size_t Half_Type_Info::GetSize()
{
    return std::visit([](auto&& arg) -> size_t {
        return ::GetSize(arg);
    }, type);
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
