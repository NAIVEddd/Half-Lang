#pragma once
#include<string>
#include"Parser/Parser.h"
#include"Parser/Primitives.h"
#include"Parser/CharParsers.h"
#include"Parser/Operator.h"
#include"Syntax/TypeCheck.h"
#include"Syntax/ExprParser.h"
//#include"IR/MakeIR.h"
#include"IR/Semant.h"
#include"CodeGen/Assem.h"
#include"CodeGen/Graph.h"
#include"CodeGen/Liveness.h"
#include"CodeGen/Color.h"
#include"CodeGen/RegAlloc.h"
#include"Pass/IrFormatPass.h"
#include"Pass/Mem2RegPass.h"

std::string test_main_0 =
R"(
int main()
{
    return 0;
}
)";

std::string test_time_1 = 
R"(
int main() {
  // sum of array
  int arr[20], n, sum = 0;
  n = getarray(arr);
  putarray(n, arr);
  int count = getint();
  putint(count);
  putch(10);
  starttime();
  for (int j = 0; j < count; ++j) {
    for (int k = 0; k < n; ++k) {
      for (int i = 0; i < n; ++i) sum += arr[i] * arr[k];
    }
  }
  stoptime();
  putint(sum);
  putch(10);

  // read characters
  n = getint();
  getch();
  for (int i = 0; i < n; ++i) putch(getch());
  putch(10);

  return 0;
}
)";

std::string test_prog_2 =
R"(
int square(int x) {
  return x * x;
}
int main()
{
    int x = 5;
    int y = square(x);
    return y;
}
)";

