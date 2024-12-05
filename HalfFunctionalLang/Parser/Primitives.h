#pragma once
#include "Parser.h"
#include <string>
#include <array>
#include <list>
#include <iterator>
#include <tuple>
#include <optional>
#include <concepts>
#include <functional>
#include <type_traits>


bool IsAnyChar(char c);
bool IsChar(char c);
bool IsDigit(char c);
bool IsDigitOrChar(char c);
bool IsHex(char c);
bool IsOctal(char c);
bool IsNewline(char c);
bool IsSpace(char c);
bool IsAsciiLower(char c);
bool IsAsciiUpper(char c);
bool IsPunctuation(char c);
using Predicate = std::function<bool(char)>;// auto(*)(char)->bool;


constexpr auto MakeCharParser(char c)
{
	return [=](ParserInput s) -> ParserResult<char>
		{
			if (s.empty() || c != s[0])
			{
				return std::nullopt;
			}
			return std::make_pair(s[0], ParserInput(s.begin() + 1, s.end()));
		};
}

template<typename PA, typename PB, typename Conv,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>,
	typename R = std::invoke_result_t<Conv, A, B>>
constexpr auto Combine(PA a, PB b, Conv conv)
{
	return [=](ParserInput s) -> ParserResult<R>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aResult = ResultA.value();
			auto ResultB = b(aResult.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bResult = ResultB.value();
			return std::make_pair(conv(aResult.first, bResult.first), bResult.second);
		};
}
template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto OnlyFirst(PA a, PB b)
{
	return Combine(a, b, [](auto l, auto r) {return l; });
}
template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto OnlySecond(PA a, PB b)
{
	return Combine(a, b, [](auto l, auto r) {return r; });
}

template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto operator<(PA a, PB b)
{
	return OnlyFirst(a, b);
}

template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto operator>(PA a, PB b)
{
	return OnlySecond(a, b);
}

template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto operator&(PA a, PB b)
{
	return [=](ParserInput s) -> ParserResult<std::pair<A, B>>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aValue = ResultA.value();
			auto ResultB = b(aValue.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bValue = ResultB.value();
			return std::make_pair(std::make_pair(aValue.first, bValue.first), bValue.second);
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto operator|(PA a, PA b)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return b(s);
			}
			return ResultA;
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto OneOrZero(PA a)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::make_pair(A{}, s);
			}
			return ResultA;
		};
}

template<typename Open, typename Close, typename R>
constexpr auto Between(Open a, Close b, R c)
{
	return a > c < b;
}

constexpr auto BetweenPair(char o, char c)
{
	return [=](ParserInput s) -> ParserResult<std::string>
		{
			if (s.empty() || s[0] != o)
			{
				return std::nullopt;
			}
			int pair = 1;
			size_t index = -1;
            size_t i = 1;
			std::vector<char> vec;
			for (auto iter = s.begin() + 1; iter != s.end(); ++iter, ++i)
			{
				if (*iter == o)
					pair += 1;
				if (*iter == c)
					pair -= 1;
				if (pair == 0)
				{
					index = i;
					break;
				}
				vec.push_back(*iter);
			}
			/*for (size_t i = 1; i < s.length(); i++)
			{
				if (s[i] == o)
					pair += 1;
				if (s[i] == c)
					pair -= 1;
				if (pair == 0)
				{
					index = i;
					break;
				}

				vec.push_back(s[i]);
			}*/

			if (index == -1)
			{
				return std::nullopt;
			}
			std::string str = std::string(vec.begin(), vec.end());
            return std::make_pair(str, ParserInput(s.begin() + index + 1, s.end()));
			//return std::make_pair(str, s.substr(index + 1));
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto Choice(const std::vector<PA> parsers)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			for (size_t i = 0; i < parsers.size(); i++)
			{
				auto Result = parsers[i](s);
				if (Result)
				{
					return Result;
				}
			}
			return std::nullopt;
		};
}

template<typename PA,
	typename A = ParserResult_t<PA>>
constexpr auto FollowedBy(PA a)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			auto Result = a(s);
			if (!Result)
			{
				return std::nullopt;
			}
			return std::make_pair(Result.value().first, s);
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto Opt(PA a)
{
	return [=](ParserInput s) -> ParserResult<std::optional<A>>
		{
			auto Result = a(s);
			if (!Result)
			{
				return std::make_pair(std::nullopt, s);
			}
			return Result;
		};
}

