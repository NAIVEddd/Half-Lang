#pragma once
#include"../Syntax/Base.h"
#include"IR.h"
#include<functional>
#include<variant>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

//using var_t = std::variant<int, long, double, std::string>;
//std::vector<var_t> vec = {10, 15l, 1.5, "hello"};
//for (auto& v : vec)
//{
//    // 4. another type-matching visitor: a class with 3 overloaded operator()'s
//    // Note: The `(auto arg)` template operator() will bind to `int` and `long`
//    //       in this case, but in its absence the `(double arg)` operator()
//    //       *will also* bind to `int` and `long` because both are implicitly
//    //       convertible to double. When using this form, care has to be taken
//    //       that implicit conversions are handled correctly.
//    std::visit(overloaded{
//        [](auto arg) { std::cout << arg << ' '; },
//        [](double arg) { std::cout << std::fixed << arg << ' '; },
//        [](const std::string& arg) { std::cout << std::quoted(arg) << ' '; }
//        }, v);
//}

