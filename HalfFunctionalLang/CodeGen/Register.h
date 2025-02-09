#pragma once

#include <string>
#include <vector>

#include "../IR/Type.h"


// if type is float then index is the index of the float register
// if type is int then index is the index of the int register
//    if type size is 4 then register is 32 bit, eax, ebx, ecx...
//    if type size is 8 then register is 64 bit, rax, rbx, rcx...
struct AS_Register
{
    std::vector<std::string> name_32bit;
    std::vector<std::string> name_64bit;
    std::vector<std::string> name_float;
    std::vector<std::string> name_arg32;
    std::vector<std::string> name_arg64;
    std::vector<std::string> name_argfloat;

    AS_Register()
    {
        name_32bit = { "eax", "ebx", "ecx", "edx", "esi", "edi", "r8d", "r9d" };
        name_64bit = { "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9" };
        name_float = { "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7" };
        name_arg32 = { "", "", "ecx", "edx", "", "", "r8d", "r9d" };
        name_arg64 = { "", "", "rcx", "rdx", "", "", "r8", "r9" };
        name_argfloat = { "", "", "xmm0", "xmm1", "", "", "xmm2", "xmm3" };
    }

    std::string get_register(int index, Half_Type_Info& ty)
    {
        if(ty.is_float())
        {
            return name_float[index];
        }
        if (ty.GetSize() == 4)
        {
            return name_32bit[index];
        }
        else if (ty.GetSize() == 8)
        {
            return name_64bit[index];
        }
        else
        {
            _ASSERT(0);
            return "";
        }
    }

    std::string get_return_register(Half_Type_Info& ty)
    {
        if (ty.is_float())
        {
            return name_float[0];
        }
        if (ty.GetSize() == 4)
        {
            return name_32bit[0];
        }
        else if (ty.GetSize() == 8)
        {
            return name_64bit[0];
        }
        else
        {
            _ASSERT(0);
            return "";
        }
    }

    std::string get_arg_register(int index, Half_Type_Info& ty)
    {
        if (ty.is_float())
        {
            return name_argfloat[index];
        }
        else if (ty.GetSize() == 4)
        {
            return name_arg32[index];
        }
        else if (ty.GetSize() == 8)
        {
            return name_arg64[index];
        }
        else
        {
            _ASSERT(0);
            return "";
        }
    }
};