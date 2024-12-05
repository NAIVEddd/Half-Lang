#pragma once
#include<iostream>
#include<functional>
#include<tuple>
#include<optional>
#include<memory>


struct ParserPos
{
    int line;
    int column;
    ParserPos(int l, int c) : line(l), column(c) {}
    bool operator==(const ParserPos& other) const
    {
        return line == other.line && column == other.column;
    }
    bool operator!=(const ParserPos& other) const
    {
        return !(*this == other);
    }
    ParserPos& operator=(const ParserPos& other)
    {
        if (this != &other) {
            line = other.line;
            column = other.column;
        }
        return *this;
    }
};
struct ParserLine
{
    int indent;
    ParserPos position;
    const std::string_view line_text;
};

struct _ParserInput
{
    struct iterator
    {
        const _ParserInput& data;
        ParserPos pos;
        iterator(const _ParserInput& d, ParserPos p) : data(d), pos(p) {}
        char operator*() const
        {
            return data[pos];
        }
        iterator& operator++();
        iterator operator++(int);
        iterator& operator+(int i);
        iterator& operator=(const iterator& other);

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
    };
    struct line_iterator
    {
        const _ParserInput& data;
        int line;
        line_iterator(const _ParserInput& d, int l) : data(d), line(l) {}
        ParserLine operator*() const;
        line_iterator& operator++();
        line_iterator operator++(int);
        line_iterator& operator+(int i);
        line_iterator& operator=(const line_iterator& other);

        bool operator==(const line_iterator& other) const;
        bool operator!=(const line_iterator& other) const;
    };
    using const_iterator = const iterator;
    using const_line_iterator = const line_iterator;

    std::shared_ptr<std::vector<std::string>> lines_raw;
    std::vector<int> line_indents;
    //std::shared_ptr<std::vector<ParserLine>> lines;
    ParserPos current_pos;

    _ParserInput(std::string s);
    template<int N>
    _ParserInput(const char(&s)[N]) : _ParserInput(std::string(s)) {}
    _ParserInput(iterator begin, iterator end);
    _ParserInput(line_iterator begin, line_iterator end);
    _ParserInput(const _ParserInput& other);
    _ParserInput& operator=(const _ParserInput& other);
    iterator begin()
    {
        return iterator(*this, current_pos);
    }
    iterator end()
    {
        return iterator(*this, ParserPos(lines_raw->size(), 0));
    }
    const_iterator cbegin() const
    {
        return iterator(*this, current_pos);
    }
    const_iterator cend() const
    {
        return iterator(*this, ParserPos(lines_raw->size(), 0));
    }
    line_iterator lbegin()
    {
        return line_iterator(*this, current_pos.line);
    }
    line_iterator lend()
    {
        return line_iterator(*this, lines_raw->size());
    }
    const_line_iterator clbegin() const
    {
        return line_iterator(*this, current_pos.line);
    }
    const_line_iterator clend() const
    {
        return line_iterator(*this, lines_raw->size());
    }
    // operator std::string() const;
    char operator[](int i) const;
    char operator[](ParserPos p) const;
    bool empty() const;
    //size_t size() const;
};

// Parser a :: String -> Maybe (a, String)
using ParserInput = struct _ParserInput;// std::string_view;
template <typename T>
using ParserResult = std::optional<std::pair<T, ParserInput>>;
template<typename P>
using OptPairParse_t = std::invoke_result_t<P, ParserInput>;
template<typename T>
using PairParse_t = typename OptPairParse_t<T>::value_type;
template<typename T>
using ParserResult_t = typename PairParse_t<T>::first_type;
template <typename T>
using Parser = std::function<ParserResult<T>(ParserInput)>;

enum class ParserState
{
	Success,
	Failure
};

//template<typename Result, typename UserState>
//struct ParserResult
//{
//	ParserState ParserState;
//	Result Result;
//	UserState UserState;
//};
