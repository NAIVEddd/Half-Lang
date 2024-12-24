#pragma once
#include"IR.h"
#include"MakeIR.h"
#include"Symbol.h"
#include"Builder.h"
#include<memory>

Half_Ir_Name Trans_Expr(Half_Expr& expr, Builder& builder);
Half_Ir_Name Trans_Expr(std::shared_ptr<Table>& table, Builder& builder, Half_Expr& expr);

const Half_Ir_Name& Trans_Var(std::shared_ptr<Table>& table, Half_Var& var, Builder& builder);
