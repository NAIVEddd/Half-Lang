#pragma once
#include<string>
#include<variant>

struct Temp
{
    struct Label
    {
        std::string l;
        Label() = default;
        Label(const Label& o) : l(o.l) {}
        Label(std::string s) : l(s) {}
        bool operator<(const Label& r) const
        {
            return l < r.l;
        }
    };
    struct Variable
    {
        std::string v;
        Variable() = default;
        Variable(const Variable& o) : v(o.v) {}
        Variable(std::string s) : v(s) {}
    };
    std::variant<Label, Variable> name;

    Temp(Label l) : name(l) {}
    Temp(Variable v) : name(v) {}
    Temp(const Temp& o) : name(o.name) {}
    Temp& operator=(const Temp& o)
    {
        name = o.name;
        return *this;
    }
    static Label NewLabel()
    {
        static int count = 0;
        return NewLabel(count++);
    }
    static Label NewLabel(int c)
    {
        auto str = std::string("t_") + std::to_string(c);
        return Label(str);
    }
    static Label NewBlockLabel()
    {
        static int count = 0;
        auto str = std::string("Block_") + std::to_string(count++);
        return Label(str);
    }
    static Temp NewLabel(std::string s)
    {
        return Temp(Label(s));
    }
    static Temp NewVariable(std::string s)
    {
        return Temp(Variable(s));
    }
};