void test_parser_function()
{
    Parser<char> charParser = OneOf(IsChar);
    Parser<char> spaceParser = OneOf(IsSpace);
    Parser<char> newLineParser = OneOf(IsNewline);
    auto firstSpace = spaceParser(test_main_0);
    if (firstSpace.has_value())
    {
        printf("First char is space\n");
    }

    // test converter
    Converter<char, int> conv = [](char) {return 1; };
    auto intParser = Pipe(newLineParser, conv);
    if (intParser(test_main_0).has_value())
    {
        printf("Pipe success\n");
    }

    // test converter2
    Converter2<char, char, int> conv2 = [](char, char) {return 1; };
    Parser<int> intParser2 = Pipe2(newLineParser, charParser, conv2);
    if (intParser2(test_main_0))
    {
        printf("Pipe2 success\n");
    }

    // test converter3
    Converter3<char, char, char, int> conv3 = [](char, char, char) {return 1; };
    Parser<int> intParser3 = Pipe3(newLineParser, charParser, charParser, conv3);
    if (intParser3(test_main_0))
    {
        printf("Pipe3 success\n");
    }

    // test converter4
    {
        Converter4<char, char, char, char, int> conv4 = [](char, char, char, char) {return 1; };
        Parser<int> intParser4 = Pipe4(newLineParser, charParser, charParser, charParser, conv4);
        if (intParser4(test_main_0))
        {
            printf("Pipe4 success\n");
        }

        Converter5<char, char, char, char, char, int> conv5 = [](char, char, char, char, char) {return 1; };
        Parser<int> intParser5 = Pipe5(newLineParser, charParser, charParser, charParser, spaceParser, conv5);
        if (intParser5(test_main_0))
        {
            printf("Pipe5 success\n");
        }
    }

    // test OnlyFirst
    {
        Converter2<char, char, int> conv_c_c_i = [](char, char) {return 3; };
        auto combine = Combine(newLineParser, charParser, conv_c_c_i);
        auto onlyFirst = OnlyFirst(newLineParser, charParser);
        auto onlySecond = OnlySecond(newLineParser, charParser);
        if (combine(test_main_0) && onlyFirst(test_main_0) && onlySecond(test_main_0))
        {
            printf("OnlyFirst and OnlySecond success\n");
        }
    }

    // test operator < and operator >
    {
        auto first = newLineParser < charParser;
        bool is_char = std::is_same<char, ParserResult_t<decltype(first)>>::value;
        auto second = charParser > intParser;
        bool is_int = std::is_same<int, ParserResult_t<decltype(second)>>::value;
        if (is_char && is_int)
        {
            printf("operator '<' and '>' success\n");
        }
    }

    // test operator &
    {
        auto seq = newLineParser & intParser;
        bool is_pair_c_i = std::is_same_v<std::pair<char, int>, ParserResult_t<decltype(seq)>>;
        if (is_pair_c_i)
        {
            printf("operator '&' success\n");
        }
    }

    // test operator |
    {
        Parser<char> space_or_char = charParser | spaceParser;
        Parser<char> n = space_or_char | newLineParser;
        Parser<char> newline_or_space_or_char = space_or_char | newLineParser;
        if (!space_or_char(test_main_0) && newline_or_space_or_char(test_main_0))
        {
            printf("operator '|' success\n");
        }
    }

    // test Operator>>
    {
        std::string prog1 = "1234";
        Parser<std::string> p_digit = ManySatisfy(IsDigit);
        Converter<std::string, int> lambda =
            [](std::string str)
            {
                return std::stoi(str);
            };
        auto i = (p_digit >> lambda)(prog1);
        _ASSERT(i && i.value().first == 1234);
    }

    // test Between
    {
        Parser<int> between = Between(newLineParser, charParser, intParser);
        bool is_int = std::is_same_v<int, ParserResult_t<decltype(between)>>;
        if (is_int)
        {
            printf("between success\n");
        }
    }

    // test Choice
    {
        Parser<char> choiceNewline = Choice(std::vector{ charParser, spaceParser, newLineParser });
        Parser<char> choiceNull = Choice(std::vector{ charParser, spaceParser });
        if (choiceNewline(test_main_0) && !choiceNull(test_main_0))
        {
            printf("Choice success\n");
        }
    }

    // test Opt
    {
        Parser<std::optional<char>> optChar = Opt(charParser);
        Parser<std::optional<char>> optNewline = Opt(newLineParser);
        auto a = optChar(test_main_0);
        auto b = optNewline(test_main_0);
        if (a && b && !a.value().first && b.value().first)
        {
            printf("Opt success\n");
        }
    }

    // test Fail
    {
        Parser<char> fail_char = Fail<char>(std::string("always fail with some message"));
        fail_char(test_main_0);
    }

    // test Array
    {
        Parser<std::optional<char>> optNewline = Opt(newLineParser);
        auto b = optNewline(test_main_0);

        Parser<std::vector<char>> pArray = Array(3, charParser);
        if (pArray(b.value().second))
        {
            printf("Parser Array success\n");
        }
    }

    // test Many
    {
        Parser<std::optional<char>> optNewline = Opt(newLineParser);
        auto b = optNewline(test_main_0);

        Parser<std::vector<char>> pArray = Many(charParser);
        if (pArray(b.value().second))
        {
            printf("Parser Many success\n");
        }

        Parser<std::vector<char>> pMany1 = Many1(charParser);
        if (!pMany1(test_main_0) && pMany1(b.value().second))
        {
            printf("Parser Many1 success\n");
        }
    }

    // test FollowedBy
    {
        Parser<char> newline = FollowedBy(newLineParser);
        if (newline(test_main_0))
        {
            printf("Parser FollowedBy success\n");
        }
    }

    // test SepBy
    {
        Parser<char> sep = MakeCharParser(',');
        Parser<std::vector<char>> pSep = SepBy(charParser, sep);
        Parser<std::vector<char>> pSepEndBy = SepEndBy(charParser, sep);
        auto end1 = pSep(std::string("a,"));
        auto end2 = pSepEndBy(std::string("a,"));

        if (pSep(std::string("a,b,c")) && pSep(std::string("a")) &&
            (not end1.value().second.empty()) &&
            end2.value().second.empty())
        {
            printf("Parser SepBy success\n");
        }
    }

    // test ManyTill
    {
        Parser<char> c = charParser;
        Parser<char> end = MakeCharParser(',');
        Parser<std::vector<char>> manyC = ManyTill(c, end);
        auto many1 = manyC("abc,");
        auto many2 = manyC("a+");
        if (many1 && !many2)
        {
            printf("Parser ManyTill success\n");
        }
    }

    // test PString
    {
        Parser<std::string> str = String("str");
        auto res = str("str parser");
        if (res)
        {
            printf("Parser String success\n");
        }

        Parser<std::string> id = ManySatisfy(IsChar, [](char c) {return IsChar(c) || IsDigit(c); });
        auto res1 = id("str parser");
        auto res2 = id("str123 parser");
        if (res1 && res2)
        {
            printf("Parser ManySatisfy success\n");
        }
    }
}

void test_syntax_parser()
{
    // test Spaces
    {
        std::string prog1 = " \r\n \t";
        Parser<char> s = AnyOf(" \t\r\n");
        auto res = Many(s)(prog1);
        _ASSERT(res);
        auto res1 = (Spaces() < Eof())(prog1);
        _ASSERT(res1);
    }

    // test 'let'
    {
        std::string prog = "let i = 1";

        Parser<std::string> p_id = ManySatisfy(IsChar, [](char c) {return IsChar(c) || IsDigit(c); });
        auto let = (p_id < Spaces())(prog);
        _ASSERT(let.value().first == "let");
        
        Parser<std::string> p_let = String("let") < Spaces();
        let = p_let(prog);
        _ASSERT(let.value().first == "let");
    }

    // test SepBy
    {
        std::string prog1 = "a ";
        
        Parser<std::string> p_id = ManySatisfy(IsChar, [](char c) {return IsChar(c) || IsDigit(c); });
        auto p_id_space = (p_id < Spaces());
        auto p_space_id = (Opt(Spaces()) > p_id);

        auto res = p_id_space(prog1);
        _ASSERT(res);
        _ASSERT(res.value().second.empty());
        res = p_space_id(prog1);
        _ASSERT(res);
        _ASSERT(not res.value().second.empty());
    }


    // test Pattern
    {
        std::string prog1 = "a, b , c ";
        std::string prog2 = "a, b, c, ";
        std::string prog3 = "a01bc , b02cd , c03de ";
        Parser<std::string> p_id = ManySatisfy(IsChar, [](char c) {return IsChar(c) || IsDigit(c); });

        auto id = p_id(prog1);
        _ASSERT(id);

        auto p_pattern = SepBy(p_id < Opt(Spaces()), OneChar(',') < Opt(Spaces()));
        auto pattern = (p_pattern < Eof())(prog1);
        _ASSERT(pattern);
        _ASSERT(pattern.value().first.size() == 3);
        pattern = p_pattern(prog2);
        _ASSERT(pattern);
        _ASSERT(pattern.value().first.size() == 3);
        pattern = (p_pattern < Eof())(prog3);
        _ASSERT(pattern);
        _ASSERT(pattern.value().first.size() == 3);
    }
}

