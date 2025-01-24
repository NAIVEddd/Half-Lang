#pragma once
#include<string>
#include<iostream>

// parse comd line args
// cmd line support :
// -std <print to standard out>
// -o <output_to_file>
// -h <help>
// -s <syntax only>
// -ir <ir only>
// -graph <graph only>

struct CmdLineArgs
{
    bool print_to_stdout = false;
    bool output_to_file = false;
    bool syntax_only = false;
    bool ir_only = false;
    bool graph_only = false;
    std::string input_file;
    std::string output_file;
};

CmdLineArgs parse_cmdline(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> [-std] [-o <output_file>] [-h] [-s] [-ir] [-graph]" << std::endl;
        exit(1);
    }
    CmdLineArgs args;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-std")
        {
            args.print_to_stdout = true;
        }
        else if (arg == "-o")
        {
            args.output_to_file = true;
            args.output_file = argv[++i];
        }
        else if (arg == "-h")
        {
            std::cout << "Usage: " << argv[0] << " <input_file> [-std] [-o <output_file>] [-h] [-s] [-ir] [-graph]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -std\t\tPrint to standard out" << std::endl;
            std::cout << "  -o\t\tOutput to file" << std::endl;
            std::cout << "  -h\t\tHelp" << std::endl;
            std::cout << "  -s\t\tSyntax only" << std::endl;
            std::cout << "  -ir\t\tIR only" << std::endl;
            std::cout << "  -graph\tGraph only" << std::endl;
            exit(0);
        }
        else if (arg == "-s")
        {
            args.syntax_only = true;
        }
        else if (arg == "-ir")
        {
            args.ir_only = true;
        }
        else if (arg == "-graph")
        {
            args.graph_only = true;
        }
        else
        {
            args.input_file = arg;
        }
    }
    return args;
}