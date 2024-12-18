#include "CharParsers.h"

ParserResult<char> OneChar(ParserInput s)
{
	if (s.empty())
	{
		return std::nullopt;
	}
    auto iter = s.begin();
    auto end = s.end();
	if (*iter == '\\')
	{
		++iter;
		if (iter == end)
		{
			return std::nullopt;
		}
		if (*iter == 'n')
		{
			return std::make_pair('\n', ParserInput(iter + 1, end));
		}
		if (*iter == 't')
		{
			return std::make_pair('\t', ParserInput(iter + 1, end));
		}
		if (*iter == 'r')
		{
			return std::make_pair('\r', ParserInput(iter + 1, end));
		}
		if (*iter == '0')
		{
			return std::make_pair('\0', ParserInput(iter + 1, end));
		}
		if (*iter == '\\')
		{
			return std::make_pair('\\', ParserInput(iter + 1, end));
		}
		if (*iter == '\'')
		{
			return std::make_pair('\'', ParserInput(iter + 1, end));
		}
		if (*iter == '\"')
		{
			return std::make_pair('\"', ParserInput(iter + 1, end));
		}
		
        // todo: other escape characters
        // hex character?
        // octal character?

        // unicode character?
	}
	return std::make_pair(s[0], ParserInput(s.begin() + 1, s.end()));
}

Parser<char> OneChar(char c)
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

Parser<char> OneOf(Predicate pred)
{
	return [=](ParserInput s) -> ParserResult<char>
		{
			if (s.empty() || !pred(s[0]))
			{
				return std::nullopt;
			}
			return std::make_pair(s[0], ParserInput(s.begin() + 1, s.end()));
		};
}

Parser<char> OneOf(std::string_view chars)
{
	return [=](ParserInput s) -> ParserResult<char>
		{
			if (s.empty() || chars.find_first_of(s[0]) == -1)
			{
				return std::nullopt;
			}
			return std::make_pair(s[0], ParserInput(s.begin() + 1, s.end()));
		};
}

Parser<char> AnyChar()
{
	return OneOf(IsChar);
}

Parser<char> Satisfy(Predicate pred)
{
	return OneOf(pred);
}

Parser<char> AnyOf(std::string_view chars)
{
	return [=](ParserInput s) -> ParserResult<char>
		{
			if (s.empty() || chars.find_first_of(s[0]) == -1)
			{
				return std::nullopt;
			}
			return std::make_pair(s[0], ParserInput(s.begin() + 1, s.end()));
		};
}

Parser<char> AsciiLower()
{
	return OneOf(IsAsciiLower);
}

Parser<char> AsciiUpper()
{
	return OneOf(IsAsciiUpper);
}

Parser<char> OneDigit()
{
	return OneOf(IsDigit);
}

Parser<char> OneDigitOrChar()
{
	return OneOf(IsDigitOrChar) | OneChar('_');
}

Parser<char> OneHex()
{
	return OneOf(IsHex);
}

Parser<char> OneOctal()
{
	return OneOf(IsOctal);
}

Parser<char> OneTab()
{
	return OneChar('\t');
}

Parser<char> Newline()
{
	return Opt(OneChar('\r')) > OneChar('\n');
}

Parser<char> Space()
{
	Parser<char> s = AnyOf(" \t\r\n");
	return s;
}

Parser<char> Spaces()
{
	return ManyAs(Space(), ' ');
}

Parser<char> Spaces1()
{
	return Space() < Many(Space());
}

Parser<char> Eof()
{
	return [](ParserInput s) -> ParserResult<char>
		{
			if (s.empty())
			{
				return std::make_pair(' ', s);
			}
			return std::nullopt;
		};
}

bool starts_with(ParserInput s, std::string_view prefix)
{
    auto iter = s.begin();
    auto end = s.end();
	for (size_t i = 0; i < prefix.size(); i++)
	{
		if (iter == end || *iter != prefix[i])
		{
			return false;
		}
		++iter;
	}
    return true;
}

Parser<std::string> String(std::string str)
{
	return [=](ParserInput s) -> ParserResult<std::string>
		{
			if (starts_with(s, str))
			{
				return std::make_pair(str,
					ParserInput(s.begin() + str.size(), s.end()));
			}
			return std::nullopt;
		};
}

Parser<std::string> ManySatisfy(Predicate pred)
{
	return [=](ParserInput s) -> ParserResult<std::string>
		{
			auto chars = Many(OneOf(pred))(s);
			if (!chars)
			{
				return std::nullopt;
			}
			auto v = chars.value().first;
			std::string str = std::string(v.begin(), v.end());
			return std::make_pair(str, chars.value().second);
		};
}

Parser<std::string> ManySatisfy1(Predicate pred)
{
	return [=](ParserInput s) -> ParserResult<std::string>
		{
			auto r = ManySatisfy(pred)(s);
			if (!r)
			{
				return std::nullopt;
			}
			if (r.value().first.size() == 0)
			{
				return std::nullopt;
			}
			return r;
		};
}

Parser<std::string> ManySatisfy(Predicate pred1, Predicate pred2)
{
	return [=](ParserInput s) -> ParserResult<std::string>
		{
			std::vector<char> result;
			auto f = OneOf(pred1)(s);
			if (!f)
			{
				return std::nullopt;
			}
			result.push_back(f.value().first);
			auto chars = Many(OneOf(pred2))(f.value().second);
			if (!chars)
			{
				std::string str = std::string(1, result[0]);
				return std::make_pair(str, f.value().second);
			}
			auto v = chars.value().first;
			result.append_range(v);
			std::string str = std::string(result.begin(), result.end());
			return std::make_pair(str, chars.value().second);
		};
}