template<typename A>
constexpr auto Fail(std::string error_message)
{
	return [=](ParserInput s) -> ParserResult<A>
		{
			printf("%s\n", error_message.c_str());
			return std::nullopt;
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto Array(int n, PA a)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			std::vector<A> ResultArray;
			ResultArray.resize(n);
			ParserInput input = s;
			for (size_t i = 0; i < n; i++)
			{
				auto Result = a(input);
				if (!Result)
				{
					return std::nullopt;
				}
				ResultArray[i] = Result.value().first;
				input = Result.value().second;
			}
			return std::make_pair(ResultArray, input);
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto Many(PA a)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			std::vector<A> ResultVector;
			ParserInput input = s;
			while (!input.empty())
			{
				auto Result = a(input);
				if (!Result)
				{
					break;
				}
				input = Result.value().second;
				ResultVector.push_back(Result.value().first);
			}
			return std::make_pair(ResultVector, input);
		};
}

template<typename PA, typename B, typename A = ParserResult_t<PA>>
constexpr auto ManyAs(PA a, B b)
{
	return [=](ParserInput s) -> ParserResult<B>
		{
			auto ignore = Many(a)(s);
			return std::make_pair(b, ignore.value().second);
		};
}

template<typename PA, typename A = ParserResult_t<PA>>
constexpr auto Many1(PA a)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			std::vector<A> ResultVector;
			auto ResultA = a(s);
			if (ResultA)
			{
				ResultVector.push_back(ResultA.value().first);
				auto vector = Many(a)(ResultA.value().second);
				if (vector)
				{
					ResultVector.append_range(vector.value().first);
					return std::make_pair(ResultVector, vector.value().second);
				}
				return std::make_pair(ResultVector, ResultA.value().second);
			}
			return std::nullopt;
		};
}

template<typename A, typename R>
using Converter = std::function<R(A)>;// auto (*)(A)->R;
template<typename A, typename B, typename R>
using Converter2 = std::function<R(A, B)>; // auto (*)(A, B)->R;
template<typename A, typename B, typename C, typename R>
using Converter3 = std::function<R(A, B, C)>; // auto (*)(A, B, C)->R;
template<typename A, typename B, typename C, typename D, typename R>
using Converter4 = std::function<R(A, B, C, D)>; // auto (*)(A, B, C, D)->R;
template<typename A, typename B, typename C, typename D, typename E, typename R>
using Converter5 = std::function<R(A, B, C, D, E)>; // auto (*)(A, B, C, D, E)->R;

template<typename PA, typename R,
	typename A = ParserResult_t<PA>>
constexpr auto Pipe(PA a, Converter<A, R> converter)
{
	return [=](ParserInput s) -> ParserResult<R>
		{
			auto ResultA = a(s);
			if (ResultA.has_value())
			{
				auto value = ResultA.value();
				return std::make_pair(converter(value.first), value.second);
			}
			return std::nullopt;
		};
}
template<typename PA, typename PB, typename R,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto Pipe2(PA a, PB b, Converter2<A, B, R> converter)
{
	return [=](ParserInput s) ->ParserResult<R>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aValue = ResultA.value();
			auto ResultB = b(aValue.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bValue = ResultB.value();
			return std::make_pair(converter(aValue.first, bValue.first), bValue.second);
		};
}
template<typename PA, typename PB, typename PC, typename R,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>,
	typename C = ParserResult_t<PC>>
constexpr auto Pipe3(PA a, PB b, PC c, Converter3<A, B, C, R> converter)
{
	return [=](ParserInput s) -> ParserResult<R>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aValue = ResultA.value();
			auto ResultB = b(aValue.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bValue = ResultB.value();
			auto ResultC = c(bValue.second);
			if (!ResultC)
			{
				return std::nullopt;
			}
			auto cValue = ResultC.value();
			return std::make_pair(
				converter(aValue.first, bValue.first, cValue.first),
				cValue.second);
		};
}
template<typename PA, typename PB, typename PC, typename PD, typename R,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>,
	typename C = ParserResult_t<PC>,
	typename D = ParserResult_t<PD>>
