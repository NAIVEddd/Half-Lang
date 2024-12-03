#pragma once
#include "Primitives.h"

ParserResult<char> OneChar(ParserInput s);

Parser<char> OneChar(char c);

Parser<char> OneOf(Predicate pred);

Parser<char> OneOf(std::string_view chars);

Parser<char> AnyChar();

Parser<char> Satisfy(Predicate pred);

Parser<char> AnyOf(std::string_view chars);

Parser<char> AsciiLower();

Parser<char> AsciiUpper();

Parser<char> OneDigit();

Parser<char> OneDigitOrChar();

Parser<char> OneHex();

Parser<char> OneOctal();

Parser<char> OneTab();

Parser<char> Newline();

Parser<char> Space();

Parser<char> Spaces();

Parser<char> Spaces1();

Parser<char> Eof();

Parser<std::string> String(std::string str);

template<typename A>
auto StringReturn(std::string str, A result)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			auto ResultStr = String(str)(s);
			if (!ResultStr)
			{
				return std::nullopt;
			}
			return std::make_pair(result, ResultStr.value().second);
		};
}

Parser<std::string> ManySatisfy(Predicate pred);

Parser<std::string> ManySatisfy1(Predicate pred);

Parser<std::string> ManySatisfy(Predicate pred1, Predicate pred2);


