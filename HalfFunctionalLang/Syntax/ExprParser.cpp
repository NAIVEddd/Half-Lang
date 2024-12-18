#include"ExprParser.h"

ParserResult<Half_Value> pchar(ParserInput s)
{
	char c = '_';

	auto punc = OneChar('\'');
	auto c1 = punc(s);
	if (!c1)
	{
		return std::nullopt;
	}
	auto c2 = OneChar(c1.value().second);
	if (!c2)
	{
		return std::nullopt;
	}
	c = c2.value().first;
	s = c2.value().second;

	auto c3 = punc(s);
	if (!c3)
	{
		return std::nullopt;
	}

	return std::make_pair(Half_Value::ValueT(c), c3.value().second);
}

ParserResult<Half_Value> pstring(ParserInput s)
{
	auto punc = OneChar('\"');
	auto c1 = punc(s);
	if (!c1)
	{
		return std::nullopt;
	}
	s = c1.value().second;

	std::vector<char> cs;
	while (c1 = OneChar(s), c1)
	{
		s = c1.value().second;
		if (c1.value().first == '\"')
		{
			break;
		}
		cs.push_back(c1.value().first);
	}
	if (!c1)
	{
		return std::nullopt;
	}

	std::string str = std::string(cs.begin(), cs.end());
	return std::make_pair(Half_Value::ValueT(str), s);
}

ParserResult<Half_Value> pint(ParserInput s)
{
	auto sign = OneOf("+-");
	auto signal = sign(s);
	std::vector<char> str;
	if (signal)
	{
		str.push_back(signal.value().first);
		s = signal.value().second;
	}
	auto istr = Many1(OneDigit())(s);
	if (!istr)
	{
		return std::nullopt;
	}
	str.resize(istr.value().first.size() + (signal ? 1 : 0));
	std::copy(istr.value().first.begin(), istr.value().first.end(),
		&str[signal ? 1 : 0]);
	auto i = std::stoi(std::string(str.begin(), str.end()));
	return std::make_pair(Half_Value::ValueT(i), istr.value().second);
}

ParserResult<Half_Value> pfloat(ParserInput s)
{
	auto sign = OneOf("+-");
	auto signal = sign(s);
	auto point = OneChar('.');
	auto digits = Many1(OneDigit());
	std::vector<char> str;
	if (signal)
	{
		str.push_back(signal.value().first);
		s = signal.value().second;
	}
	auto p1 = digits(s);
	if (!p1)
	{
		return std::nullopt;
	}
	str.append_range(p1.value().first);
	s = p1.value().second;

	auto p2 = point(s);
	if (!p2)
	{
		return std::nullopt;
	}
	str.push_back('.');
	s = p2.value().second;

	auto p3 = digits(s);
	if (!p3)
	{
		return std::nullopt;
	}
	str.append_range(p3.value().first);
	s = p3.value().second;

	float f = std::stof(std::string(str.begin(), str.end()));
	return std::make_pair(Half_Value(f), s);
}

ParserResult<Half_Value> pvalue(ParserInput s)
{
	auto pv = Choice(std::vector<Parser<Half_Value>>{ pchar, pstring, pfloat, pint});

	return pv(s);
}

ParserResult<std::string> pvariablename(ParserInput s)
{
	auto head = AnyChar();
	auto tail = Many(OneDigitOrChar());
	Converter2<char, std::vector<char>, std::string> tostring =
		[](char c, std::vector<char>&& cs)
		{
			std::string str = std::string(1, c) + std::string(cs.begin(), cs.end());
			return str;
		};
	auto pname = Pipe2(head, tail, tostring);
	auto n = pname(s);
	if (!n || Keyword.contains(n.value().first))
	{
		return std::nullopt;
	}
	return n;
}

ParserResult<std::string> pfieldname(ParserInput s)
{
	auto punc = OneChar('.');
	auto name = punc > pvariablename;
	return name(s);
}

ParserResult<std::string> psubscriptstring(ParserInput s)
{
	auto pstr = BetweenPair('[', ']');
	return pstr(s);
}

ParserResult<Half_Var> psimplevar(ParserInput s)
{
	auto name = pvariablename(s);
	if (!name)
	{
		return std::nullopt;
	}
	auto res = Half_Var::SimpleVar(name.value().first);
	return std::make_pair(res, name.value().second);
}

