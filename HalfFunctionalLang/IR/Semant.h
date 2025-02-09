#pragma once
#include"IR.h"
#include"MakeIR.h"
#include"Symbol.h"
#include"Builder.h"
#include"Type.h"
#include<memory>

Half_Ir_GetElementPtr Trans_LeftVar_Builder(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder);

void Init_Basic_Type(std::shared_ptr<Table>& table);
Half_Type_Info Get_Expr_Type(std::shared_ptr<Table>& table, Half_Expr& expr);
std::shared_ptr<Half_Type_Info> Trans_Type(std::shared_ptr<Table>& table, Half_TypeDecl& type);
Value Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr);
void Trans_Outer(Half_OuterExpr& outer, Builder& builder);
void Trans_Outer(std::shared_ptr<Table>& table, Builder& builder, Half_OuterExpr& outer);
