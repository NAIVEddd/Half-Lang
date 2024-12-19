#include "Symbol.h"

Table::~Table()
{
    if (stack)
    {
        stack->Release(values.size() * 4);
    }
}

void Table::insert(Symbol& s)
{
    // value at offset 0 is stack frame pointer
    // so we start at offset 4
    s.offset = stack->Alloc(4);
    values.insert({ s.name, s });
}