void test_operator_parser()
{
    // test removeindex
    {
        std::vector<int> i1 = { 0, 1, 2, 3, 4, 5 };
        RemoveIndex(i1, 0);
        _ASSERT(i1.size() == 5);
        _ASSERT(i1[0] == 1);
        i1 = { 0, 1, 2, 3, 4, 5 };
        RemoveIndex(i1, 5);
        _ASSERT(i1.size() == 5);
        _ASSERT(i1[4] == 4);
    }

    // test a + b
    {
        std::string prog = "1 + 3";
        std::string prog1 = "1 + 2 * 3";
        std::string prog2 = "1 + 2 * 3 - 4 ";
        std::string prog3 = "1 + (6 - 3 * 1)";
        std::string prog4 = "1 + 2 * 3 - 10 / 5";
        Parser<std::vector<char>> num = Many1(OneOf(IsDigit)) < Spaces();
        Converter<std::vector<char>, int> cstoint =
            [](std::vector<char> v)
            {
                return std::stoi(std::string(v.begin(), v.end()));
            };
        Parser<int> pint = Pipe(num, cstoint);
        auto opParser = OperatorParser(pint);
        Converter2<int, int, int> plus = [](int l, int r) {return l + r; };
        auto op_plus = InfixOperator<int>("+", 6, plus);
        auto op_minus = InfixOperator<int>("-", 6, [](int l, int r) {return l - r; });
        auto op_multy = InfixOperator<int>("*", 7, [](int l, int r) {return l * r; });
        auto op_devide = InfixOperator<int>("/", 7, [](int l, int r) {return l / r; });
        auto op_and = InfixOperator<int>("&&", 1, [](int l, int r) {return l && r; });
        auto op_or = InfixOperator<int>("||", 1, [](int l, int r) {return l || r; });
        auto op_eq = InfixOperator<int>("==", 3, [](int l, int r) {return l == r; });
        auto op_not_eq = InfixOperator<int>("!=", 3, [](int l, int r) {return l != r; });
        auto op_less = InfixOperator<int>("<", 4, [](int l, int r) {return l < r; });
        auto op_less_eq = InfixOperator<int>("<=", 4, [](int l, int r) {return l <= r; });
        auto op_greater = InfixOperator<int>(">", 4, [](int l, int r) {return l > r; });
        auto op_greater_eq = InfixOperator<int>(">=", 4, [](int l, int r) {return l >= r; });
        auto op_shift_l = InfixOperator<int>("<<", 5, [](int l, int r) {return l << r; });
        auto op_shift_r = InfixOperator<int>(">>", 5, [](int l, int r) {return l >> r; });

        opParser.AddOperator(op_plus);
        opParser.AddOperator(op_minus);
        opParser.AddOperator(op_multy);
        opParser.AddOperator(op_devide);
        opParser.AddOperator(op_and);
        opParser.AddOperator(op_or);
        opParser.AddOperator(op_eq);
        opParser.AddOperator(op_not_eq);
        opParser.AddOperator(op_less);
        opParser.AddOperator(op_less_eq);
        opParser.AddOperator(op_greater);
        opParser.AddOperator(op_greater_eq);
        opParser.AddOperator(op_shift_l);
        opParser.AddOperator(op_shift_r);
        auto result = opParser.GetParser()(prog);
        _ASSERT(result);
        _ASSERT(result.value().first == 4);
        
        result = opParser.GetParser()(prog1);
        _ASSERT(result);
        _ASSERT(result.value().first == 7);
        
        result = opParser.GetParser()(prog2);
        _ASSERT(result);
        _ASSERT(result.value().first == 3);
        
        result = opParser.GetParser()(prog3);
        _ASSERT(result);
        _ASSERT(result.value().first == 4);

        result = opParser.GetParser()(prog4);
        _ASSERT(result);
        _ASSERT(result.value().first == 5);

        std::string prog5 = "1 + 2 * 3 - 2 - 5";
        result = opParser.GetParser()(prog5);
        _ASSERT(result);
        _ASSERT(result.value().first == 0);

        std::string prog6 = "1 + 2 * 3 - 2 * 2 - 1";
        result = opParser.GetParser()(prog6);
        _ASSERT(result);
        _ASSERT(result.value().first == 2);

        std::string prog7 = "1 + 2 * (5 - 2 * 2) - 1";
        result = opParser.GetParser()(prog7);
        _ASSERT(result);
        _ASSERT(result.value().first == 2);

        // test logic oper precedence
        std::string prog8 = "1 + 2 * 3 - 2 * 2";
        result = opParser.GetParser()(prog8);
        _ASSERT(result);
        _ASSERT(result.value().first == 3);

        std::string prog9 = "1 + 2 == 3 + 0";
        result = opParser.GetParser()(prog9);
        _ASSERT(result);
        _ASSERT(result.value().first == 1);

        std::string prog10 = "3 == 3 * 1 && 1 + 2 == 3 + 0";
        result = opParser.GetParser()(prog10);
        _ASSERT(result);
        _ASSERT(result.value().first == 1);

        std::string prog11 = "6 + 0 == 3 << 1";
        result = opParser.GetParser()(prog11);
        _ASSERT(result);
        _ASSERT(result.value().first == 1);
    }
}

