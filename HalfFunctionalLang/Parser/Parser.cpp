#include"Parser.h"

_ParserInput::_ParserInput(std::string s)
    : current_pos(0, 0)
    , lines_raw(std::make_shared<std::vector<std::string>>())
    //, lines(std::make_shared<std::vector<ParserLine>>())
{
    std::string line;
    int indent = 0;
    bool is_indent = true;
    bool is_skip = false;
    int current_line = 0;
    for (auto c : s)
    {
        if (is_indent)
        {
            switch (c)
            {
            case ' ':
                indent++;
                break;
            case '\t':
                indent += 4;
                break;
            case '\r':
                is_skip = true;
                break;
            default:
                is_indent = false;
                break;
            }
        }
        if (is_skip)
        {
            is_skip = false;
            continue;
        }
        if (c == '\n')
        {
            line.push_back(c);
            lines_raw->push_back(line);
            std::string_view line_view(lines_raw->back());
            line_indents.push_back(indent);
            //lines->push_back(ParserLine{ indent, ParserPos(current_line, indent), line_view });
            line.clear();
            indent = 0;
            current_line++;
            is_indent = true;
        }
        else if (c == '\t')
        {
            line.append(4, ' ');
        }
        else
        {
            line.push_back(c);
        }
    }
    if (!line.empty())
    {
        lines_raw->push_back(line);
        std::string_view line_view(lines_raw->back());
        line_indents.push_back(indent);
        //lines->push_back(ParserLine{ indent, ParserPos(current_line, indent), line_view });
    }
}

_ParserInput::_ParserInput(iterator begin, iterator end)
    : lines_raw(begin.data.lines_raw)
    , line_indents(begin.data.line_indents)
    //, lines(begin.data.lines)
    , current_pos(begin.pos)
{
    _ASSERT(end.pos.line == (*end.data.lines_raw).size());
    _ASSERT(end.pos.column == 0);
}

_ParserInput::_ParserInput(line_iterator begin, line_iterator end)
    : lines_raw(std::make_shared<std::vector<std::string>>())
    //, line_indents()
    //, lines(std::make_shared<std::vector<ParserLine>>())
    , current_pos(0, 0)
{
    _ASSERT(end.line > begin.line);
    _ASSERT(end.data.lines_raw == begin.data.lines_raw);
    //_ASSERT(begin.data.lines->size() == begin.data.lines_raw->size());
    _ASSERT(end.line <= begin.data.lines_raw->size());

    // copy lines from begin to end
    auto& o_lines_raw = *begin.data.lines_raw;
    auto o_lines = begin.data.line_indents;
    //auto& o_lines = *begin.data.lines;
    lines_raw->reserve(end.line - begin.line);
    line_indents.reserve(end.line - begin.line);
    //lines->reserve(end.line - begin.line);
    for (int i = begin.line; i < end.line; i++)
    {
        lines_raw->push_back(o_lines_raw[i]);
        std::string_view line_view(lines_raw->back());
        line_indents.push_back(o_lines[i]);
        //lines->push_back(ParserLine{ o_lines[i].indent, ParserPos(i - begin.line, 0), line_view});
    }
}

_ParserInput::_ParserInput(const _ParserInput& other)
    : lines_raw(other.lines_raw)
    , line_indents(other.line_indents)
    //, lines(other.lines)
    , current_pos(other.current_pos)
{
}

_ParserInput& _ParserInput::operator=(const _ParserInput& other)
{
    if (this != &other)
    {
        lines_raw = other.lines_raw;
        line_indents = other.line_indents;
        //lines = other.lines;
        current_pos = other.current_pos;
    }
    return *this;
}

char _ParserInput::operator[](int i) const
{
    _ASSERT(i >= 0);
    i += current_pos.column;

    auto& lines_raw = *this->lines_raw;
    int line_index = current_pos.line;
    while ((line_index < lines_raw.size()) && (i >= lines_raw[line_index].size()))
    {
        i -= lines_raw[line_index].size();
        ++line_index;
    }
    if (line_index >= lines_raw.size())
    {
        return '\0';
    }
    _ASSERT(i < lines_raw[line_index].size());

    return lines_raw[line_index][i];
    //auto& line = (*lines)[line_index];
    //return i < line.indent? ' ' : line.line_text[i];
}

char _ParserInput::operator[](ParserPos p) const
{
    _ASSERT(p.line >= 0);
    _ASSERT(p.column >= 0);
    auto input(*this);
    input.current_pos = p;
    return input[0];
}

bool _ParserInput::empty() const
{
    return current_pos.line >= (*lines_raw).size();
}

//size_t _ParserInput::size() const
//{
//
//}

_ParserInput::iterator& _ParserInput::iterator::operator++()
{
    auto& raw = *data.lines_raw;
    _ASSERT(pos.line < raw.size());

    pos.column++;
    if (raw[pos.line].size() == pos.column)
    {
        pos.line++;
        pos.column = 0;
    }

    return *this;
}

_ParserInput::iterator _ParserInput::iterator::operator++(int)
{
    auto temp = *this;
    ++(*this);
    return temp;
}

_ParserInput::iterator& _ParserInput::iterator::operator+(int i)
{
    while (i > 0)
    {
        ++(*this);
        --i;
    }
    return *this;
}

_ParserInput::iterator& _ParserInput::iterator::operator=(const iterator& other)
{
    _ASSERT(&data == &other.data);
    if (this != &other) {
        pos = other.pos;
    }
    return *this;
}

bool _ParserInput::iterator::operator==(const iterator& other) const
{
    _ASSERT(&data == &other.data);
    return pos == other.pos;
}

bool _ParserInput::iterator::operator!=(const iterator& other) const
{
    return !(*this == other);
}

ParserLine _ParserInput::line_iterator::operator*() const
{
    /*_ASSERT(line < data.lines->size());
    return data.lines->at(line);*/
    _ASSERT(line < data.lines_raw->size());
    return ParserLine{ data.line_indents[line], ParserPos(line, 0), std::string_view(data.lines_raw->at(line)) };
}

_ParserInput::line_iterator& _ParserInput::line_iterator::operator++()
{
    ++line;
    return *this;
}

_ParserInput::line_iterator _ParserInput::line_iterator::operator++(int)
{
    auto temp = *this;
    ++(*this);
    return temp;
}

_ParserInput::line_iterator& _ParserInput::line_iterator::operator+(int i)
{
    line += i;
    return *this;
}

_ParserInput::line_iterator& _ParserInput::line_iterator::operator=(const line_iterator& other)
{
    _ASSERT(&data == &other.data);
    if (this != &other) {
        line = other.line;
    }
    return *this;
}

bool _ParserInput::line_iterator::operator==(const line_iterator& other) const
{
    _ASSERT(&data == &other.data);
    return line == other.line;
}

bool _ParserInput::line_iterator::operator!=(const line_iterator& other) const
{
    return !(*this == other);
}
