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
function square n =
    let x = n * n
    x
end)";
    std::string prog5 =
        R"(
function sum_0_to_i(i int) : int =
    let c = 1
    let sum = 0
    while c < i && 1 < 2 do
        let x =
            if c > 5 then
                1
            else
                0
            end
        sum = sum + c
        c = c + 1
    end
    sum
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
    {
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

        
    }

    {
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
    }
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
        {   // test prog1_if
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
        }
        {   // test while code gen
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
        }
    }
}

void test_ir()
{
    {
        std::string prog1 = "999";
        auto e1 = pexpr(prog1);
        _ASSERT(e1);

        //auto v = std::get<std::shared_ptr<Half_Value>>(e1.value().first.expr);
        //auto ir = IR_Make_Value(v);
        auto ir = Trans_Expr(e1.value().first);
        auto ptr = std::get<std::shared_ptr<Half_Ir_Const>>(ir.exp);
        _ASSERT(ptr);
    }

    {
        std::string prog1 = "1+2";
        auto e1 = pexpr(prog1);
        _ASSERT(e1);

        //auto op = std::get<std::shared_ptr<Half_Value>>(e1.value().first.expr);
        auto ir = Trans_Expr(e1.value().first);
        auto index = ir.exp.index();
        _ASSERT(index >= 0);
    }

    {
        std::string prog1 = "\
            function square(n int) = n * n end\
            function main() = square(5) end";
        auto f1 = pprogram(prog1);
        _ASSERT(f1);

        auto ir = Trans_Expr(f1.value().first);
        auto exp = ir.exp;
    }

    /* {
        std::string prog1 = "	\
            function square(n int) = n * n end\
            function main() =	\
                let x = 10		\
                x = x + 1		\
                square(x)		\
            end";
        auto f1 = pprogram(prog1);
        _ASSERT(f1);
        _ASSERT(f1.value().second.empty());

        auto ir = Trans_Expr(f1.value().first);
        auto exp = ir.exp;
        std::vector<AS_Instr> instrs;
        MunchExp(ir, instrs);
    }*/

    {
        std::string prog1 = "			\
            function square(n int) =	\
                let x = n * n			\
                x						\
            end";
        auto f1 = pprogram(prog1);
        _ASSERT(f1);
        _ASSERT(f1.value().second.empty());

        auto ir = Trans_Expr(f1.value().first);
        auto exp = ir.exp;
        std::vector<AS_Instr> instrs;
        MunchExp(ir, instrs);
        for (size_t i = 0; i < instrs.size(); i++)
        {
            printf("%zd\n", instrs[i].index());
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

        /*printf("\n");
        for (size_t i = 0; i < g.instrs.size(); i++)
        {
            printf("%s", to_string(g.instrs[i]).c_str());
        }*/
    }
}