void test_expr_parser()
{
    // test parse char
    {
        std::string prog1 = "'a'";
        auto c1 = pchar(prog1);
        _ASSERT(c1);

        std::string prog2 = "'\\n'";
        auto c2 = pchar(prog2);
        _ASSERT(c2);

        std::string prog3 = "'\r";
        auto c3 = pchar(prog3);
        _ASSERT(!c3);
    }

    // test parse string
    {
        std::string prog1 = "\"abc\"";
        auto c1 = pstring(prog1);
        _ASSERT(c1);

        std::string prog2 = "\"\r\n123 {}()+-*/'[]\\/.,\"";
        auto c2 = pstring(prog2);
        _ASSERT(c2);

        std::string prog3 = "\" sdf123";
        auto c3 = pstring(prog3);
        _ASSERT(!c3);
    }

    // test parse int
    {
        std::string prog1 = "1234";
        auto i1 = pint(prog1);
        _ASSERT(i1);

        std::string prog2 = "+1056";
        auto i2 = pint(prog2);
        _ASSERT(i2);

        std::string prog3 = "-56";
        auto i3 = pint(prog3);
        _ASSERT(i3);
    }

    // test parse float
    {
        std::string prog1 = "-0.0";
        auto f1 = pfloat(prog1);
        _ASSERT(f1);

        std::string prog2 = "+0.0";
        auto f2 = pfloat(prog2);
        _ASSERT(f2);

        std::string prog3 = "65536.5";
        auto f3 = pfloat(prog3);
        _ASSERT(f3);

        std::string prog4 = "-780.0";
        auto f4 = pfloat(prog4);
        _ASSERT(f4);
    }

    // test pvalue
    {
        std::string prog1 = "'a'";
        auto v1 = pvalue(prog1);
        _ASSERT(v1);

        std::string prog2 = "\"abc\"";
        auto v2 = pvalue(prog2);
        _ASSERT(v2);

        std::string prog3 = "-56";
        auto v3 = pvalue(prog3);
        _ASSERT(v3);

        std::string prog4 = "65536.5";
        auto v4 = pvalue(prog4);
        _ASSERT(v4);

        std::string prog5 = "'a'65536.5\"abc\"-56";
        auto pv5 = Many(pvalue) < Eof();
        auto v5 = pv5(prog5);
        _ASSERT(v5);
    }

    // test Half_Var
    {
        // test simple var
        {
            std::string prog1 = "abc123";
            auto v1 = psimplevar(prog1);
            _ASSERT(v1);

            std::string prog2 = "1abc";
            auto v2 = psimplevar(prog2);
            _ASSERT(!v2);
        }

        // test fieldname
        {
            std::string prog1 = ".abc123";
            auto v1 = pfieldname(prog1);
            _ASSERT(v1);
        }

        // test subscript
        {
            std::string prog1 = "[abc]";
            auto v1 = psubscriptstring(prog1);
            _ASSERT(v1);

            std::string prog2 = "[abc[123][0]]";
            auto v2 = psubscriptstring(prog2);
            auto v2_1 = Eof()(v2.value().second);
            _ASSERT(v2 && v2_1);

            std::string prog3 = "[abc";
            auto v3 = psubscriptstring(prog3);
            _ASSERT(!v3);
        }

        // test variable
        {
            std::string keyword = "let";
            auto v0 = pvar(keyword);
            _ASSERT(!v0);

            std::string prog1 = "a1";
            auto v1 = pvar(prog1);
            _ASSERT(v1);

            std::string prog2 = "a1.b2";
            auto v2 = pvar(prog2);
            _ASSERT(v2);

            std::string prog3 = "a1[abc]";
            auto v3 = pvar(prog3);
            _ASSERT(v3);

            std::string prog4 = "a1.b2[0]";
            auto v4 = pvar(prog4);
            _ASSERT(v4);

            std::string prog5 = "a1.b2[0][0].c3";
            auto v5 = pvar(prog5);
            _ASSERT(v5);
        }
    }

    // test funcall
    {
        std::string prog1 = "print( 1 , 2 , 3 )";
        auto f1 = pfuncall(prog1);
        _ASSERT(f1);
        _ASSERT(f1.value().second.empty());

        std::string prog2 = "print()";
        auto f2 = pfuncall(prog2);
        _ASSERT(f2);
        _ASSERT(f2.value().second.empty());

        std::string prog3 = "print(bar(1), v[1])";
        auto f3 = pfuncall(prog3);
        _ASSERT(f3);
        _ASSERT(f3.value().second.empty());
    }

    // test op
    {
        {
            std::string prog = "1 + 3";
            auto o1 = pop(prog);
            _ASSERT(o1);

            std::string prog2 = "a + b[bar01]";
            auto o2 = pop(prog2);
            _ASSERT(o2);

            std::string prog3 = "print(1, 2) + a * 2 - b[f]";
            auto o3 = pop(prog3);
            _ASSERT(o3);
        }
        
        {
            std::string prog1 = "1 + 2 * 3";
            auto o1 = pop(prog1);
            _ASSERT(o1);
        }

        {
            std::string prog1 = "1 + 2 * 3 - 4 ";
            auto o1 = pop(prog1);
            _ASSERT(o1);
        }

        {
            std::string prog1 = "1 + (6 - 3 * 1)";
            auto o1 = pop(prog1);
            _ASSERT(o1);
        }

        {
            std::string prog1 = "1 + 2 * 3 - 10 / 5";
            auto o1 = pop(prog1);
            _ASSERT(o1);
        }
    }

    // test assign
    {
        std::string prog1 = "a = 123";
        auto a1 = passign(prog1);
        _ASSERT(a1);

        std::string prog2 = "a = b[0]";
        auto a2 = passign(prog2);
        _ASSERT(a2);
    }

    // test if
    {
        std::string prog1 = "if a then 1 else 0 end";
        auto i1 = pif(prog1);
        _ASSERT(i1);
        _ASSERT(i1.value().second.empty());
    }

    // test function
    {
        std::string prog1 = "function print() = abc + 123 end";
        auto f1 = pfuncdecl(prog1);
        _ASSERT(f1);
        _ASSERT(f1.value().second.empty());

        std::string prog2 = "function print(a int, b char) = a - b end";
        auto f2 = pfuncdecl(prog2);
        _ASSERT(f2);
        _ASSERT(f2.value().second.empty());
    }

    // test def type
    {
        std::string prog = "type any = {any:int}";
        auto t1 = pdeftype(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());

        std::string prog2 = "type list = {first:int, rest:list}";
        auto t2 = pdeftype(prog2);
        _ASSERT(t2);
        _ASSERT(t2.value().second.empty());
    }

    // type usage
    {
        // there is a list type
        std::string prog = "list";
        auto t1 = ptypeuse(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());

        // there is a array type, it's size is 10
        std::string prog2 = "list[10]";
        auto t2 = ptypeuse(prog2);
        _ASSERT(t2);
        _ASSERT(t2.value().second.empty());

        // there is a ptr type
        std::string prog3 = "ptr of list";
        auto t3 = ptypeuse(prog3);
        _ASSERT(t3);
        _ASSERT(t3.value().second.empty());

        // there is a array type, it's size is 10
        std::string prog4 = "array of int[10]";
        auto t4 = ptypeuse(prog4);
        _ASSERT(t4);
        _ASSERT(t4.value().second.empty());

        // not support incomplete array type
        std::string prog5 = "array of list";
        auto t5 = ptypeuse(prog5);
        _ASSERT(!t5);
    }

    // test ptr type
    {
        std::string prog = "ptr of int";
        auto t1 = ppointertype(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    // test array type
    {
        std::string prog = "array of int [10]";
        auto t1 = parraytype(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    // test struct body define
    {
        std::string prog = "{a:int, b:char}";
        auto t1 = pstructbody(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }

    // test array init
    {
        std::string prog = "int3 [[1, 2, 3]]";
        auto t1 = parrayinit(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());

        std::string prog2 = "a[0] + a[1]";
        auto t2 = pop(prog2);
        _ASSERT(t2);
    }

    {
        std::string prog = "type list = {first:int, rest:list}";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "type list = array of int[10]";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "ints [[x]]";
        auto t1 = parraynew(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "type list = array of int";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "type list = ptr of int";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "type list = int";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());
    }
    {
        std::string prog = "type list = function ( ptr, ... ) : nil";
        auto t1 = ptypedecl(prog);
        _ASSERT(t1);
        _ASSERT(t1.value().second.empty());

        auto t2 = pexpr(prog);
        _ASSERT(t2);
        _ASSERT(t2.value().second.empty());
    }

    // test type size
    {
        std::shared_ptr<Table> global = std::make_shared<Table>();
        Init_Basic_Type(global);
        auto get_type_ptr = [&](const std::string& prog)
            {
                auto t1 = ptypedecl(prog);
                _ASSERT(t1);
                auto pty = Trans_Type(global, t1.value().first);
                return pty;
            };

        {
            std::string prog = "type a = int";
            auto pty = get_type_ptr(prog);
            _ASSERT(pty->GetSize() == 4);

            std::string prog2 = "type b = char";
            pty = get_type_ptr(prog2);
            _ASSERT(pty->GetSize() == 1);

            std::string prog3 = "type c = ptr of int";
            pty = get_type_ptr(prog3);
            _ASSERT(pty->GetSize() == 8);

            std::string prog4 = "type d = array of int[10]";
            pty = get_type_ptr(prog4);
            _ASSERT(pty->GetSize() == 40);

            std::string prog5 = "type e = d";
            pty = get_type_ptr(prog5);
            _ASSERT(pty->GetSize() == 40);

            std::string prog6 = "type f = array of a[10]";
            pty = get_type_ptr(prog6);
            _ASSERT(pty->GetSize() == 40);

            std::string prog7 = "type g = {x:int, y:int, z: int}";
            pty = get_type_ptr(prog7);
            _ASSERT(pty->GetSize() == 12);
        }
    }

    // test let
    {
        // test var def
        {
            std::string prog1 = "let a = 0";
            auto l1 = plet(prog1);
            _ASSERT(l1);
            _ASSERT(l1.value().second.empty());
        }
    }

    // test for
    {
        // test for loop
        {
            std::string prog1 = "for i = 0 to 10 do i end";
            auto f1 = pfor(prog1);
            _ASSERT(f1);
            _ASSERT(f1.value().second.empty());

            std::string prog2 = "for i = 10 downto -10 do i end";
            auto f2 = pfor(prog2);
            _ASSERT(f2);
            _ASSERT(f2.value().second.empty());

            std::string prog3 = "for i = 10 downto -10 do print(i) end";
            auto f3 = pfor(prog3);
            _ASSERT(f3);
            _ASSERT(f3.value().second.empty());
        }
    }

    // test while
    {
        {
            std::string prog1 = "while i do abc[1] end";
            auto w1 = pwhile(prog1);
            _ASSERT(w1);
        }
    }
}

void test_parserinput()
{
    std::string prog1 =
R"(function square(n int) : int =
    let x = n * n
    x
end)";
    std::string prog1_if =
R"(function square_biger_20(n int) : int =
    n + 10
    let x = n * n
    if x > 20 then
        x
    else
        0
    end
end)";
    std::string prog2 =
        prog1 +
        R"(
function main() : int =
    let x = 10
    x = x + 1
    square(x)
end)";
    std::string prog3 =
        R"(
function square n : int =
    let x = n * n
    x
end)";
    std::string prog4 =
        R"(
function square (n int) : int =
    let x = n * n
    x
end)";
    std::string prog5 =
        R"(
function while_sum_0_to_i(i int) : int =
    let c = 1
    let sum = 0
    while c <= i do
        sum = sum + c
        c = c + 1
    end
    sum
end)";
    std::string prog6 =
        R"(
