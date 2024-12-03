#include "test.h"
//#include "Parser/Primitives.h"
#include "Parser/CharParsers.h"
#include "Parser/Operator.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    //test_parser_function();
    //test_syntax_parser();
    //test_operator_parser();
    //test_expr_parser();
    test_ir();
    std::cout << test_main_0;
#endif // DEBUG


    if (argc < 2) {
        fs::path p = fs::path(argv[0]);
        std::cerr << "Usage: " << p.filename() << " <input_file>" << std::endl;
        //return 1;
    }

    std::vector<std::string> args(argv, argv + argc);
    std::string input_path = args[0];
    std::ifstream infile(input_path);
    if (!infile) {
        std::cerr << "Error opening file: " << input_path << std::endl;
        return 1;
    }

    fs::path input_file = fs::path(args[1]);
    if (input_file.is_relative())
    {
        input_file = fs::path(args[0]).parent_path() / input_file;
    }
    fs::path output_file = input_file;
    output_file.replace_extension(".s");

    std::ifstream input(input_file);
    std::ofstream output(output_file, std::ios::out | std::ios::trunc);

    if (input.is_open() && input.good())
    {
        std::cout << "File is open" << std::endl;
    }
    input.clear();

    std::string prog;
    std::string line;
    while (std::getline(input, line))
    {
        std::cout << line << std::endl;
        prog.append(line + "\n");
        //output << line << std::endl;
    }

    auto f1 = pprogram(prog);
    auto ir = Trans_Expr(f1.value().first);

    // generate ast
    std::vector<AS_Instr> instrs;
    MunchExp(ir, instrs);

    // analyze liveness
    auto g = Graph();
    g.initialize(instrs);
    auto live = Liveness();
    live.rinitialize(g);

    //RegAlloc reg;
    RegAlloc regalloc;
    regalloc.allocate(g, live);

    // output to file
    for (size_t i = 0; i < g.instrs.size(); i++)
    {
        output << to_string(g.instrs[i]);
    }

    input.close();
    output.close();
    return 0;
}
