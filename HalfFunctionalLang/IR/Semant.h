#pragma once
#include"IR.h"
#include"MakeIR.h"
#include"Symbol.h"
#include"Builder.h"
#include"Type.h"
#include<memory>

size_t Access_Var_Offset(std::shared_ptr<Table>& table, Half_Var& var, Half_Type_Info& type);
Half_Ir_GetElementPtr Trans_LeftVar_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder);

void Init_Basic_Type(std::shared_ptr<Table>& table);
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl& type);
Half_Ir_Name Trans_Expr(Half_Expr& expr, Builder& builder);
Half_Ir_Name Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr);
Value Trans_Expr(Half_Expr& expr, Builder& builder, int);
Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr, int);

const Half_Ir_Name& Trans_Var(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder);