ParserResult<Half_Var> pvar(ParserInput s)
{
	auto svar = psimplevar(s);
	if (!svar)
	{
		return std::nullopt;
	}
	s = svar.value().second;
	Half_Var var(std::move(svar.value().first));

	while (!s.empty())
	{
		auto name = pfieldname(s);
		if (name)
		{
			s = name.value().second;
			auto n = name.value().first;
			auto ptr = std::make_shared<Half_Var>(var);
			var = std::move(Half_Var::FieldVar(ptr, n));
			continue;
		}
		auto subscrupt = psubscriptstring(s);
		if (subscrupt)
		{
			s = subscrupt.value().second;
			auto prog = subscrupt.value().first;
			auto expr = pexpr(prog);
			if (!expr)
			{
				// TODO:: error message
				return std::nullopt;
			}
			auto vptr = std::make_shared<Half_Var>(var);
			auto iptr = std::make_shared<Half_Expr>(expr.value().first);
			var = std::move(Half_Var::SubscriptVar(vptr, iptr));
			continue;
		}
		break;
	}

	return std::make_pair(var, s);
}

ParserResult<Half_Funcall> pfuncall(ParserInput s)
{
	auto space = Spaces();
	auto name = OnlyFirst(space > pvariablename, space)(s);
	if (!name)
	{
		return std::nullopt;
	}
	auto expr = BetweenPair('(', ')')(name.value().second);
	if (!expr)
	{
		return std::nullopt;
	}
	Parser<std::vector<Half_Expr>> p1 = SepBy(Between(space, space, pexpr), OneChar(','));
	Parser<std::vector<Half_Expr>> p2 = ManyAs(space, std::vector<Half_Expr>());
	auto pargs = OnlyFirst(p1 | p2, Eof());
	auto args = pargs(expr.value().first);
	if (!args)
	{
		// TODO: error message
		return std::nullopt;
	}
	auto funcall = Half_Funcall(name.value().first, std::move(args.value().first));
	return std::make_pair(funcall, expr.value().second);
}

Converter2<Half_Op::Half_OpExpr, Half_Op::Half_OpExpr, Half_Op::Half_OpExpr> GetOpConverter(const std::string& op)
{
    return [=](Half_Op::Half_OpExpr l, Half_Op::Half_OpExpr r)
        {
            return Half_Op::Half_OpExpr(Half_Op(op, std::move(l), std::move(r)));
        };
}

Parser<Half_Op> GetOpParser()
{
	using A = Half_Op::Half_OpExpr;
	auto space = Spaces();
	Converter<Half_Value, A> inttoopexpr =
		[](Half_Value i) {return A(i); };
	Converter<Half_Funcall, A> functiontoopexpr =
		[](Half_Funcall f) {return A(f); };
	Converter<Half_Var, A> vartoopexpr =
		[](Half_Var v) {return A(v); };
	Parser<A> parsevalue = Pipe(pvalue, inttoopexpr);
	Parser<A> parsefuncall = Pipe(pfuncall, functiontoopexpr);
	Parser<A> parsevar = Pipe(pvar, vartoopexpr);

	auto choice = Choice(std::vector{ parsefuncall, parsevalue, parsevar });
	auto popexpr = Between(space, space, choice);
	static auto parser = OperatorParser(popexpr);

    //auto op_and = InfixOperator("and", 1, GetOpConverter("and"));
    auto op_and = InfixOperator("&&", 1, GetOpConverter("&&"));
    //auto op_or = InfixOperator("or", 1, GetOpConverter("or"));
    auto op_or = InfixOperator("||", 1, GetOpConverter("||"));
    auto op_bit_and = InfixOperator("&", 2, GetOpConverter("&"));
    auto op_bit_or = InfixOperator("|", 2, GetOpConverter("|"));
    auto op_bit_xor = InfixOperator("^", 2, GetOpConverter("^"));
    auto op_eq = InfixOperator("==", 3, GetOpConverter("=="));
    auto op_not_eq = InfixOperator("!=", 3, GetOpConverter("!="));
    auto op_less = InfixOperator("<", 4, GetOpConverter("<"));
    auto op_less_eq = InfixOperator("<=", 4, GetOpConverter("<="));
    auto op_greater = InfixOperator(">", 4, GetOpConverter(">"));
    auto op_greater_eq = InfixOperator(">=", 4, GetOpConverter(">="));
    auto op_shift_l = InfixOperator("<<", 5, GetOpConverter("<<"));
    auto op_shift_r = InfixOperator(">>", 5, GetOpConverter(">>"));
	auto op_plus = InfixOperator("+", 6, GetOpConverter("+"));
	auto op_minus = InfixOperator("-", 6, GetOpConverter("-"));
	auto op_multy = InfixOperator("*", 7, GetOpConverter("*"));
	auto op_devide = InfixOperator("/", 7, GetOpConverter("/"));
    auto op_mod = InfixOperator("%", 7, GetOpConverter("%"));

    parser.AddOperator(op_and);
    parser.AddOperator(op_or);
    parser.AddOperator(op_bit_and);
    parser.AddOperator(op_bit_or);
    parser.AddOperator(op_bit_xor);
    parser.AddOperator(op_eq);
    parser.AddOperator(op_not_eq);
    parser.AddOperator(op_less);
    parser.AddOperator(op_less_eq);
    parser.AddOperator(op_greater);
    parser.AddOperator(op_greater_eq);
    parser.AddOperator(op_shift_l);
    parser.AddOperator(op_shift_r);
	parser.AddOperator(op_plus);
	parser.AddOperator(op_minus);
	parser.AddOperator(op_multy);
	parser.AddOperator(op_devide);
    parser.AddOperator(op_mod);

	Converter<A, Half_Op> conv =
		[](A a)
		{
			try
			{
				return std::get<Half_Op>(a);
			}
			catch (const std::bad_variant_access&)
			{
				printf("Parse op failed!\n");
				return Half_Op();
			}
		};
	auto p = Pipe<Parser<A>, Half_Op>(parser.GetParser(), conv);
	return p;
}