function for_sum_0_to_i(i int) : int =
    let sum = 0
    for c = 1 to i + 1 do
        sum = sum + c
    end
    sum
end)";
    std::string prog7 =
        prog5 + prog6 +
        R"(
function main() : int =
    let x = 10
    let y = 0
    let z = 0
    y = while_sum_0_to_i(x)
    z = for_sum_0_to_i(x)
    if y == z then
        1
    else
        0
    end
end)";

    // test struct type
    std::string prog8 =
        R"(
type point3 = {x:int, y:int, z:int}
function main() : int =
    let p1 = point3 {x=1, y=2, z=3}
    let p2 = point3 { 3, 4, 5 }
    p1.x + p1.y + p1.z + p2.x + p2.y + p2.z
end)";

    // test array type
    std::string prog9 =
        R"(
type dyn_arr = array of int
type int10 = array of int[10]
function main() : int =
    let a = int10 [[1, 2, 3, 4, 5, 6, 7, 8, 9, 10]]
    a[1] = 3
    a[0] + a[1]
end)";

    std::string prog10 =
        R"(
type point = 
    {
        x:int,
        y:int,
        z:int,
        w:int
    }
type p10 = array of point
function main() : int =
    let ps = point{1,2,3,4}
    ps.z = 6
    0
end)";

    std::string prog11_0 =
        R"(
