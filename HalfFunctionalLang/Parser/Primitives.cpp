#include "Primitives.h"

//template<typename TResult, std::forward_iterator TIterator>
//inline Reply<TResult, TIterator>::Reply(TResult Result_, TIterator Iterator_)
//{
//}

//template<typename TResult, std::forward_iterator TIterator>
//Reply<TResult, TIterator>::Reply(ReplyStatus Status_, std::list<std::string>&& ErrorMessage_)
//{
//}

bool IsAnyChar(char c)
{
    return IsChar(c) || IsNewline(c) || IsDigit(c) || IsSpace(c);
}

bool IsChar(char c)
{
    return IsAsciiLower(c) || IsAsciiUpper(c);
}

bool IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

bool IsDigitOrChar(char c)
{
    return IsDigit(c) || IsChar(c);
}

bool IsHex(char c)
{
    return IsDigit(c) ||
        (c <= 'f' && c >= 'a') ||
        (c <= 'F' && c >= 'A');
}

bool IsOctal(char c)
{
    return c >= '0' && c <= '7';
}

bool IsNewline(char c)
{
    return c == '\n';
}

bool IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

bool IsAsciiLower(char c)
{
    return c <= 'z' && c >= 'a';
}

bool IsAsciiUpper(char c)
{
    return c <= 'Z' && c >= 'A';
}

bool IsPunctuation(char c)
{
    static std::string punc = "+-*/!=&|%^<>";
    return punc.find(c) != -1;
}

PKeyword::PKeyword(std::string Keyword_)
    : Keyword(Keyword_)
{
}

PKeyword::~PKeyword()
{
}

//Reply<std::string, std::string::iterator> PKeyword::Parse(Iterator<std::string::iterator>&& Iterator_)
//{
//    
//    return Reply<std::string, std::string::iterator>(std::string(), std::string::iterator());
//}
