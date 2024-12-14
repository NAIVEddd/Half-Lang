#include "test.h"
//#include "Parser/Primitives.h"
#include "Parser/CharParsers.h"
#include "Parser/Operator.h"
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

// 寄存器分配
std::unordered_map<std::string, std::string> registerAllocation = {
    {"t1", "eax"},
    {"t2", "ebx"},
    {"a", "ecx"},
    {"b", "edx"},
    {"c", "esi"},
    {"d", "edi"}
};

// 将三地址代码转换为x86汇编指令
std::vector<X86Instruction> generateX86Assembly(const std::vector<ThreeAddressCode>& tac) {
    std::vector<X86Instruction> instructions;

    for (const auto& code : tac) {
        if (code.op == "+") {
            instructions.push_back({ "mov", registerAllocation[code.arg1], registerAllocation[code.result] });
            instructions.push_back({ "add", registerAllocation[code.arg2], registerAllocation[code.result] });
        }
        else if (code.op == "*") {
            instructions.push_back({ "mov", registerAllocation[code.arg1], registerAllocation[code.result] });
            instructions.push_back({ "imul", registerAllocation[code.arg2], registerAllocation[code.result] });
        }
        else if (code.op == "=") {
            instructions.push_back({ "mov", registerAllocation[code.arg1], registerAllocation[code.result] });
        }
    }

    return instructions;
}

// 输出x86汇编代码
void printX86Assembly(const std::vector<X86Instruction>& instructions) {
    for (const auto& instr : instructions) {
        std::cout << instr.op << " " << instr.arg1;
        if (!instr.arg2.empty()) {
            std::cout << ", " << instr.arg2;
        }
        std::cout << std::endl;
    }
}

int trans_three_address_code() {
    // 示例三地址代码
    std::vector<ThreeAddressCode> tac = {
        {"+", "a", "b", "t1"},
        {"*", "t1", "c", "t2"},
        {"=", "t2", "", "d"}
    };

    // 生成x86汇编指令
    std::vector<X86Instruction> instructions = generateX86Assembly(tac);

    // 输出x86汇编代码
    printX86Assembly(instructions);

    return 0;
}


namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
#ifdef _DEBUG
    /*trans_three_address_code();
    test_parser_function();
    test_syntax_parser();
    test_operator_parser();
    test_expr_parser();
    test_ir();*/
    test_parserinput();
    //std::cout << test_main_0;
    return 0;
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