type int_array = array of int
)";

    std::string prog11_1 =
        R"(
function swap(arr int_array, i int, j int) : int =
    let t = arr[i]
    arr[i] = arr[j]
    arr[j] = t
    0
end)";

    std::string prog11_2 = 
        R"(
function partition(arr int_array, low int, high int) : int =
    let pivot = arr[low]
    let i = low
    let j = high
    while i < j do
        while i < j && arr[j] >= pivot do
            j = j - 1
        end
        if i < j then
            arr[i] = arr[j]
            i = i + 1
        else
            0
        end
        while i < j && arr[i] <= pivot do
            i = i + 1
        end
        if i < j then
            arr[j] = arr[i]
            j = j - 1
        else
            0
        end
    end
    arr[i] = pivot
    i
end)";

    std::string prog11_3 =
        R"(
function sort(arr int_array, low int, high int) : int =
    if low < high then
        let pivotIndex = partition(arr, low, high)
        sort(arr, low, pivotIndex - 1)
        sort(arr, pivotIndex + 1, high)
        1
    else
        2
    end
end)";

    std::string prog11 = prog11_0 + prog11_1 + prog11_2 + prog11_3 +
        R"(
type int_10 = array of int[11]
function main() : int =
    let arr = int_10 [[3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 7]]
    sort(arr, 0, 10)
