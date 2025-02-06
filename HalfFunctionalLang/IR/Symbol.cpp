#include "Symbol.h"

Table::~Table()
{
    if (stack)
    {
        stack->Release(total_size);
    }
}

void Table::insert(Temp::Label l, std::string str)
{
    if (parent)
    {
        parent->insert(l, str);
    }
    else
    {
        strings.insert({ l, str });
    }
}

void Table::insert(Symbol& s)
{
    auto sz = s.type.GetSize();
    sz = sz ? sz : 4;
    total_size += sz;
    RealAddress addr;
    addr.base = Temp::Label("bottom");
    addr.offset = stack->Alloc(sz);
    addr.type = s.type;
    if (!s.addr.real_address)
    {
        s.addr.real_address = std::make_shared<RealAddress>(addr);
    }
    else
    {
        s.addr.real_address->base = addr.base;
        s.addr.real_address->offset = addr.offset;
        s.addr.real_address->type = addr.type;
    }
    s.addr.target_type = s.type;
    s.offset = s.addr.real_address->offset;
    values.insert({ s.name, s });
}

std::optional<Symbol> Table::find(std::string n, bool rec) const
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

std::optional<FunctionSymbol> Table::findFunc(std::string n, bool rec)
{
    auto search = funcs.find(n);
    if (search == funcs.end())
    {
        if (rec)
        {
            if (!parent)
            {
                if (stack)
                {
                    return stack->findFunc(n);
                }
                return std::nullopt;
            }
            return parent->findFunc(n);
        }
        return std::nullopt;
    }
    return search->second;
}

std::optional<std::shared_ptr<Half_Type_Info>> Table::findType(std::string n, bool rec)
{
    auto search = types.find(n);
    if (search == types.end())
    {
        if (rec)
        {
            if (!parent)
            {
                if (stack)
                {
                    return stack->findType(n);
                }
                return std::nullopt;
            }
            return parent->findType(n, rec);
        }
        return std::nullopt;
    }

    // get actual type
    auto& type = search->second;

    return type;
}