ParserResult<Half_Op> pop(ParserInput s)
{
	static auto opparser = GetOpParser();

	return opparser(s);
}

Parser<Half_Assign> GetAssignParser()
{
	return [](ParserInput s)
		{
			auto space = Spaces();
			auto parsevar = Between(space, space, pvar);
			auto parseexpr = Between(space, space, pexpr);
			auto equalsign = OneChar('=');
			auto parseequal = Between(space, space, equalsign);
			Converter2<Half_Var, Half_Expr, Half_Assign> conv =
				[](Half_Var var, Half_Expr expr)
				{
					return Half_Assign(var, expr);
				};
			auto parseassign = Pipe2(parsevar < parseequal, parseexpr, conv);
			return parseassign(s);
		};
}

ParserResult<Half_Assign> passign(ParserInput s)
{
	static auto parseassign = GetAssignParser();
	return parseassign(s);
}

Parser<Half_If> GetIfParser()
{
	return [](ParserInput s) -> ParserResult<Half_If>
		{
			auto space = Spaces();
			auto pif = Between(space, space, String("if"));
			auto _if = pif(s);
			if (!_if)
			{
				return std::nullopt;
			}
			s = _if.value().second;

			auto pthen = Between(space, space, String("then"));
			auto pelse = Between(space, space, String("else"));
			auto pend = Between(space, space, String("end"));
			auto pcond = pexpr;
			auto ptrue = pthen > pmanyexpr;
			auto pfalse = (pelse > pmanyexpr) < pend;
			Converter3<Half_Expr, Half_Expr, Half_Expr, Half_If> conv =
				[](Half_Expr c, Half_Expr t, Half_Expr f)
				{
					return Half_If(c, t, f);
				};
			auto parse = Pipe3(pcond, ptrue, pfalse, conv);
			auto res = parse(s);
			if (!res)
			{
				printf("Parse if failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			return res;
		};
}

ParserResult<Half_If> pif(ParserInput s)
{
	static auto parser = GetIfParser();
	return parser(s);
}

Parser<Half_FuncDecl> GetFuncDeclParser()
{
	return [](ParserInput s) -> ParserResult<Half_FuncDecl>
		{
			auto space = Spaces();
			auto keyfunc = Between(space, space, String("function"));
			auto _function = keyfunc(s);
			if (!_function)
			{
				return std::nullopt;
			}
			s = _function.value().second;

			auto keysep = Between(space, space, OneChar(','));
			auto keyassign = Between(space, space, OneChar('='));
			auto keyend = Between(space, space, String("end"));
			auto keyopen = Between(space, space, OneChar('('));
			auto keyclose = Between(space, space, OneChar(')'));
			auto keycolon = Between(space, space, OneChar(':'));
			auto pname = Between(space, space, pvariablename);
			Converter2<std::string, std::string, Half_FuncDecl::TypeField> tofield =
				[](std::string a, std::string b)
				{
					Half_FuncDecl::TypeField field;
					field.var_name = a;
					field.type_name = b;
					return field;
				};
			Converter<std::vector<std::string>, std::vector<Half_FuncDecl::TypeField>> tofield_array =
				[](std::vector<std::string> vs)
				{
					std::vector<Half_FuncDecl::TypeField> fields;
					for (auto& v : vs)
					{
						fields.push_back(Half_FuncDecl::TypeField(v, ""));
					}
					return fields;
				};
			auto ppair = Pipe2(pname, pname, tofield);

			// parameters like '(a int, b char)'
			Parser<std::vector<Half_FuncDecl::TypeField>> pairs = Between(keyopen, keyclose, OneOrZero(SepBy(ppair, keysep)));
			// parameters like '(a int) (b char)
			auto pair = Between(keyopen, keyclose, ppair);
			Parser<std::vector<Half_FuncDecl::TypeField>> pair_array = Many1(pair);
			// parameters like 'a b c'
			auto pnames_array = Many1(pname);
			Parser<std::vector<Half_FuncDecl::TypeField>> deftype_array = Pipe(pnames_array, tofield_array);

			auto oneof_pairs = Choice(std::vector{ pairs, pair_array, deftype_array });

			// parse function return type, default type is empty string
			Converter<std::optional<std::string>, std::string> defaulttype =
				[](std::optional<std::string> s)
				{
					if (s)
					{
						return s.value();
					}
					return std::string("");
				};
			auto prtype = Pipe(Opt(keycolon > pname), defaulttype);

			auto pfuncname = pname;
			auto rname = pfuncname(s);
			if (!rname)
			{
				printf("Parse function name failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			s = rname.value().second;
			auto rpairs = oneof_pairs(s);
			if (!rpairs)
			{
				printf("Parse function parameters failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			s = rpairs.value().second;

			auto parse_type = (prtype < keyassign);
			auto rtype = parse_type(s);
			s = rtype.value().second;

			auto pbody = PipeExpr(ManyTill(pexpr, keyend));
			auto rbody = pbody(s);
			if (!rbody)
			{
				printf("Parse function body failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			s = rbody.value().second;

			return std::make_pair(
				Half_FuncDecl(
					rname.value().first,
					rpairs.value().first,
					rtype.value().first,
					rbody.value().first), s);
		};
}

ParserResult<Half_FuncDecl> pfuncdecl(ParserInput s)
{
	static auto parser = GetFuncDeclParser();
	return parser(s);
}

ParserResult<Def_Type> pdeftype(ParserInput s)
{
	auto space = Spaces();
	auto keytype = Between(space, space, String("type"));
	auto keyequal = Between(space, space, OneChar('='));
	auto keyopen = Between(space, space, OneChar('{'));
	auto keyclose = Between(space, space, OneChar('}'));
	auto keyinter = Between(space, space, OneChar(':'));
	auto keysep = Between(space, space, OneChar(','));
	auto pname = Between(space, space, pvariablename);
	Converter3<std::string, char, std::string, Def_Type::StructType::TypePair> totypepair =
		[](std::string n, char _, std::string t)
		{
			return Def_Type::StructType::TypePair(n, t);
		};
	auto ppair = Pipe3(pname, keyinter, pname, totypepair);
	auto pmanypair = SepBy(ppair, keysep);
	Converter3<char, std::vector<Def_Type::StructType::TypePair>, char, std::vector<Def_Type::StructType::TypePair>> tostructtype =
		[](char, std::vector<Def_Type::StructType::TypePair> tys, char)
		{
			return tys;
		};
	auto pbody = Pipe3(keyopen, pmanypair, keyclose, tostructtype);

	auto rtype = keytype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	s = rtype.value().second;

	Converter2<std::string, std::vector<Def_Type::StructType::TypePair>, Def_Type> totype =
		[](std::string n, std::vector<Def_Type::StructType::TypePair> ts)
		{
			return Def_Type(Def_Type::StructType(n, ts));
		};
	auto ptype = Pipe2((pname < keyequal), pbody, totype);
	auto rstruct = ptype(s);
	if (!rstruct)
	{
		return std::nullopt;
	}
	s = rstruct.value().second;

	return std::make_pair(rstruct.value().first, s);
}

Parser<Half_Let> GetLetParser()
{
	return [](ParserInput s) -> ParserResult<Half_Let>
		{
			auto space = Spaces();
			auto clet = Between(space, space, String("let"));
			auto _let = clet(s);
			if (!_let)
			{
				return std::nullopt;
			}

			auto cassign = Between(space, space, passign);
			Converter<Half_Assign, Half_Let> conv =
				[](Half_Assign a)
				{
					return Half_Let(a);
				};
			auto parse = Pipe(OnlySecond(clet, cassign), conv);
			auto res = parse(s);
			if (!res)
			{
				printf("Parse let failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			return res;
		};
}

ParserResult<Half_Let> plet(ParserInput s)
{
	static auto parse = GetLetParser();
	return parse(s);
}

Parser<Half_For> GetForParser()
{
	return [](ParserInput s) -> ParserResult<Half_For>
		{
			auto space = Spaces();
			auto keyfor = Between(space, space, String("for"));
			auto _for = keyfor(s);
			if (!_for)
			{
				return std::nullopt;
			}

			auto keyto = Between(space, space, String("to"));
			auto keydownto = Between(space, space, String("downto"));
			auto keywith = Between(space, space, String("with"));
			auto keydo = Between(space, space, String("do"));
			auto keyend = Between(space, space, String("end"));
			auto punc = Between(space, space, OneChar('='));

			auto pv = (keyfor > pvar);
			auto rvar = pv(s);
			if (!rvar)
			{
				return std::nullopt;
			}
			s = rvar.value().second;

			auto pstart = (punc > pexpr);
			auto rstart = pstart(s);
			if (!rstart)
			{
				return std::nullopt;
			}
			s = rstart.value().second;

			bool up = true;
			auto pend = (keyto > pexpr);
			auto rend = pend(s);
			if (!rend)
			{
				rend = (keydownto > pexpr)(s);
				if (!rend)
				{
					return std::nullopt;
				}
				up = false;
			}
			s = rend.value().second;

			auto pbody = ((keydo > pmanyexpr) < keyend);
			auto rbody = pbody(s);
			if (!rbody)
			{
				return std::nullopt;
			}
			s = rbody.value().second;

			auto f = Half_For(rvar.value().first, rstart.value().first, rend.value().first, rbody.value().first, up);
			return std::make_pair(f, s);
		};
}

ParserResult<Half_For> pfor(ParserInput s)
{
	static auto parser = GetForParser();
	return parser(s);
}

Parser<Half_While> GetWhileParser()
{
	return [](ParserInput s) -> ParserResult<Half_While>
		{
			auto space = Spaces();
			auto keywhile = Between(space, space, String("while"));
			auto _while = keywhile(s);
			if (!_while)
			{
				return std::nullopt;
			}

			auto keydo = Between(space, space, String("do"));
			auto keyend = Between(space, space, String("end"));
			auto pcondition = (keywhile > pexpr) < keydo;
			auto pbody = OnlyFirst(pmanyexpr, keyend);

			// start parse
			auto condition = pcondition(s);
			if (!condition)
			{
				printf("Parse while condition failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			s = condition.value().second;

			auto body = pbody(s);
			if (!body)
			{
				printf("Parse while body failed! at line:%zd col:%zd\n",
					s.current_pos.line, s.current_pos.column);
				return std::nullopt;
			}
			s = body.value().second;
			return std::make_pair(Half_While(condition.value().first, body.value().first), s);
		};
}

ParserResult<Half_While> pwhile(ParserInput s)
{
	static auto parse = GetWhileParser();
	return parse(s);
}

ParserResult<Half_Expr> pmanyexpr(ParserInput s)
{
	static auto exprparser = PipeExpr(Many1(pexpr));
	return exprparser(s);
}

Parser<Half_Expr> GetExprParser()
{
	return [](ParserInput s) ->ParserResult<Half_Expr>
		{
			auto assignparser = PipeExpr(passign);
			auto letparser = PipeExpr(plet);
			auto ifparser = PipeExpr(pif);
			auto forparser = PipeExpr(pfor);
			auto whileparser = PipeExpr(pwhile);
			auto opparser = PipeExpr(pop);
			auto funcallparser = PipeExpr(pfuncall);
			auto varparser = PipeExpr(pvar);
			auto valueparser = PipeExpr(pvalue);
			auto fundefparser = PipeExpr(pfuncdecl);

			auto c = Choice(std::vector{
				fundefparser,
				letparser, assignparser, ifparser, forparser, whileparser,
				opparser, funcallparser, varparser, valueparser, });
			auto r = c(s);
			if (!r)
			{
				return std::nullopt;
			}
			s = r.value().second;

			return std::make_pair(r.value().first, s);
		};
}

ParserResult<Half_Expr> pexpr(ParserInput s)
{
	static auto parser = GetExprParser();
	return parser(s);
}

ParserResult<Half_Expr> pprogram(ParserInput s)
{
	return pmanyexpr(s);
}