end)";

    std::string prog12 =
        R"(
function main() : int =
    let str = "hello world"
    print(str)
    0
end)";

    std::string prog13 =
        R"(
type int_array = array of int

function Index2D (row int, col int, row_size int) : int =
    row * row_size + col
end

function CountLiveNeighbors (arr int_array, row int, col int, row_size int) : int =
    let count = 0
    let idx = 0
    for i = -1 to 2 do
        for j = -1 to 2 do
            let new_row = row + i
            let new_col = col + j
            idx = Index2D(new_row, new_col, row_size)
            count = count + arr[idx]
        end
    end
    idx = Index2D(row, col, row_size)
    count - arr[idx]
end

function IsAlive (arr int_array, row int, col int, row_size int) : int =
    let count = CountLiveNeighbors(arr, row, col, row_size)
    let idx = Index2D(row, col, row_size)
    if arr[idx] == 1 then
        if count < 2 || count > 3 then
            0
        else
            1
        end
    else
        if count == 3 then
            1
        else
            0
        end
    end
end

function UpdateArray(arr int_array, new_arr int_array, row_size int, col_size int) : int =
    row_size = row_size - 1
    col_size = col_size - 1
    for i = 1 to row_size do
        for j = 1 to col_size do
            let idx = Index2D(i, j, row_size)
            new_arr[idx] = IsAlive(arr, i, j, row_size)
        end
    end
    0