constexpr auto Pipe4(PA a, PB b, PC c, PD d, Converter4<A, B, C, D, R> converter)
{
	return [=](ParserInput s) -> ParserResult<R>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aValue = ResultA.value();
			auto ResultB = b(aValue.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bValue = ResultB.value();
			auto ResultC = c(bValue.second);
			if (!ResultC)
			{
				return std::nullopt;
			}
			auto cValue = ResultC.value();
			auto ResultD = d(cValue.second);
			if (!ResultD)
			{
				return std::nullopt;
			}
			auto dValue = ResultD.value();
			return std::make_pair(converter(aValue.first, bValue.first, cValue.first, dValue.first), dValue.second);
		};
}
template<typename PA, typename PB, typename PC, typename PD, typename PE, typename R,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>,
	typename C = ParserResult_t<PC>,
	typename D = ParserResult_t<PD>,
	typename E = ParserResult_t<PE>>
constexpr auto Pipe5(PA a, PB b, PC c, PD d, PE e, Converter5<A, B, C, D, E, R> converter)
{
	return [=](ParserInput s) -> ParserResult<R>
		{
			auto ResultA = a(s);
			if (!ResultA)
			{
				return std::nullopt;
			}
			auto aValue = ResultA.value();
			auto ResultB = b(aValue.second);
			if (!ResultB)
			{
				return std::nullopt;
			}
			auto bValue = ResultB.value();
			auto ResultC = c(bValue.second);
			if (!ResultC)
			{
				return std::nullopt;
			}
			auto cValue = ResultC.value();
			auto ResultD = d(cValue.second);
			if (!ResultD)
			{
				return std::nullopt;
			}
			auto dValue = ResultD.value();
			auto ResultE = e(dValue.second);
			if (!ResultE)
			{
				return std::nullopt;
			}
			auto eValue = ResultE.value();
			return std::make_pair(converter(aValue.first, bValue.first, cValue.first, dValue.first, eValue.first), eValue.second);
		};
}

template<typename PA, typename R,
	typename A = ParserResult_t<PA>>
constexpr auto operator>>(PA a, Converter<A, R> comv)
{
	return Pipe(a, comv);
}

// "a, b"
// "a,  "
// "a   "
template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
	constexpr auto SepBy(PA a, PB sep)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			Parser<A> b = sep > a;
			Parser<std::vector<A>> many = Many(b);
			Converter2<A, std::vector<A>, std::vector<A>> lambda =
				[](A l, std::vector<A> r)
				{
					std::vector<A> Result;
					Result.push_back(l);
					Result.append_range(r);
					return Result;
				};
			auto parser = Pipe2(a, many, lambda);
			return parser(s);
		};
}

template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto SepEndBy(PA a, PB sep)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			Parser<A> b = sep > a;
			Parser<std::vector<A>> many = Many(b);
			Converter2<A, std::vector<A>, std::vector<A>> lambda =
				[](A l, std::vector<A> r)
				{
					std::vector<A> Result;
					Result.push_back(l);
					Result.append_range(r);
					return Result;
				};
			return (Pipe2(a, many, lambda) < Opt(sep))(s);
		};
}

template<typename PA, typename PB,
	typename A = ParserResult_t<PA>,
	typename B = ParserResult_t<PB>>
constexpr auto ManyTill(PA a, PB end)
{
	return [=](ParserInput s) -> ParserResult<std::vector<A>>
		{
			Parser<B> followed = FollowedBy(end);
			ParserInput input = s;
			std::vector<A> Result;
			for (auto ResultEnd = followed(input); !ResultEnd; ResultEnd = followed(input))
			{
				auto ResultA = a(input);
				if (!ResultA)
				{
					return std::nullopt;
				}
				Result.push_back(ResultA.value().first);
				input = ResultA.value().second;
			}
			auto ResultEnd = end(input);
			if (!ResultEnd)
			{
				return std::nullopt;
			}
			return std::make_pair(Result, ResultEnd.value().second);
		};
}


enum class ReplyStatus
{
	Ok,
	Error,
};

template<std::forward_iterator TIterator>
struct Iterator
{
	ParserPos Position;
	TIterator StdIterator;
};

template<typename TResult, std::forward_iterator TIterator>
struct Reply
{
	Reply(TResult Result_, TIterator Iterator_);
	Reply(ReplyStatus Status_, std::list<std::string>&& ErrorMessage_);

	TResult Result;
	Iterator<TIterator> Iterator;
	ReplyStatus Status;
	std::list<std::string> ErrorMessage;
};




class PKeyword
{
public:
	PKeyword(std::string Keyword_);
	~PKeyword();

	Reply<std::string, std::string::iterator> Parse(Iterator<std::string::iterator>&& Iterator_);

	std::string Keyword;
};

