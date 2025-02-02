#include "test.h"
//#include "Parser/Primitives.h"
#include "Parser/CharParsers.h"
#include "Parser/Operator.h"
#include "cmdline.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

// 三地址代码表示
struct ThreeAddressCode {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

// x86汇编指令表示
struct X86Instruction {
    std::string op;
    std::string arg1;
    std::string arg2;
};


namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    /*trans_three_address_code();
    test_parser_function();
    test_syntax_parser();
    test_operator_parser();
    test_expr_parser();*/
    /*test_ir(); */
    //test_parserinput();
    //std::cout << test_main_0;
    //return 0;
#endif // DEBUG


    auto cmdline = parse_cmdline(argc, argv);

    std::vector<std::string> args(argv, argv + argc);
    std::string input_path = args[0];
    std::ifstream infile(input_path);
    if (!infile) {
        std::cerr << "Error opening file: " << input_path << std::endl;
        return 1;
    }

    fs::path input_file = fs::path(cmdline.input_file);
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
        //std::cout << "File is open" << std::endl;
    }
    input.clear();

    std::string prog;
    std::string line;
    while (std::getline(input, line))
    {
        //std::cout << line << std::endl;
        prog.append(line + "\n");
        //output << line << std::endl;
    }
    input.close();


    Builder builder;
    auto f1 = pprogram(prog);
    if (!f1)
    {
        std::cerr << "Syntax error" << std::endl;
        return 1;
    }
    Trans_Outer(f1.value().first, builder);

    if (cmdline.syntax_only)
    {
        return 0;
    }

    // generate ast
    for (auto idx = 0; idx < builder.blocks[0].exps.size(); ++idx)
    {
        std::vector<AS_Block> blocks;
        MunchExp_llvmlike(builder.blocks[0].exps[idx], blocks);
        std::vector<Graph> graphs;
        for (size_t i = 0; i < blocks.size(); i++)
        {
            Graph g;
            g.initialize_new(blocks[i]);
            //printf("def: %zd, use: %zd\n", g.Def().size(), g.Use().size());
            graphs.push_back(g);
        }

        if (cmdline.ir_only)
        {
            IR_Print_Pass printer;
            std::vector<std::string> lines;
            printer.Run(builder.blocks[0].exps[idx]);
            printer.dump(lines);
            if (cmdline.print_to_stdout)
            {
                for (size_t i = 0; i < lines.size(); i++)
                {
                    printf("%s", lines[i].c_str());
                }
            }
            if (cmdline.output_to_file)
            {
                for (size_t i = 0; i < lines.size(); i++)
                {
                    output << lines[i];
                }
            }
            continue;
        }

        Liveness_Graph liveness;
        liveness.rinitialize(graphs);

        RegAlloc regalloc;
        regalloc.allocate(graphs, liveness);


        if (cmdline.graph_only)
        {
            if (cmdline.print_to_stdout)
            {
                for (auto& g : graphs)
                {
                    for (size_t i = 0; i < g.Nodes.size(); i++)
                    {
                        printf("%s", to_string(g.Nodes[i].info).c_str());
                    }
                }
            }
            if (cmdline.output_to_file)
            {
                for (auto& g : graphs)
                {
                    for (size_t i = 0; i < g.Nodes.size(); i++)
                    {
                        // output to file
                        output << to_string(g.Nodes[i].info);
                    }
                }
            }
            continue;
        }

        AS_Declear decl(blocks[0].label.l);
        if (cmdline.output_to_file)
        {
            output << to_string(decl);
            for (auto& g : graphs)
            {
                for (size_t i = 0; i < g.Nodes.size(); i++)
                {
                    // output to file
                    output << to_string(g.Nodes[i].info);
                }
            }
        }
        if (cmdline.print_to_stdout)
        {
            printf("%s", to_string(decl).c_str());
            for (auto& g : graphs)
            {
                for (size_t i = 0; i < g.Nodes.size(); i++)
                {
                    printf("%s", to_string(g.Nodes[i].info).c_str());
                }
            }
        }
    }

    output.close();
    return 0;
}