end)";

    /* {
        _ParserInput p1(prog1);
        _ASSERT(p1[0] == 'f');
        _ASSERT(p1[1] == 'u');
        _ASSERT(p1[25] == ' ');
        _ASSERT(p1[29] == 'e');
        _ASSERT(p1[30] == 't');
        p1.current_pos.column = 10;
        _ASSERT(p1[0] == 'q');
        p1.current_pos.line = 3;
        _ASSERT(p1[10] == '\0');
    }*/
    /* {
        _ParserInput p1(prog1);
        std::string func = "function";
        auto it = p1.begin();
        _ASSERT(*it == 'f');
        for (size_t i = 0; i < func.size(); i++)
        {
            _ASSERT(*it == func[i]);
            ++it;
        }
        it = p1.begin();
        for (size_t i = 0; i < func.size(); i++)
        {
            _ASSERT(*it == func[i]);
            it++;
        }
    }*/
    /* {
        ParserInput p1(prog2);
        printf("\n%s\n", prog2.c_str());
        auto e1 = pexpr(p1);
        _ASSERT(e1);
        auto e2 = pexpr(e1.value().second);
        _ASSERT(e2);

        auto f1 = pfuncdecl(p1);
        _ASSERT(f1);
        auto f2 = pfuncdecl(prog3);
        _ASSERT(f2);
        auto f3 = pfuncdecl(prog4);
        _ASSERT(f3);

        
    }*/

    /* {
        std::string assign = "let x = 10";
        std::string assign2 = "let x = 10\n x = 0";
        std::string assign3 = "let x = 10\n x = 1 + 2\n x = 1";
        std::string if_1 = assign + "\n if x > 0 then 1 else -1 end";
        {
            auto check = TypeCheck();
            if (check.Check(pexpr(assign).value().first))
            {
                printf("TypeCheck success\n");
            }
        }
        {
            auto check = TypeCheck();
            auto e = pprogram(assign2);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
        }
        {
            auto check = TypeCheck();
            auto e = pprogram(assign3);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck(op +) success\n");
            }
        }
        {
            auto check = TypeCheck();
            auto e = pprogram(prog2);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
        }
        {
            auto check = TypeCheck();
            auto e = pprogram(if_1);
            if (!check.Check(e.value().first))
            {
                printf("TypeCheck Failed!!!  ---------\n");
            }
        }
    }*/
    {
        /* { // test prog1
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog1);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            auto ir = Trans_Expr(e.value().first);
            auto exp = ir.exp;
            std::vector<AS_Instr> instrs;
            
            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());
            auto exps = b.GetBlock(0).exps[0];
            MunchExp_llvmlike(exps, instrs);
            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/
        /*{   // test prog1_if
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog1_if);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            std::vector<AS_Instr> instrs;

            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());

            MunchExps_llvmlike(b, instrs);

            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/
        /*{   // test while code gen
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog5);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            std::vector<AS_Instr> instrs;

            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());

            MunchExps_llvmlike(b, instrs);

            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/
        /*{   // test for code gen
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog6);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            std::vector<AS_Instr> instrs;

            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());

            MunchExps_llvmlike(b, instrs);

            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/

        /*{   // test for code gen
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog7);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            std::vector<AS_Instr> instrs;

            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());

            MunchExps_llvmlike(b, instrs);

            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/
        /*{   // test for code gen
            Builder b;
            auto check = TypeCheck();
            auto e = pprogram(prog8);
            if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }
            std::vector<AS_Instr> instrs;

            auto ir_name = Trans_Expr(e.value().first, b);
            printf("\n%s\n", to_string(ir_name.name).c_str());

            MunchExps_llvmlike(b, instrs);

            printf("\nCount %zd\n", instrs.size());
            for (size_t i = 0; i < instrs.size(); i++)
            {
                printf("%s", to_string(instrs[i]).c_str());
            }
            auto g = Graph();
            g.initialize(instrs);
            auto live = Liveness();
            live.initialize(g);
            printf("\nCount %zd\n", g.Nodes.size());
            auto rlive = Liveness();
            rlive.rinitialize(g);
            live == rlive;
            Color color(8);
            color.initialize(live);
            color.allocate();
            color.print();
            RegAlloc regalloc;
            regalloc.allocate(g, live);
            for (size_t i = 0; i < g.instrs.size(); i++)
            {
                printf("%s", to_string(g.instrs[i]).c_str());
            }
        }*/
        {   // test for code gen
            Builder b;
            auto check = TypeCheck();
            //auto e = pprogram(prog11_0 + prog11_2 + prog11_3);
            //auto e = pprogram(prog13);
            auto e = pprogram(prog11);
            _ASSERT(e.value().second.empty());
            /*if (check.Check(e.value().first))
            {
                printf("TypeCheck success\n");
            }*/

            Trans_Outer(e.value().first, b);
            printf("\nbuilder exprs count: %zd\n", b.blocks[0].exps.size());
            
            for (auto& block : b.blocks)
            {
                for (auto& exp : block.exps)
                {
                    printf("exp index: %zd\n", exp.exp.index());
                    if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(&exp.exp))
                    {
                        printf("func name: %s\n", (*pfunc)->name.c_str());
                        for (auto& block : (*pfunc)->blocks)
                        {
                            printf("    block name: %s\n", block.label.l.c_str());
                            for (auto i : block.preds)
                            {
                                printf("        pred: %s\n", (*pfunc)->blocks[i].label.l.c_str());
                            }
                            for (auto i : block.succs)
                            {
                                printf("        succ: %s\n", (*pfunc)->blocks[i].label.l.c_str());
                            }
                        }
                    }
                }
            }

            for (auto idx = 0; idx < b.blocks[0].exps.size(); ++idx)
            {
                std::vector<AS_Block> blocks;
                MunchExp_llvmlike(b.blocks[0].exps[idx], blocks);
                std::vector<Graph> graphs;
                for (size_t i = 0; i < blocks.size(); i++)
                {
                    Graph g;
                    g.initialize_new(blocks[i]);
                    //printf("def: %zd, use: %zd\n", g.Def().size(), g.Use().size());
                    graphs.push_back(g);
                }

                for (auto& g : graphs)
                {
                    for (size_t i = 0; i < g.Nodes.size(); i++)
                    {
                        printf("%s", to_string(g.Nodes[i].info).c_str());
                    }
                }

                Liveness_Graph liveness;
                liveness.rinitialize(graphs);
                RegAlloc regalloc;
                regalloc.allocate(graphs, liveness);

                for (auto& g : graphs)
                {
                    for (size_t i = 0; i < g.Nodes.size(); i++)
                    {
                        printf("%s", to_string(g.Nodes[i].info).c_str());
                    }
                }
            }

            for (auto idx = 0; idx < b.blocks[0].exps.size(); ++idx)
            {
                IR_Print_Pass printer;
                std::vector<std::string> lines;
                auto& e = b.blocks[0].exps[idx];
                Mem2RegPass mem2reg;
                if (auto pfunc = std::get_if<std::shared_ptr<Half_Ir_Function>>(&e.exp))
                {
                    printf("func name: %s\n", (*pfunc)->name.c_str());
                    mem2reg.Run(**pfunc);
                    CFG cfg(**pfunc);
                    cfg.dump();
                    DominatorTree dom(cfg);
                    dom.dump();
                    printf("\n");
                }
                printer.Run(e);
                printer.dump(lines);
                for (size_t i = 0; i < lines.size(); i++)
                {
                    //printf("%s\n", lines[i].c_str());
                }
            }
        }
    }
}

void test_ir()
{
    Builder builder;
    {
        std::string prog1 = "999";
        auto e1 = pexpr(prog1);
        _ASSERT(e1);

    }

    {
        std::string prog1 = "1+2";
        auto e1 = pexpr(prog1);
        _ASSERT(e1);

    }

    {
        std::string prog1 = "\
            function square(n int) = n * n end\
            function main() = square(5) end";
        auto f1 = pprogram(prog1);
        _ASSERT(f1);

        Trans_Outer(f1.value().first, builder);
    }
}
