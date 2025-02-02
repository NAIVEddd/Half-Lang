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

bool IsHeadChar(char c)
{
	return IsChar(c) || c == '_';
}

ParserResult<std::string> pvariablename(ParserInput s)
{
	auto head = OneOf(IsHeadChar);
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

ParserResult<std::string> ptypename(ParserInput s)
{
	// parse '.'
	//auto point = OneChar('.');
	auto head = OneOf(IsHeadChar);
	auto tail = Many(OneDigitOrChar());
	Converter2<char, std::vector<char>, std::string> tostring =
		[](char c, std::vector<char>&& cs)
		{
			std::string str = std::string(1, c) + std::string(cs.begin(), cs.end());
			return str;
		};
	auto pname = Pipe2(head, tail, tostring);
	auto n = pname(s);
	//if (Keyword.contains(n.value().first))
	//{
	//	// print error message, row and col
	//	printf("Error: %s is a keyword\n", n.value().first.c_str());
	//	printf("    At Line: %zd, Col: %zd\n", s.current_pos.line, s.current_pos.column);
 //       return std::nullopt;
	//}

	if (!n)
	{
        // not a string type name
		// maybe '...' type
        auto points = Many(OneChar('.'));
		auto p = points(s);
		if (!p || p.value().first.size() != 3)
		{
			return std::nullopt;
		}
		return std::make_pair("...", p.value().second);
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
	auto pinline_expr = OnlyFirst(BetweenPair('(', ')'), space);
	auto expr = pinline_expr(name.value().second);
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

Parser<Half_ArrayInit> GetArrayInitParser()
{
    return [](ParserInput s) -> ParserResult<Half_ArrayInit>
        {
            auto space = Spaces();

			auto array_elem_name = ptypename(s);
			if (!array_elem_name)
			{
				return std::nullopt;
			}
			s = array_elem_name.value().second;

            auto popen = Between(space, space, String("[["));
            auto pclose = Between(space, space, String("]]"));
            auto psep = Between(space, space, OneChar(','));
            auto parrayinitbody = SepBy(Between(space, space, pexpr), psep);
            auto pbody = Between(popen, pclose, parrayinitbody);
            auto body = pbody(s);
            if (!body)
            {
                return std::nullopt;
            }
            auto& exprs = body.value().first;
            return std::make_pair(Half_ArrayInit(array_elem_name.value().first, exprs), body.value().second);
        };
}

ParserResult<Half_ArrayInit> parrayinit(ParserInput s)
{
    static auto parsearrayinit = GetArrayInitParser();
    auto res = parsearrayinit(s);
	return res;
}

Parser<Half_ArrayNew> GetArrayNewParser()
{
	return [](ParserInput s) -> ParserResult<Half_ArrayNew>
		{
			auto space = Spaces();

			auto type_name = ptypename(s);
			if (!type_name)
			{
				return std::nullopt;
			}
			s = type_name.value().second;

			auto popen = Between(space, space, String("[["));
			auto pclose = Between(space, space, String("]]"));
			auto pcount = Between(popen, pclose, pexpr);
            auto count = pcount(s);
			if (!count)
			{
				return std::nullopt;
			}
			auto& expr = count.value().first;
			return std::make_pair(Half_ArrayNew(type_name.value().first, expr), count.value().second);
		};
}

ParserResult<Half_ArrayNew> parraynew(ParserInput s)
{
    static auto parsearraynew = GetArrayNewParser();
    auto res = parsearraynew(s);
    return res;
}

Parser<Half_StructInit> GetStructInitParser()
{
    return [](ParserInput s) -> ParserResult<Half_StructInit>
        {
            auto space = Spaces();
            
            auto struct_name = ptypename(s);
            if (!struct_name)
            {
                return std::nullopt;
            }
            s = struct_name.value().second;

            auto popen = Between(space, space, OneChar('{'));
            auto pclose = Between(space, space, OneChar('}'));
            auto psep = Between(space, space, OneChar(','));
            auto pequal = Between(space, space, OneChar('='));
            auto pstructinitbody = SepBy(Between(space, space, pexpr), psep);

            // 1. field = expr	{a = 1, c = 3, b = x}
            // 2. var or value	{1, 2, c}
            Converter2<std::string, Half_Expr, Half_StructInit::FieldInit> toFieldInit =
                [](std::string name, Half_Expr expr)
                {
                    return Half_StructInit::FieldInit(name, expr);
                };
            Converter<Half_Var, Half_StructInit::FieldInit> varToFieldInit =
				[](Half_Var v)
				{
					return Half_StructInit::FieldInit("", v);
				};
            Converter<Half_Value, Half_StructInit::FieldInit> valueToFieldInit =
                [](Half_Value v)
                {
                    return Half_StructInit::FieldInit("", v);
                };
			Parser<std::string> pname = OnlyFirst(pvariablename, pequal);
            Parser<Half_StructInit::FieldInit> assign = Pipe2(pname, pexpr, toFieldInit);
			Parser<Half_StructInit::FieldInit> var = Pipe(pvar, varToFieldInit);
            Parser<Half_StructInit::FieldInit> value = Pipe(pvalue, valueToFieldInit);
			auto ponefield = Choice(std::vector{ assign, var, value });
            auto pfield = SepBy(ponefield, psep);
            auto pbody = Between(popen, pclose, pfield);

            auto body = pbody(s);
            if (!body)
            {
                return std::nullopt;
            }
            auto& field_exprs = body.value().first;

            return std::make_pair(Half_StructInit(struct_name.value().first, field_exprs), body.value().second);
        };
}

ParserResult<Half_StructInit> pstructinitbody(ParserInput s)
{
	static auto parsestructinit = GetStructInitParser();
    auto res = parsestructinit(s);
	return res;
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

ParserResult<int> pconstint(ParserInput s)
{
	std::vector<char> str;
	auto istr = Many1(OneDigit())(s);
	if (!istr)
	{
		return std::nullopt;
	}
	str.resize(istr.value().first.size());
	std::copy(istr.value().first.begin(), istr.value().first.end(),
		&str[0]);
	auto i = std::stoi(std::string(str.begin(), str.end()));
	return std::make_pair(i, istr.value().second);
}

ParserResult<Half_TypeDecl::Nil> pniltype(ParserInput s)
{
	auto space = Spaces();
	auto keynil = Between(space, space, String("nil"));
	auto rnil = keynil(s);
	if (!rnil)
	{
		return std::nullopt;
	}
	return std::make_pair(Half_TypeDecl::Nil(), rnil.value().second);
}
// ParserResult<Half_TypeDecl::RenameType> prenametype(ParserInput s)
// {
// 	auto space = Spaces();
// 	auto keytype = Between(space, space, String("type"));
// 	auto rtype = keytype(s);
// 	if (!rtype)
// 	{
// 		return std::nullopt;
// 	}
// 	s = rtype.value().second;

// 	auto keyequal = Between(space, space, OneChar('='));
// 	auto requal = keyequal(s);
// 	if (!requal)
// 	{
// 		return std::nullopt;
// 	}
// 	s = requal.value().second;

// 	auto keyname = pvariablename;
// 	auto rname = keyname(s);
// 	if (!rname)
// 	{
// 		return std::nullopt;
// 	}
// 	return std::make_pair(Half_TypeDecl::RenameType(rname.value().first), rname.value().second);
// }
// ParserResult<Half_TypeDecl::TupleType> ptupletype(ParserInput s);
ParserResult<Half_TypeDecl::Additional> padditionaltype(ParserInput s)
{
	auto space = Spaces();
	auto keyadditional = Between(space, space, String("..."));
	auto radditional = keyadditional(s);
	if (!radditional)
	{
		return std::nullopt;
	}
	return std::make_pair(Half_TypeDecl::Additional(), radditional.value().second);
}

ParserResult<Half_TypeDecl::Ptr> ppointertype(ParserInput s)
{
	// parse pointer type like 'ptr of int'
	auto space = Spaces();
	auto keypointer = Between(space, space, String("ptr"));
	auto rpointer = keypointer(s);
	if (!rpointer)
	{
		return std::nullopt;
	}
	s = rpointer.value().second;

	auto keyof = Between(space, space, String("of"));
	auto rkeyof = keyof(s);
	if (!rkeyof)
	{
		return std::nullopt;
	}
	s = rkeyof.value().second;

	auto ptype = pvariablename;
	auto rtype = ptype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	return std::make_pair(Half_TypeDecl::Ptr(rtype.value().first), rtype.value().second);
}

ParserResult<Half_TypeDecl::IncompleteArrayType> pincompletetype(ParserInput s)
{
	auto space = Spaces();
	auto keyarray = Between(space, space, String("array"));
	auto rarray = keyarray(s);
	if (!rarray)
	{
		return std::nullopt;
	}
	s = rarray.value().second;

	auto keyof = Between(space, space, String("of"));
	auto rkeyof = keyof(s);
	if (!rkeyof)
	{
		return std::nullopt;
	}
	s = rkeyof.value().second;

	auto ptype = pvariablename;
	auto rtype = ptype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	return std::make_pair(Half_TypeDecl::IncompleteArrayType(rtype.value().first), rtype.value().second);
}

ParserResult<Half_TypeDecl::ArrayType> parraytype(ParserInput s)
{
	auto space = Spaces();
	auto keyarray = Between(space, space, String("array"));
	auto rarray = keyarray(s);
	if (!rarray)
	{
		return std::nullopt;
	}
	s = rarray.value().second;
	
	auto keyopen = Between(space, space, OneChar('['));
	auto keyclose = Between(space, space, OneChar(']'));
	auto keyof = Between(space, space, String("of"));
	auto keycolon = Between(space, space, OneChar(':'));
	auto keysep = Between(space, space, OneChar(','));
	auto pname = Between(space, space, pvariablename);
	Converter3<std::string, std::string, int, Half_TypeDecl::ArrayType> toarraytype =
		[](std::string, std::string t, int sz)
		{
			return Half_TypeDecl::ArrayType(t, sz);
		};
	Converter3<char, int, char, int> tosize =
		[](char, int s, char)
		{
			return s;
		};
	auto psize = Pipe3(keyopen, pconstint, keyclose, tosize);
	// parse array type like 'array of int [10]'
	auto ptype = Pipe3(keyof, pname, psize, toarraytype);
	auto rtype = ptype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	return rtype;
}

ParserResult<Half_TypeDecl::StructType> pstructbody(ParserInput s)
{
	auto space = Spaces();
	auto rspace = space(s);
	s = rspace.value().second;
	if(s[0] != '{')
	{
		return std::nullopt;
	}

	auto keyopen = Between(space, space, OneChar('{'));
	auto keyclose = Between(space, space, OneChar('}'));
	auto keyinter = Between(space, space, OneChar(':'));
	auto keysep = Between(space, space, OneChar(','));
	auto pname = Between(space, space, pvariablename);
	Converter3<std::string, char, std::string, Half_TypeDecl::StructType::TypePair> totypepair =
		[](std::string n, char _, std::string t)
		{
			return Half_TypeDecl::StructType::TypePair(n, t);
		};
	auto ppair = Pipe3(pname, keyinter, pname, totypepair);
	auto pmanypair = SepBy(ppair, keysep);
	Converter3<char, std::vector<Half_TypeDecl::StructType::TypePair>, char, std::vector<Half_TypeDecl::StructType::TypePair>> tostructtype =
		[](char, std::vector<Half_TypeDecl::StructType::TypePair> tys, char)
		{
			return tys;
		};
	auto pbody = Pipe3(keyopen, pmanypair, keyclose, tostructtype);

	auto rbody = pbody(s);
	if (!rbody)
	{
		printf("Parse struct body failed! at line:%zd col:%zd\n",
			s.current_pos.line, s.current_pos.column);
		return std::nullopt;
	}
	Half_TypeDecl::StructType st("", rbody.value().first);
	s = rbody.value().second;

	return std::make_pair(st, s);
}

ParserResult<Half_TypeDecl::FuncType> pfunctype(ParserInput s)
{
	// parse function type like 'function (int, char) : int'
	//   TODO: or 'function int -> char'
	auto space = Spaces();
	auto keyfunc = Between(space, space, String("function"));
	auto rfunc = keyfunc(s);
	if (!rfunc)
	{
		return std::nullopt;
	}
	s = rfunc.value().second;

	auto keyopen = Between(space, space, OneChar('('));
	auto keyclose = Between(space, space, OneChar(')'));
	auto keycolon = Between(space, space, OneChar(':'));
	auto keyarrow = Between(space, space, String("->"));
	auto keysep = Between(space, space, OneChar(','));
	auto pname = Between(space, space, ptypename);
	auto pmany_arg_types = SepBy(pname, keysep);
	auto parg_types = Between(keyopen, keyclose, pmany_arg_types);
	auto rarg_types = parg_types(s);
	if (!rarg_types)
	{
		printf("Parse function arguments failed! at line:%zd col:%zd\n",
			s.current_pos.line, s.current_pos.column);
		return std::nullopt;
	}
	s = rarg_types.value().second;

	auto prettype = keycolon > pname;
	auto rrettype = prettype(s);
	if (!rrettype)
	{
		printf("Parse function return type failed! at line:%zd col:%zd\n",
			s.current_pos.line, s.current_pos.column);
		return std::nullopt;
	}
	s = rrettype.value().second;
	auto res = Half_TypeDecl::FuncType(rrettype.value().first, rarg_types.value().first);
	return std::make_pair(res, s);
}

ParserResult<Half_TypeDecl> ptypeuse(ParserInput s)
{
	// parse type like 'int' or 'int_array [10]' or 'array of int [10]' or 'ptr of int'
	auto space = Spaces();
	auto keyarray = Between(space, space, String("array"));
	auto keyptr = Between(space, space, String("ptr"));
	auto keyof = Between(space, space, String("of"));
	auto keyopen = Between(space, space, OneChar('['));
	auto keyclose = Between(space, space, OneChar(']'));
	auto keycolon = Between(space, space, OneChar(':'));
	auto keysep = Between(space, space, OneChar(','));
	auto pname = Between(space, space, pvariablename);
	auto psize = Between(keyopen, keyclose, pconstint);
	
	{	// 'int_array [10]'
		Converter2<std::string, int, Half_TypeDecl::CompleteArrayType> toarraytype =
			[](std::string t, int sz)
			{
				return Half_TypeDecl::CompleteArrayType(t, sz);
			};
		auto pcompletearraytype = Pipe2(pname, psize, toarraytype);
		// 'int_array [10]'
		auto rcompletearraytype = pcompletearraytype(s);
		if (rcompletearraytype)
		{
			return rcompletearraytype;
		}
	}
	
	{	// 'array of int [10]'
		auto parray_t = keyarray > keyof > pname;
		Converter2<std::string, int, Half_TypeDecl::ArrayType> make_type =
			[](std::string t, int sz)
			{
				return Half_TypeDecl::ArrayType(t, sz);
			};
		auto parray = Pipe2(parray_t, psize, make_type);
		auto rarray = parray(s);
		if (rarray)
		{
			return rarray;
		}
	}

	{	// 'ptr of int'
		auto pptr = keyptr > keyof > pname;
		Converter<std::string, Half_TypeDecl::Ptr> make_ptr_type =
			[](std::string t)
			{
				return Half_TypeDecl::Ptr(t);
			};
		auto ppointer = Pipe(pptr, make_ptr_type);
		auto rpointer = ppointer(s);
		if (rpointer)
		{
			return rpointer;
		}
	}

	// 'int'
	auto rname = pname(s);
	if(!rname || Keyword.contains(rname.value().first))
	{
		return std::nullopt;
	}
	return std::make_pair(Half_TypeDecl(rname.value().first), rname.value().second);
}

ParserResult<Half_TypeDecl> ptypedecl(ParserInput s)
{
	auto space = Spaces();
	auto keytype = Between(space, space, String("type"));
	auto rtype = keytype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	s = rtype.value().second;

	auto keyequal = Between(space, space, OneChar('='));
	auto pname = Between(space, space, pvariablename);

	auto rtypename = OnlyFirst(pname, keyequal)(s);
	if (!rtypename)
	{
		printf("Parse type name failed! at line:%zd col:%zd\n",
			s.current_pos.line, s.current_pos.column);
		return std::nullopt;
	}
	s = rtypename.value().second;

	{	// check if type is struct
		auto rbody = pstructbody(s);
		if (rbody)
		{
			auto structtype = rbody.value().first;
			structtype.name = rtypename.value().first;
			return std::make_pair(Half_TypeDecl(structtype), rbody.value().second);
		}
	}

		// check if type is a rename_type
	    //    like 'function (int, char) : int' (function type)
		//		or 'type a = ptr of int'
		//   	or 'type a = nil'
		//   	or 'type a = array of int [10]'
		//   	or 'type a = array of int'
		//   	or 'type a = int_array [10]'
		//   	or 'type a = int_array'
	{	// rename type
		{  // parse like 'type a = function (ptr, ...) : int' or 'type a = int'
			auto rtype = pfunctype(s);
			if (rtype)
			{
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
				return std::make_pair(res, rtype.value().second);
			}
		}
		{  //   parse like 'type a = ptr of int'
			auto rtype = ppointertype(s);
			if (rtype)
			{
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
				return std::make_pair(res, rtype.value().second);
			}
		}

		{  //   parse like 'type a = nil'
			auto rtype = pniltype(s);
			if (rtype)
			{
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
				return std::make_pair(res, rtype.value().second);
			}
		}

		// syntax like 'type a = ...' is not supported
		{  //   parse like 'type a = ...'
			// auto rtype = padditionaltype(s);
			// if (rtype)
			// {
			// 	auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
			// 	return std::make_pair(res, rtype.value().second);
			// }
		}

		{  //   parse like 'type a = array of int [10]'
			auto rtype = parraytype(s);
			if (rtype)
			{
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
				return std::make_pair(res, rtype.value().second);
			}
		}

		{  //   parse like 'type a = array of int'
			auto rtype = pincompletetype(s);
			if (rtype)
			{
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, rtype.value().first);
				return std::make_pair(res, rtype.value().second);
			}
		}

		{  //   parse like 'type a = int_array [10]' or 'type a = int_array'
		    do
			{
				auto rincompletetype = pname(s);
				if (!rincompletetype)
				{
					break;
				}
				s = rincompletetype.value().second;

				// parse '[' int ']'
				auto keyopen = Between(space, space, OneChar('['));
				auto keyclose = Between(space, space, OneChar(']'));
				Converter3<char, int, char, int> tosize =
					[](char, int s, char)
					{
						return s;
					};
				auto psize = Pipe3(keyopen, pconstint, keyclose, tosize);
				auto rsize = psize(s);
				// if size is not found, then it is a simple rename type
				// 	like 'type a = int_array'
				if (!rsize)
				{
					auto res = Half_TypeDecl::RenameType(rtypename.value().first, rincompletetype.value().first);
					return std::make_pair(res, s);
				}
				// if size is found, then it is a complete array type
				// 	like 'type a = int_array [10]'
				Half_TypeDecl::CompleteArrayType ArrayType(rincompletetype.value().first, rsize.value().first);
				auto res = Half_TypeDecl::RenameType(rtypename.value().first, ArrayType);
				return std::make_pair(res, rsize.value().second);
			} while (false);
		}
	}

	// If none of the conditions are met, return std::nullopt
	printf("Parse type failed! at line:%zd col:%zd\n",
		s.current_pos.line, s.current_pos.column);
	return std::nullopt;
}

ParserResult<Def_Type> pdeftype(ParserInput s)
{
	auto space = Spaces();
	auto keytype = Between(space, space, String("type"));
	auto rtype = keytype(s);
	if (!rtype)
	{
		return std::nullopt;
	}
	s = rtype.value().second;

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
			//auto fundefparser = PipeExpr(pfuncdecl);
            //auto typeparser = PipeExpr(ptypedecl);
            auto structinitparser = PipeExpr(pstructinitbody);
            auto arrayinitparser = PipeExpr(parrayinit);
            auto arraynewparser = PipeExpr(parraynew);

			auto c = Choice(std::vector{
				//fundefparser, typeparser,
				letparser, assignparser, ifparser, forparser, whileparser,
                opparser, structinitparser, arrayinitparser, arraynewparser,
				funcallparser, varparser, valueparser, });
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

ParserResult<Half_OuterExpr> pmanyouterexpr(ParserInput s)
{
	static auto exprparser = PipeOuterExpr(Many1(pouterexpr));
	return exprparser(s);
}

Parser<Half_OuterExpr> GetOuterExprParser()
{
    return [](ParserInput s) ->ParserResult<Half_OuterExpr>
        {
            auto fundefparser = PipeOuterExpr(pfuncdecl);
            auto typeparser = PipeOuterExpr(ptypedecl);
            auto c = Choice(std::vector{fundefparser, typeparser });
            auto r = c(s);
            if (!r)
            {
                return std::nullopt;
            }
            s = r.value().second;
            return std::make_pair(r.value().first, s);
        };
}

ParserResult<Half_OuterExpr> pouterexpr(ParserInput s)
{
    static auto parser = GetOuterExprParser();
    return parser(s);
}

ParserResult<Half_OuterExpr> pprogram(ParserInput s)
{
	return pmanyouterexpr(s);
}